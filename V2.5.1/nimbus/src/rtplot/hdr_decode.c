/*
-------------------------------------------------------------------------
OBJECT NAME:	hdr_decode.c

FULL NAME:		Decode Header

ENTRY POINTS:	DecodeHeader()

STATIC FNS:		in_hdr(), in_sdi(), in_irs(), in_ophir3(), in_pms1d(),
				in_pms1dv2(), locatePMS(), add_file_to_???TBL(),
				add_name_to_???TBL(), add_derived_names(), in_ins(),
				check_cal_coes()

DESCRIPTION:	Read header & add variables to appropriate table.  There
				are 3 major tables here:
					- SDI table
					- Raw table (From block probes)
					- Derived table

				This module is the brunt end interface between an ADS tape
				and the rest of nimbus.

INPUT:			Header filename.

OUTPUT:			sdi, nsdi, raw, nraw, derived, nderive
				(These globals are initialized in this file)

REFERENCES:		Header API (libhdr_api.a)
				OpenProjectFile(), AccessProjectFile()
				ReadTextFile(), FreeTextFile()
				SearchList(), SearchDERIVEFTNS()
				ReadStaticLags(), SetUpDependencies(), ReadDefaultsFile()
				SortTable()

REFERENCED BY:	winput.c, nimbus.c

NOTE:			This file is from hell.

COPYRIGHT:		University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include <stdlib.h>
#include <unistd.h>

#include "raf.h"
#include "define.h"
#include "decode.h"
#include "ctape.h"	/* ADS header API	*/
#include "amlib.h"



/* Global to ease parameter passing in this file */
static int	InertialSystemCount;
static int	probe_cnt = 0;
static long	start, rate;
static char	*item_type, location[NAMELEN];


static RAWTBL	*add_name_to_RAWTBL();
static DERTBL	*add_name_to_DERTBL();
static void	add_file_to_RAWTBL(), add_file_to_DERTBL(), in_hdr(), in_sdi(),
			in_irs(), in_ophir3(), in_pms1d(), in_pms1dv2(), in_ins();


extern int	Aircraft;

/* -------------------------------------------------------------------- */
int DecodeHeader(char header_file[])
{
	char	*vn;
	FILE	*fp;
	char	*loc, *p;
	int		infd;		/* fd of header source */

	nsdi = nraw = InertialSystemCount = 0;

	probe_cnt = 0;

	if ((infd = InitFlightHeader(header_file, LEAVE_OPEN)) == ERR)
		{
		sprintf(buffer, "Header decode failed, taperr = %d.", taperr);
		HandleError(buffer);
		return(ERR);
		}

	GetAircraft(&p);
	Aircraft = atoi(&p[1]);

	GetFlightNumber(&p);
	FlightNumber = atoi(p);

	GetProjectNumber(&ProjectNumber);

	sprintf(buffer, "%s/%s", ProjectDirectory, ProjectNumber);
	if (access(buffer, R_OK) == ERR)
		{
		sprintf(buffer, "No project directory for %s.", ProjectNumber);
		HandleError(buffer);
		return(ERR);
		}


	/* Extract ProjectName
	 */
	if ((fp = OpenProjectFile("%s/%s/ProjectName", "r", RETURN)) != NULL)
		{
		fgets(buffer, 512, fp);
		buffer[strlen(buffer)-1] = '\0';	/* Remove '\n'	*/

		ProjectName = (char *)GetMemory(strlen(buffer)+1);
		strcpy(ProjectName, buffer);
		sprintf(title, "%s - %s", ProjectName, ProjectNumber);
		fclose(fp);
		}
	else
		ProjectName = "";


	SetLookupSuffix((char *)NULL);


	/* This is the main loop, loop through all variables in header
	 */
	for (vn = GetFirst(); vn; vn = GetNext())
		{
		GetItemType(vn, &item_type);
		GetStart(vn, &start);

		if (GetRate(vn, &rate) == ERR)
			rate = 1;

		location[0] = '\0';

		/* Initialize variable/probe into appropriate table(s).
		 */
		if (!strcmp(item_type, HSKP_STR) ||
		    !strcmp(item_type, DIGOUT_STR) ||
		    !strcmp(item_type, SDI_STR))
			{
			in_sdi(vn); 
			continue;
			}


		if (!strcmp(item_type, HDR_STR))
			in_hdr(vn);
		else
		if (!strcmp(item_type, IRS_STR))
			{
			if (GetLocation(vn, &loc) ==  OK)
				{
				location[0] = '_';
				strcpy(&location[1], loc);
				}

			strcpy(buffer, item_type);
			strcat(buffer, location);
			in_irs(vn);
			}
		else
		if (!strcmp(item_type, UVHYG_STR))
			{
			rate = sizeof(struct Uv_blk) / sizeof(struct Uv_smp);
			add_file_to_RAWTBL(UVHYGNAMES);
			}
		else
		if (!strcmp(item_type, OPHIR3_STR))
			{
			in_ophir3(vn);
			}
		else
		if (!strcmp(item_type, LRNC_STR))
			{
			add_file_to_RAWTBL(LRNCNAMES);
			}
		else
		if (!strcmp(item_type, GPS_TRIM_STR) ||
		    !strcmp(item_type, GPS_TANS2_STR) ||
		    !strcmp(item_type, GPS_TANS3_STR))
			{
			add_file_to_RAWTBL(GPSTANSNAMES);
			}
		else
        if (!strcmp(item_type, PPS_GPS_STR))
            {
            add_file_to_RAWTBL(GPSPPSNAMES);
            }
        else
        if (!strcmp(item_type, HW_GPS_STR))
            {
            if (GetLocation(vn, &loc) ==  OK)
                {
                location[0] = '_';
                strcpy(&location[1], loc);
                }

            add_file_to_RAWTBL(GPSHWNAMES);
            }
        else
		if (!strcmp(item_type, DME_STR))
			{
			add_file_to_RAWTBL(DMENAMES);
			}
		else
        if (!strcmp(item_type, DPRES_STR))
            {
            rate = 5;
            add_name_to_RAWTBL("XPSFD");
            }
        else
		if (!strcmp(item_type, SER_STR))
			add_name_to_RAWTBL("SERIAL");
		}



{
/*
int	i;
for (i = 0; i < nsdi; ++i)
	printf("%-12s%5d\n", sdi[i]->name, sdi[i]->convertOffset);

for (i = 0; i < nraw; ++i)
	printf("%-12s%3d%5d\n", raw[i]->name, raw[i]->SampleRate, raw[i]->NRstart);

*/
}


	sdi[nsdi] = NULL;
	raw[nraw] = NULL;

	SortTable((char **)sdi, 0, nsdi - 1);
	SortTable((char **)raw, 0, nraw - 1);

	return(infd);

}	/* END DECODEHEADER */

/* -------------------------------------------------------------------- */
static void in_hdr(vn)
char	vn[];
{
	/* These items must be added to the tables first (i.e. don't
	 * add any variables above the main loop in the fn above).
	 */
	start = 0; rate = 1;
	add_name_to_RAWTBL("HOUR");
	add_name_to_RAWTBL("MINUTE");
	add_name_to_RAWTBL("SECOND");

}	/* END IN_HDR */

/* -------------------------------------------------------------------- */
static void in_sdi(vn)
char	vn[];
{
	SDITBL	*cp;
	char	*type;
	float	*f;

	if (strcmp(vn, "DUMMY") == 0)
		return;

	GetADStype(&type);

	/* By placing an SDI variable into the 'deriveftns[]' list
	 * (ninc/amlib.fns) the variable will automatically be placed
	 * into the 'raw' list instead of 'sdi' (i.e. if you have an SDI
	 * variable that requires special processing).
	 */
	if (SearchDERIVEFTNS(vn) != ERR)
		{
		RAWTBL	*rp;
		long	offset;

		rp = add_name_to_RAWTBL(vn);

		if (GetConversionOffset(vn, &(rp->convertOffset)) == ERR ||
			atoi(ProjectNumber) == 818)
			rp->convertOffset = 0.0;

		GetConversionFactor(vn, &(rp->convertFactor));
		GetOrder(vn, &(rp->order));
		GetCalCoeff(vn, &f);
		memcpy((char *)rp->cof, (char *)f, (int)sizeof(float) * rp->order);
		rp->order = check_cal_coes(rp->order, rp->cof);
		GetSampleOffset(vn, &offset);
		rp->ADSoffset = offset >> 1;

        if (strcmp(rp->name, "HGM232") == 0) {  /* Status bit for APN-232   */
            rp = add_name_to_RAWTBL("HGM232S");
            rp->ADSoffset = offset >> 1;
            }
		}


	/* Ok, it's strictly nth order polynomial, put it in the SDI table.
	 */
	if (nsdi == MAX_SDI)
		{
		fprintf(stderr, "MAX_SDI reached, modify in nimbus.h and recompile.\n");
		exit(1);
		}

	cp = sdi[nsdi++] = (SDITBL *)GetMemory(sizeof(SDITBL));

	if (GetConversionOffset(vn, &(cp->convertOffset)) == ERR)
		cp->convertOffset = 0.0;

	GetConversionFactor(vn, &(cp->convertFactor));
	GetSampleOffset(vn, &(cp->ADSoffset));
	GetOrder(vn, &(cp->order));
	GetType(vn, &type);

	strcpy(cp->name, vn);
	cp->SampleRate	= rate;
	cp->OutputRate	= LOW_RATE;
	cp->ADSstart	= start >> 1;
	cp->ADSoffset	>>= 1;
	cp->type		= type[0];
	cp->StaticLag	= 0;
	cp->SpikeSlope	= 0;
	cp->Average		= NULL;
	cp->Dirty		= FALSE;
	cp->Modulo		= NULL;
	cp->Output		= TRUE;
	cp->DependedUpon= FALSE;

	GetCalCoeff(vn, &f);
	memcpy((char *)cp->cof, (char *)f, (int)sizeof(float) * cp->order);
	cp->order = check_cal_coes(cp->order, cp->cof);

}	/* END IN_SDI */

/* -------------------------------------------------------------------- */
/* NOTE: this proc is assuming the primary IRS probe is stored in the
 * header first.  Currently this is LEFT PIT.  xbuild.c is responsible
 * for insuring this.
 */
static void in_irs(vn)
char	vn[];
{
	int		i;
	RAWTBL	*rp;
	char	*names[50];
	char	name[NAMELEN];

	if (InertialSystemCount == 0)
		location[0] = '\0';

	/* This is how I resolv conflicting names between IRS & INS
	 */
	SetLookupSuffix("_IRS");

	ReadTextFile(IRSNAMES, names);

	for (i = 0; names[i]; ++i)
		{
		sscanf(names[i], "%s %d", name, &rate);

		if ((rp = add_name_to_RAWTBL(name)) == (RAWTBL *)ERR)
			continue;
		}


	FreeTextFile(names);

	SetLookupSuffix((char *)NULL);

}	/* END IN_IRS */

/* -------------------------------------------------------------------- */
static void in_ins(vn)
char	vn[];		/* Variable Name	*/
{
	int		i;
	char	*names[50];
	char	name[NAMELEN];

	/* This is how I resolve conflicting names between IRS & INS
	 */
	SetLookupSuffix("_INS");

	ReadTextFile(INSNAMES, names);

	for (i = 0; names[i]; ++i)
		{
		sscanf(names[i], "%s %d", name, &rate);
		add_name_to_RAWTBL(name);
		}

	FreeTextFile(names);

	SetLookupSuffix((char *)NULL);

}	/* END IN_INS */

/* -------------------------------------------------------------------- */
static void in_ophir3(vn)
char	vn[];		/* Variable Name	*/
{
	RAWTBL	*rp;
	int		i, j;
	char	*list[32];

	ReadTextFile(OPHIR3NAMES, list);

	for (i = 0; list[i]; ++i)
		{
		if ((rp = add_name_to_RAWTBL(strtok(list[i], " \t"))) == (RAWTBL *)ERR)
			continue;

		rp->convertOffset	= 0.0;
		rp->convertFactor	= 1.0;
		rp->order			= atoi(strtok((char *)NULL, " \t"));

		for (j = 0; j < rp->order; ++j)
			rp->cof[j] = (float)atof(strtok((char *)NULL, " \t"));
		}

	FreeTextFile(list);

}	/* END IN_OPHIR3 */


/* -------------------------------------------------------------------- */
/* Header Decode Atomic functions										*/
/* -------------------------------------------------------------------- */
static void add_file_to_RAWTBL(filename)
char	filename[];
{
	FILE	*fp;

	fp = OpenProjectFile(filename, "r", EXIT);

	while (fscanf(fp, "%s", buffer) != EOF)
		if (buffer[0] != COMMENT)
			add_name_to_RAWTBL(buffer);

	fclose(fp);

}	/* END ADD_FILE_TO_RAWTBL */

/* -------------------------------------------------------------------- */
static RAWTBL *add_name_to_RAWTBL(name)
char	name[];
{
	int		indx;
	RAWTBL	*rp;

	if ((indx = SearchDERIVEFTNS(name)) == ERR)
		{
		sprintf(buffer, "Throwing away %s, has no decode function.\n", name);
		fprintf(stderr, buffer);
		return((RAWTBL *)ERR);
		}

	if (nraw == MAX_RAW)
		{
		fprintf(stderr, "MAX_RAW reached, modify in nimbus.h and recompile.");
		exit(1);
		}

	rp = raw[nraw++] = (RAWTBL *)GetMemory(sizeof(RAWTBL));

	strcpy(rp->name, name);
	if (*location)
		strcat(rp->name, location);

	rp->xlate			= deriveftns[indx].xlate;

	rp->ADSstart		= start >> 1;
	rp->ADSoffset		= 1;
	rp->SampleRate		= rate;
	rp->OutputRate		= LOW_RATE;
	rp->Length			= 1;
	rp->convertOffset	= 0.0;
	rp->convertFactor	= 1.0;
	rp->order			= 0;
	rp->StaticLag		= 0;
	rp->SpikeSlope		= 0;
	rp->DynamicLag		= 0;

	rp->Dirty			= FALSE;
	rp->Average			= NULL;
	rp->Modulo			= NULL;
	rp->Output			= TRUE;
	rp->DependedUpon	= FALSE;
	rp->ProbeType		= 0;

	return(rp);

}	/* END ADD_NAME_TO_RAWTBL */

/* -------------------------------------------------------------------- */
/* Strip out trailing 0 cal coe's
 */
check_cal_coes(order, coef)
int		order;
float	*coef;
{
	do
		{
		--order;
		}
	while (order >= 0 && coef[order] == 0.0);

	return(order + 1);

}	/* END CHECK_CAL_COES */

/* END HDR_DECODE.C */
