/*
-------------------------------------------------------------------------
OBJECT NAME:	ncbad.c

FULL NAME:	Replaces data with MISSING_VALUE

ENTRY POINTS:	main()

STATIC FNS:	none

DESCRIPTION:	Replaces a segment of data with -32767.0.

INPUT:		Nimbus netCDF file.

OUTPUT:		changes netCDF parameters
		
REFERENCES:	libraf.a, libnetcdf.a

REFERENCED BY:	User

COMPILE:
	gcc -O2 ncbad.c -o ncbad -lnetcdf -lraf

COPYRIGHT:	University Corporation for Atmospheric Research, 1999

Version 1:	programmed by Andre Prevot 11/16/1995
	2:	Cleaned up and made more generic Chris Webster 3/99
-------------------------------------------------------------------------
*/

#include <math.h>
#include <netcdf.h>
#include <stdio.h>

char	buffer[1024];		/* Generic volatile string space */


/* -------------------------------------------------------------------- */
main()
{
  int 	i, j;
  int	InputFile;
  int	hourID, minuteID, secondID, varID, nDims;
  short	*hour, *minute, *second;
  int	shour, sminute, ssecond, ehour, eminute, esecond, nstart, nend;
  int	nRecords, numberSeconds, dataRate;
  char	InputFileName[512];
  char	varName[25];
  float	*data;

  ncopts = 0;


  /* Query user for various information and Open files. */
  printf("\n\nEnter netCDF file name: (without an extension)\n");
  gets(InputFileName);
  strcat(InputFileName,".cdf");
			
  while (nc_open(InputFileName, NC_WRITE, &InputFile) != NC_NOERR)
    {
    printf("\n\nEnter netCDF file name: (without an extension)\n");
    gets(InputFileName);
    strcat(InputFileName,".cdf");
    }

  printf("\n\nEnter variable name: ");
  strupr(gets(varName));


  /* Init netCDF file. */
  nc_inq_unlimdim(InputFile, &nRecords);
  nc_inq_dimlen(InputFile, nRecords, &nRecords);

  if (nc_inq_varid(InputFile, varName, &varID) != NC_NOERR)
    {
    printf("Variable [%s] does not exist in file.\n", varName);
    return(1);
    }
  if (nc_inq_varid(InputFile, "HOUR", &hourID) != NC_NOERR)
    {
    printf("Variable HOUR does not exist in file.\n");
    return(1);
    }
  if (nc_inq_varid(InputFile, "MINUTE", &minuteID) != NC_NOERR)
    {
    printf("Variable MINUTE does not exist in file.\n");
    return(1);
    }
  if (nc_inq_varid(InputFile, "SECOND", &secondID) != NC_NOERR)
    {
    printf("Variable SECOND does not exist in file.\n");
    return(1);
    }

  printf("\n\nEnter start time [HHMMSS] :");
  gets(buffer);
  sscanf(buffer, "%02d%02d%02d", &shour, &sminute, &ssecond);

  printf("\n\nEnter end time [HHMMSS] :");
  gets(buffer);
  sscanf(buffer, "%02d%02d%02d", &ehour, &eminute, &esecond);

  if (ehour < shour)
    ehour = ehour + 24;

  numberSeconds = (ehour - shour) * 3600 + (eminute - sminute) * 60 + (esecond - ssecond);

  nc_inq_varndims(InputFile, varID, &nDims);

  if (nDims > 1)
    {
    int	ids[6];

    nc_inq_vardimid(InputFile, varID, ids);
    nc_inq_dimlen(InputFile, ids[1], &dataRate);
    }
  else
    dataRate = 1;

  /* Read all data at once and then scan array, and compute zero's
   */
  printf("\nReading data...");  fflush(stdout);

  data = (float *)GetMemory(sizeof(float) * nRecords * dataRate);
  hour = (short *)GetMemory(sizeof(short) * nRecords);
  minute = (short *)GetMemory(sizeof(short) * nRecords);
  second = (short *)GetMemory(sizeof(short) * nRecords);
					
  nc_get_var_float(InputFile, varID, data);
  nc_get_var_short(InputFile, hourID, hour);
  nc_get_var_short(InputFile, minuteID, minute);
  nc_get_var_short(InputFile, secondID, second);

  for (i = 0; i < nRecords; ++i)
    {
    if (hour[i] == shour && minute[i] == sminute && second[i] == ssecond)
      nstart = i;
    if (hour[i] == ehour && minute[i] == eminute && second[i] == esecond)
      nend = i;
    }

  nend = nstart + numberSeconds;
  printf("\nnstart : %d , nend : %d\n",nstart, nend);

  nstart *= dataRate;
  nend *= dataRate;

  for (i = nstart; i < nend; ++i)
    data[i] = -32767.0;

  printf("\nWriting data...");  fflush(stdout);

  nc_put_var_float(InputFile, varID, data); 
  nc_close(InputFile);

  putchar('\n');
  return(0);

}	/* END MAIN */

