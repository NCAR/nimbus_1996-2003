/*
-------------------------------------------------------------------------
OBJECT NAME:	adsIO.c

FULL NAME:		ADS Record IO routines

ENTRY POINTS:	ExtractHeaderIntoFile(char *ADSfileName)
				FindFirstLogicalRecord(char *record, long starttime)
				FindNextLogicalRecord(char *record, long endtime)
				FindNextDataRecord(char *record)
				GetNextADSfile()
				CloseADSfile()

STATIC FNS:		none

DESCRIPTION:	These routines locate data records that start with the
				ID = 0x8681.  (i.e. skips all PMS2D records).

INPUT:			Pointer to where to place the record
				Time of first record desired / Time last record desired

OUTPUT:			Length of record or ERR

REFERENCES:		tapeIO.c

REFERENCED BY:	lrloop.c, hrloop.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>

#include "nimbus.h"
#include "decode.h"
#include "ctape.h"
#include "raf.h"

/* Values for DiskData, 1st one matches FALSE.	*/
#define TAPE_DATA	0
#define RAW_ADS		1
#define COS_BLOCKED	2

/* ADS image record types (sans pms2d).	*/
#define ADS_WORD	0x4144
#define HDR_WORD	0x5448
#define SDI_WORD	SDIWRD

#define FIRST_DATA_RECORD	((long)3)


static bool	DiskData = FALSE;
static char	phys_rec[MX_PHYS] = "";
static char *adsFileName;
static long	lrlen, lrppr, currentLR;
static int	infd;

extern long	LITTON51_start;

static long		FindNextDataRecord(char buff[]);


/* -------------------------------------------------------------------- */
long FindFirstLogicalRecord(
	char	record[],	/* First Data Record, for start time	*/
	long	starttime)	/* User specified start time		*/
{
	long		rectime, nbytes;
	static int	firstTime = TRUE;

	if (firstTime)
		{
		int		rc;

		firstTime = FALSE;

		get_lrlen(&lrlen);
		get_lrppr(&lrppr);

		switch (DiskData)
			{
			case RAW_ADS: rc = lseek(infd, 0L, SEEK_SET); break;
			case COS_BLOCKED: rc = 0; break;
			default: rc = TapeSeek(FIRST_DATA_RECORD);
			}

		if (rc == ERR)
			return(ERR);

		if ((nbytes = FindNextDataRecord(phys_rec)) <= 0)
			return(nbytes);

		if (starttime == BEG_OF_TAPE)
			return(FindNextLogicalRecord(record, starttime));
		}
	else
		currentLR = 0;


	rectime = (long)HdrBlkTimeToSeconds((struct Hdr_blk  *)phys_rec);

	/*		     12:00:00			   23:59:59	*/
	if (rectime < 43200L && starttime > 86399L)
		rectime += 86399L;

	while (starttime > (rectime + lrppr))
		{
		if ((nbytes = FindNextDataRecord(phys_rec)) <= 0)
			return(nbytes);

		rectime = HdrBlkTimeToSeconds((struct Hdr_blk  *)phys_rec);

		/*	     12:00:00		   23:59:59	*/
		if (rectime < 43200L && starttime > 86399L)
			rectime += 86399L;

		FlushXEvents();
		}


	/* Cover the case if start time is before first record on file
	 */
	if (starttime < rectime)
		starttime = rectime;


	while ((nbytes = FindNextLogicalRecord(record, starttime)) > 0)
		{
		rectime = HdrBlkTimeToSeconds((struct Hdr_blk *)record);

		/*	     12:00:00		   23:59:59	*/
		if (rectime < 43200L && starttime > 86399L)
			rectime += 86399L;

		if (rectime >= starttime)
			break;
		}

	return(nbytes);

}	/* END FINDFIRSTLOGICALRECORD */

/* -------------------------------------------------------------------- */
long FindNextLogicalRecord(
	char	record[],
	long	endtime)
{
	int	nbytes, rectime, i;
	long	TansStart;

	rectime = HdrBlkTimeToSeconds(&phys_rec[currentLR * lrlen]);

	if (endtime != END_OF_TAPE)
		{
		/*	     12:00:00		 23:59:59	*/
		if (rectime < 43200L && endtime > 86399L)
			rectime += 86399L;

		if (rectime > endtime)
			return(0);	/* End Of Time Segment	*/
		}

	memcpy(record, &phys_rec[currentLR * lrlen], lrlen);

	if (++currentLR >= lrppr)
		{
		if ((nbytes = FindNextDataRecord(phys_rec)) <= 0)
			return(nbytes);

		currentLR = 0;
		}

	
	/* Lag the Litton 51 INS one second.
	 */
	if (LITTON51_present)
		memcpy( &record[LITTON51_start],
			&phys_rec[currentLR * lrlen + LITTON51_start],
			sizeof(struct Ins_blk));


	/* Lag GPS position & velocity, only if off by 1 second.
	 */
	if (GetStart("GPST3", &TansStart) != ERR)
		{
		Gps_blk	*gp_src = (Gps_blk *)&phys_rec[currentLR * lrlen + TansStart],
				*gp_dst = (Gps_blk *)&record[TansStart];

		if ((int)gp_dst->gpstime == (int)gp_dst->postime + 1)
			{
			gp_dst->glat = gp_src->glat;
			gp_dst->glon = gp_src->glon;
			gp_dst->galt = gp_src->galt;
			gp_dst->postime = gp_src->postime;
			}

		if ((int)gp_dst->gpstime == (int)gp_dst->veltime + 1)
			{
			gp_dst->veleast = gp_src->veleast;
			gp_dst->velnrth = gp_src->velnrth;
			gp_dst->velup = gp_src->velup;
			gp_dst->veltime = gp_src->veltime;
			}
		}

	return(lrlen);

}	/* END FINDNEXTLOGICALRECORD */

/* -------------------------------------------------------------------- */
char *ExtractHeaderIntoFile(char *fileName)
{
	int		nBytes, rc;
	int		iflag = 0, mode = 0, nWords = 4096, iconv = 0;
	char	*tmpFile;
	FILE	*fp;

	DiskData = RAW_ADS;
	adsFileName = fileName;

	tmpFile = tempnam("/tmp/", "hdr.");

	if ((fp = fopen(tmpFile, "wb")) == NULL)
		{
		fprintf(stderr, "Unable to open tmpFile %s, fatal.\n", tmpFile);
		exit(1);
		}

	if (strncmp(adsFileName, "/dev/rmt/", 9) == 0)
		{
		DiskData = FALSE;
		TapeOpen(adsFileName);

		nBytes = TapeRead(phys_rec);	/* Skip "ADS_DATA_TAPE" record	*/
		nBytes = TapeRead(phys_rec);
		fwrite(phys_rec, nBytes, 1, fp);
		fclose(fp);
		return(tmpFile);
		}


	if ((infd = crayopen(adsFileName, &iflag, &mode, strlen(adsFileName))) < 0)
		DiskData = RAW_ADS;
	else
		{
		DiskData = COS_BLOCKED;

		if ((rc = crayread(&infd, phys_rec, &nWords, &iconv)) < 0)
			{
			crayclose(&infd);
			DiskData = RAW_ADS;
			}
		else
			{
			if ((rc = crayread(&infd, phys_rec, &nWords, &iconv)) < 0) {
				fprintf(stderr, "crayread: read error %d\n", rc);
				exit(1);
				}

			/* 8 is cray word size, see man page before adjusting.	*/
fprintf(stderr, "adsIO: COS blocked.\n");
			fwrite(phys_rec, (rc * 8) + 8, 1, fp);
			crayread(&infd, phys_rec, &nWords, &iconv);
			}
		}


	if (DiskData == RAW_ADS)
		{
		Fl	fi;
fprintf(stderr, "adsIO: RAW_ADS\n");
		if ((infd = open(adsFileName, O_RDONLY)) < 0)
			{
			fprintf(stderr, "Failure opening input file %s.\n", adsFileName);
			exit(1);
			}

		read(infd, phys_rec, 200);

		if (strncmp(FIRST_REC_STRING, (char *)phys_rec, strlen(FIRST_REC_STRING)) != 0)
			{
			memcpy((char *)&fi, phys_rec, sizeof(Fl));
			lseek(infd, 0L, SEEK_SET);
			}
		else
			{
			memcpy((char *)&fi, &phys_rec[20], sizeof(Fl));
			lseek(infd, 20L, SEEK_SET);
			}

		nBytes = fi.thdrlen;

		read(infd, phys_rec, nBytes);
		fwrite(phys_rec, nBytes, 1, fp);
		}

	fclose(fp);
	return(tmpFile);

}	/* END EXTRACTHEADERINTOFILE */

/* -------------------------------------------------------------------- */
int CloseADSfile()
{
	switch (DiskData)
		{
		case FALSE:
			TapeClose();
			return(0);

		case RAW_ADS:
			return(close(infd));

		case COS_BLOCKED:
			return(crayclose(&infd));
		}

}	/* END CLOSEADSFILE */

/* -------------------------------------------------------------------- */
int GetNextADSfile()
{
	char    *dot = strchr(adsFileName, '.');
	int		iflag = 0, mode = 0;

	static char seq = 'a';

	if (DiskData == FALSE || dot == NULL || dot[-1] != seq)
		return(FALSE);

	dot[-1] = ++seq;

	if (access(adsFileName, R_OK) == ERR)
		return(FALSE);

	if (DiskData == RAW_ADS)
		close(infd);
	else
		crayclose(&infd);

	if (DiskData == RAW_ADS && (infd = open(adsFileName, O_RDONLY)) == ERR)
		{
		LogMessage("Can't open next file in sequence.\n");
		return(FALSE);
		}

	if (DiskData == COS_BLOCKED && (infd = crayopen(adsFileName, &iflag, &mode,
												strlen(adsFileName))) < 0)
		{
		LogMessage("Can't open next file in sequence.\n");
		return(FALSE);
		}

	return(TRUE);

}	/* END GETNEXTADSFILE */

/* -------------------------------------------------------------------- */
#define ONE_WORD	sizeof(short)

static long FindNextDataRecord(char buff[])
{
	long	nbytes;

	do
		{
		if (DiskData == FALSE)
			nbytes = TapeRead(buff);
		else
		if (DiskData == COS_BLOCKED)
			{
			int		nWords = 4096, iconv = 0;

			if ((nbytes = crayread(&infd, buff, &nWords, &iconv)) < 0 &&
														GetNextADSfile())
				nbytes =  crayread(&infd, buff, &nWords, &iconv);
			}
		else
		if (DiskData == RAW_ADS)
			{
			long	len;
			int		size = ONE_WORD;

			if ((nbytes = read(infd, buff, size)) == 0 && GetNextADSfile())
				nbytes = read(infd, buff, size);

			if (nbytes <= 0)
				break;

			switch (*((ushort *)buff))
				{
				case SDI_WORD:
					size = lrppr * lrlen - ONE_WORD;
					break;

				case ADS_WORD:
					size = 18;
					break;

				case HDR_WORD:
					get_thdrlen(&len);
					size = len - ONE_WORD;
					break;

				case PMS2DC1: case PMS2DC2: /* PMS2D */
				case PMS2DP1: case PMS2DP2:
				case PMS2DH1: case PMS2DH2:	/* HVPS */
					size = PMS2_RECSIZE - ONE_WORD;
					break;

				case PMS2DG1: case PMS2DG2: /* GrayScale */
					size = 32000 - ONE_WORD;
					break;

				default:
					size = 0;
				}

			nbytes += read(infd, &buff[ONE_WORD], size);
			}

		if (AsyncFileEnabled && IsThisAnAsyncRecord(buff))
			WriteAsyncData(buff);
		}
	while (nbytes > 0 && *((ushort *)buff) != SDI_WORD);

	currentLR = 0;
	return(nbytes);

}	/* END FINDNEXTDATARECORD */

/* -------------------------------------------------------------------- */
IsThisAnAsyncRecord(short buff[])
{
	if (buff[0] == PMS2DC1 || buff[0] == PMS2DC2 ||
		buff[0] == PMS2DP1 || buff[0] == PMS2DP2 ||
		buff[0] == PMS2DG1 || buff[0] == PMS2DG2 ||
		buff[0] == PMS2DH1 || buff[0] == PMS2DH2)
		return(TRUE);
	else
		return(FALSE);

}	/* END ISTHISANASYNCRECORD */

/* END RECORDIO.C */
