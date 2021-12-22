/*
-------------------------------------------------------------------------
OBJECT NAME:	asc2cdf.c

FULL NAME:	ASCII to Nimbus-netCDF Low Rate

ENTRY POINTS:	main()

STATIC FNS:		

DESCRIPTION:	Translate ASCII file to Nimbus Low Rate netCDF file

INPUT:		ASCII file.

OUTPUT:		Nibmus netCDF low rate file.

REFERENCES:		

REFERENCED BY:	User

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-7
-------------------------------------------------------------------------
*/

#include "define.h"
#include "constants.h"

char	buffer[BUFFSIZE];

int	ncid;
FILE	*inFP;
int	timeOffsetID, varid[MAX_VARS], nVariables, nRecords;
time_t	BaseTime = 0;
float	scaleFactors[MAX_VARS], missingVals[MAX_VARS];
char	*time_vars[] = {"HOUR", "MINUTE", "SECOND"};

/* Command line option flags.
 */
bool	NASAfile = FALSE, sexSinceMidnight = FALSE;
int	SkipNlines = 1;
int	unitsLineNum = -1;
int	BaseDataRate = 1;

static int ProcessArgv(int argc, char **argv);
static void WriteMissingData(int, int);


/* -------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
  int	i, hour, minute, second, currSecond, lastSecond;
  int	startHour, startMinute, startSecond;
  char	*p;
  float	dataValue;

  i = ProcessArgv(argc, argv);

  if ((argc - i) < 2)
    {
    fprintf(stderr, "Usage: asc2cdf [-nasa] [-m] [-r n] [-s n] ascii_file output.cdf\n");
    exit(1);
    }

  if ((inFP = fopen(argv[i], "r")) == NULL)
    {
    fprintf(stderr, "Can't open %s.\n", buffer);
    exit(1);
    }


  if (nc_create(argv[i+1], NC_CLOBBER, &ncid) != NC_NOERR)
    {
    fprintf(stderr, "Can't destroy %s.\n", argv[2]);
    exit(1);
    }


  if (NASAfile)
    {
    CreateNASANetCDF(inFP);
    SetNASABaseTime();
    }
  else
    {
    CreatePlainNetCDF(inFP);
    SetPlainBaseTime();
    }

  printf("Base Data Rate = %d\n", BaseDataRate);


  /* Start uploading data.
   */
  rewind(inFP);

  while (SkipNlines--)
    fgets(buffer, BUFFSIZE, inFP);


  for (nRecords = 0; fgets(buffer, BUFFSIZE, inFP); )
    {
    if (strlen(buffer) < 2)
      continue;

    p = strtok(buffer, " \t");

    if (sexSinceMidnight)
      {
      currSecond = atof(p) + 0.5;

      hour = currSecond / 3600; currSecond -= hour * 3600;
      minute = currSecond / 60; currSecond -= minute * 60;
      second = currSecond;
      }
    else
      {
      hour = atoi(p);
      minute = atoi(&p[3]);
      second = atoi(&p[6]);
      }

    currSecond = hour * 3600 + minute * 60 + second;

    if (nRecords == 0)
      {
      startHour = hour;
      startMinute = minute;
      startSecond = second;
      }
    else
    if (currSecond == lastSecond)
      {
      printf("Duplicate time stamp, ignoring.\n");
      lastSecond = currSecond;
      continue;
      }
    else
    if (currSecond > lastSecond + BaseDataRate)
      {
      if (currSecond - lastSecond > 2)
        printf("last time = %d, new time = %d\n", lastSecond, currSecond);
      WriteMissingData(currSecond, lastSecond);
      }

    lastSecond = currSecond;

    dataValue = (float)(nRecords * BaseDataRate);
    nc_put_var1_float(ncid, timeOffsetID, &nRecords, &dataValue);
    dataValue = (float)hour;
    nc_put_var1_float(ncid, varid[0], &nRecords, &dataValue);
    dataValue = (float)minute;
    nc_put_var1_float(ncid, varid[1], &nRecords, &dataValue);
    dataValue = (float)second;
    nc_put_var1_float(ncid, varid[2], &nRecords, &dataValue);

    for (i = 0; i < nVariables; ++i)
      {
      p = strtok(NULL, " \t\n");
      dataValue = atof(p);

      if (NASAfile)
        if (dataValue == missingVals[i])
          dataValue = MISSING_VALUE;
        else
          dataValue *= scaleFactors[i];

      nc_put_var1_float(ncid, varid[i+3], &nRecords, &dataValue);
      }

    ++nRecords;
    }


  sprintf(buffer, "%02d:%02d:%02d-%02d:%02d:%02d",
          startHour, startMinute, startSecond, hour, minute, second);

  nc_put_att_text(ncid, NC_GLOBAL, "TimeInterval", strlen(buffer)+1, buffer);
  nc_close(ncid);
  chmod(argv[2], 0666);

  fclose(inFP);

  return(0);

}	/* END MAIN */

/* -------------------------------------------------------------------- */
static void WriteMissingData(int currSecond, int lastSecond)
{
  int	i, j, ts;
  int	hour, minute, second;
  float	dataValue;

  ts = (lastSecond += BaseDataRate);

  hour = ts / 3600; ts -= hour * 3600;
  minute = ts / 60; ts -= minute * 60;
  second = ts;

  for (i = lastSecond; i < currSecond; i += BaseDataRate)
    {
    dataValue = (float)(nRecords * BaseDataRate);
    nc_put_var1_float(ncid, timeOffsetID, &nRecords, &dataValue);
    dataValue = (float)hour;
    nc_put_var1_float(ncid, varid[0], &nRecords, &dataValue);
    dataValue = (float)minute;
    nc_put_var1_float(ncid, varid[1], &nRecords, &dataValue);
    dataValue = (float)second;
    nc_put_var1_float(ncid, varid[2], &nRecords, &dataValue);

    dataValue = MISSING_VALUE;

    for (j = 0; j < nVariables; ++j)
      nc_put_var1_float(ncid, varid[j+3], &nRecords, &dataValue);

    if ((second += BaseDataRate) > 59)
      {
      second = 0;
      if (++minute > 59)
        {
        minute = 0;
        if (++hour > 23)
          hour = 0;
        }
      }
    }

}	/* END WRITEMISSINGDATA */

/* -------------------------------------------------------------------- */
static int ProcessArgv(int argc, char **argv)
{
  int	i;

  for (i = 1; i < argc; ++i)
    {
    if (argv[i][0] != '-')
      break;

    switch (argv[i][1])
      {
      case 'b':
        BaseTime = atoi(argv[++i]);
        break;

      case 'm':
        sexSinceMidnight = TRUE;
        break;

      case 'n':
        NASAfile = TRUE;
        sexSinceMidnight = TRUE;
        break;

      case 'r':
        BaseDataRate = atoi(argv[++i]);
        break;

      case 's':
        SkipNlines = atoi(argv[++i]);
        break;

      case 'u':
        unitsLineNum = atoi(argv[++i]);
        break;

      default:
        fprintf(stderr, "Invalid option %s, ignoring.\n", argv[i]);
      }

    }

  return(i);

}	/* END PROCESSARGV */

/* END ASC2CDF.C */
