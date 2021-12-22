/*
-------------------------------------------------------------------------
OBJECT NAME:	dap2cdf.c

FULL NAME:		DAP to Nimbus Low Rate

ENTRY POINTS:	main()

STATIC FNS:		

DESCRIPTION:	Translate DAP file to Nimbus Low Rate netCDF file

INPUT:			DAP file.

OUTPUT:			Nibmus netCDF low rate file.

REFERENCES:		

REFERENCED BY:	User

COPYRIGHT:		University Corporation for Atmospheric Research, 1994
-------------------------------------------------------------------------
*/

#include <netcdf.h>
#include <stdio.h>
#include <time.h>

#include "constants.h"
#include "file.h"
#include "vardb.h"

char	buffer[1024];

struct Daphead1	dhdr1;
struct Daphead2	dhdr2;
struct Dapdata	ddata;

int		ncid;
int		varid[2000];
void	*data_p[2000];
char	*time_vars[] = {"HOUR", "MINUTE", "SECOND"};

char	*getenv();

/* -------------------------------------------------------------------- */
main(argc, argv)
int		argc;
char	*argv[];
{
	int		i, indx;
	char	*dapdata, units[16];
	float	f;
	FILE	*hdr_fp, *data_fp;

	if (argc < 3)
		{
		fprintf(stderr, "Usage: dap2cdf KOF10 /home/local/data/kofc10.cdf\n");
		exit(1);
		}


	strupr(argv[1]);
	dapdata = getenv("DATA_DIR");

	sprintf(buffer, "%s/H%s", dapdata, argv[1]);

	if ((hdr_fp = fopen(buffer, "rb")) == NULL)
		{
		fprintf(stderr, "Can't open %s.\n", buffer);
		exit(1);
		}

	sprintf(buffer, "%s/D%s", dapdata, argv[1]);

	if ((data_fp = fopen(buffer, "rb")) == NULL)
		{
		fprintf(stderr, "Can't open %s.\n", buffer);
		exit(1);
		}


	if ((ncid = nccreate(argv[2], NC_CLOBBER)) == (-1))
		{
		fprintf(stderr, "Can't destroy %s.\n", argv[2]);
		exit(1);
		}


	fread(&dhdr1, sizeof(struct Daphead1), 1, hdr_fp);
	fread(&dhdr2, sizeof(struct Daphead2), 1, hdr_fp);


	if (dhdr1.ideltt != 1000)
		{
		fprintf(stderr, "dap2cdf only supports 1hz DAP data files.\n");
		ncclose(ncid);
		fclose(data_fp);
		fclose(hdr_fp);

		exit(1);
		}


	CreateNetCDF();



	for (i = 0; i < dhdr1.nmrecs; ++i)
		{
		fread((char *)&ddata, dhdr1.nwords * 4, 1, data_fp);

		if (i == 0)
			SetBaseTime();

		if (dhdr2.itmseg[0][0] > ddata.ihr)
			ddata.ihr += 24;

		*((float *)data_p[0]) = (float)i;
		*((float *)data_p[1]) = (float)ddata.ihr;
		*((float *)data_p[2]) = (float)ddata.imin;
		*((float *)data_p[3]) = (float)ddata.isec;

		ncrecput(ncid, (long)i, data_p);
		}

	ncclose(ncid);
	chmod(argv[2], 0666);

	fclose(hdr_fp);
	fclose(data_fp);

}	/* END MAIN */

/* -------------------------------------------------------------------- */
#define DEFAULT_TI_LENGTH	(19 * MAX_TIME_SLICES)
#define MISSING_VALUE		(-32767.0)

static int			baseTimeID;
static struct tm	StartFlight;
static float		TimeOffset = 0;


/* -------------------------------------------------------------------- */
SetBaseTime()
{
	char			*p;
	struct tm		*gt;
	time_t			BaseTime;


	StartFlight.tm_hour	= ddata.ihr;
	StartFlight.tm_min	= ddata.imin;
	StartFlight.tm_sec	= ddata.isec;
	StartFlight.tm_isdst = -1;

	BaseTime = mktime(&StartFlight);
	gt = gmtime(&BaseTime);
	StartFlight.tm_hour += StartFlight.tm_hour - gt->tm_hour;

	BaseTime = mktime(&StartFlight);
	ncvarput1(ncid, baseTimeID, NULL, (void *)&BaseTime);

	ncsync(ncid);

}	/* END SETBASETIME */

/* -------------------------------------------------------------------- */
CreateNetCDF()
{
	register int	i;

	int			timeOffsetID;
	int			ndims, dims[3],
				TimeDim,
				LowRateDim, HighRateDim, Dim5Hz, Dim10Hz, Dim50Hz, Dim250Hz,
				Dim1000Hz, Vector16Dim, Vector32Dim, Vector64Dim;
	int			j, indx, orate = 1;
	char		*p;
	float		missing_val = MISSING_VALUE;


	if (InitializeVarDB("/home/local/proj/defaults/VarDB") == ERR)
		{
		fprintf(stderr, "Can't open /home/local/proj/defaults/VarDB\n");
		exit(1);
		}


	/* Dimensions.
	 */
	TimeDim		= ncdimdef(ncid, "Time", NC_UNLIMITED);

	LowRateDim	= ncdimdef(ncid, "sps1", 1);
	Dim5Hz		= ncdimdef(ncid, "sps5", 5);
	Dim10Hz		= ncdimdef(ncid, "sps10", 10);
	HighRateDim	= ncdimdef(ncid, "sps25", 25);
	Dim50Hz		= ncdimdef(ncid, "sps50", 50);
	Dim250Hz	= ncdimdef(ncid, "sps250", 250);

	Vector16Dim	= ncdimdef(ncid, "Vector16", 16);
	Vector32Dim	= ncdimdef(ncid, "Vector32", 32);
	Vector64Dim	= ncdimdef(ncid, "Vector64", 64);


	/* Global Attributes.
	 */
	strcpy(buffer, "NCAR Research Aviation Facility");
	ncattput(ncid,NC_GLOBAL,"Source", NC_CHAR, strlen(buffer)+1,(void *)buffer);

	strcpy(buffer, "P.O. Box 3000, Boulder, CO 80307-3000");
	ncattput(ncid,NC_GLOBAL,"Address", NC_CHAR, strlen(buffer)+1,(void *)buffer);

	strcpy(buffer, "(303) 497-1030");
	ncattput(ncid,NC_GLOBAL,"Phone", NC_CHAR, strlen(buffer)+1,(void *)buffer);

	strcpy(buffer, "NCAR-RAF/nimbus");
	ncattput(ncid, NC_GLOBAL, "Conventions", NC_CHAR,
										strlen(buffer)+1, (void *)buffer);


	strcpy(buffer, "This file contains PRELIMINARY DATA that are NOT to be used for critical analysis.");

	ncattput(ncid, NC_GLOBAL, "WARNING", NC_CHAR,
										strlen(buffer)+1, (void *)buffer);


	{
	time_t		t;
	struct tm	tm;

	t = time(0);
	tm = *gmtime(&t);
	strftime(buffer, 128, "%h %d %R GMT %Y", &tm);
	ncattput(ncid, NC_GLOBAL, "DateConvertedFromDAP", NC_CHAR,
										strlen(buffer)+1, (void *)buffer);
	}


	strncpy(buffer, dhdr1.iarcft, 4); buffer[4] = '\0';
	ncattput(ncid, NC_GLOBAL, "Aircraft", NC_CHAR, strlen(buffer)+1, (void *)buffer);

	sprintf(buffer, "%d", dhdr1.iproj);
	ncattput(ncid, NC_GLOBAL, "ProjectNumber", NC_CHAR, strlen(buffer)+1, (void *)buffer);

	sprintf(buffer, "%d", dhdr1.iflght);
	ncattput(ncid, NC_GLOBAL, "FlightNumber", NC_CHAR, strlen(buffer)+1, (void *)buffer);

	StartFlight.tm_mon = dhdr1.idated[1];
	StartFlight.tm_mday = dhdr1.idated[2];
	StartFlight.tm_year = dhdr1.idated[0];

	sprintf(buffer, "%02d/%02d/%d", dhdr1.idated[0], dhdr1.idated[1], dhdr1.idated[2]);
	ncattput(ncid, NC_GLOBAL, "FlightDate", NC_CHAR, strlen(buffer)+1, (void *)buffer);


	/* Time segments.
	 */
	buffer[0] = '\0';

	for (i = 0; i < dhdr1.ntmseg; ++i)
		{
		char	temp[50];

		sprintf(temp, "%02d:%02d:%02d-%02d:%02d:%02d", dhdr2.itmseg[i][0],
			dhdr2.itmseg[i][1], dhdr2.itmseg[i][2], dhdr2.itmseg[i][3],
			dhdr2.itmseg[i][4], dhdr2.itmseg[i][5]);

		if (i > 0)
			strcat(buffer, ", ");

		strcat(buffer, temp);
		}

	ncattput(ncid, NC_GLOBAL, "TimeInterval", NC_CHAR, strlen(buffer)+1,
														 (void *)buffer);


	/* First dimension is time dimension.
	 * Second is Rate in Hz.
	 * Third is Vector Length.
	 */
	ndims = 1;
	dims[0] = TimeDim;


	/* Time Variables, here to keep Gary/WINDS happy.
	 */
	baseTimeID = ncvardef(ncid, "base_time", NC_LONG, 0, 0);
	strcpy(buffer, "Seconds since Jan 1, 1970.");
	ncattput(ncid, baseTimeID, "long_name", NC_CHAR, strlen(buffer)+1, buffer);
	StartFlight.tm_mon -= 1;

	timeOffsetID = ncvardef(ncid, "time_offset", NC_FLOAT, 1, dims);
	strcpy(buffer, "Seconds since base_time.");
	ncattput(ncid, timeOffsetID, "long_name", NC_CHAR, strlen(buffer)+1,buffer);
	data_p[0] = (float *)malloc(sizeof(float));


	/* Create Time variables.
	 */
	for (i = 0; i < 3; ++i)
		{
		varid[i] = ncvardef(ncid, time_vars[i], NC_FLOAT, ndims, dims);

		p = VarDB_GetUnits(time_vars[i]);
		ncattput(ncid, varid[i], "units", NC_CHAR, strlen(p)+1, p);
		p = VarDB_GetTitle(time_vars[i]);
		ncattput(ncid, varid[i], "long_name", NC_CHAR, strlen(p)+1, p);
		ncattput(ncid, varid[i], "OutputRate", NC_LONG, 1, &orate);
		ncattput(ncid, varid[i], "MissingValue", NC_FLOAT, 1, &missing_val);

		data_p[i+1] = (float *)malloc(sizeof(float));
		}



	/* For each variable:
	 *	- Set dimensions
	 *	- define variable
	 *	- Set attributes
	 *	- Set data_pointer
	 */
	for (i = 0; i < dhdr1.nvar; ++i)
		{
		strncpy(buffer, dhdr1.names[i], 6);

		for (p = &buffer[5]; *p == ' '; --p)
			;

		*(p+1) = '\0';

		varid[i+3] = ncvardef(ncid, buffer, NC_FLOAT, ndims, dims);

		p = VarDB_GetUnits(buffer);
		ncattput(ncid, varid[i+3], "units", NC_CHAR, strlen(p)+1, p);
		p = VarDB_GetTitle(buffer);
		ncattput(ncid, varid[i+3], "long_name", NC_CHAR, strlen(p)+1, p);
		ncattput(ncid, varid[i+3], "OutputRate", NC_LONG, 1, &orate);
		ncattput(ncid, varid[i+3], "MissingValue", NC_FLOAT, 1, &missing_val);

		data_p[i+4] = &ddata.values[i];
		}

	ncendef(ncid);

	ReleaseVarDB();

}	/* END CREATENETCDF */

/* END DAP2CDF.C */
