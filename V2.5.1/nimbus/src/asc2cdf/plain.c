/*
-------------------------------------------------------------------------
OBJECT NAME:	plain.c

FULL NAME:	Plain ASCII files.

ENTRY POINTS:	SetPlainBaseTime()
		CreatePlainNetCDF()

STATIC FNS:	none

DESCRIPTION:	

REFERENCES:	none

REFERENCED BY:	main()

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-7
-------------------------------------------------------------------------
*/

#include "define.h"
#include "constants.h"
#include "vardb.h"

static int	baseTimeID;


/* -------------------------------------------------------------------- */
void SetPlainBaseTime()
{
  nc_put_var_long(ncid, baseTimeID, &BaseTime);

}	/* END SETBASETIME */

/* -------------------------------------------------------------------- */
void CreatePlainNetCDF(FILE *fp)
{
  int	i, orate = 1;
  int	ndims, dims[3], TimeDim, LowRateDim, HighRateDim, Dim5Hz, Dim10Hz,
        Dim50Hz, Dim250Hz, Dim1000Hz, Vector16Dim, Vector32Dim, Vector64Dim;
  char	*p, *p1;
  float	missing_val = MISSING_VALUE;


  if (InitializeVarDB("/home/local/proj/defaults/VarDB") == ERR)
    {
    fprintf(stderr, "Can't open /home/local/proj/defaults/VarDB\n");
    exit(1);
    }


  /* Dimensions.
  */
  nc_def_dim(ncid, "Time", NC_UNLIMITED, &TimeDim);

  nc_def_dim(ncid, "sps1", 1, &LowRateDim);
/*  nc_def_dim(ncid, "sps5", 5, &Dim5Hz);
  nc_def_dim(ncid, "sps10", 10, &Dim10Hz);
  nc_def_dim(ncid, "sps25", 25, &HighRateDim);
  nc_def_dim(ncid, "sps50", 50, &Dim50Hz);
  nc_def_dim(ncid, "sps250", 250, &Dim250Hz);
*/
/*  nc_def_dim(ncid, "Vector16", 16, Vector16Dim);
  nc_def_dim(ncid, "Vector32", 32, Vector32Dim);
  nc_def_dim(ncid, "Vector64", 64, Vector64Dim);
*/

  /* Global Attributes.
   */
  strcpy(buffer, "NCAR-RAF/nimbus");
  nc_put_att_text(ncid, NC_GLOBAL, "Conventions", strlen(buffer)+1, buffer);


  {
  time_t	t;
  struct tm	tm;

  t = time(0);
  tm = *gmtime(&t);
  strftime(buffer, 128, "%h %d %R GMT %Y", &tm);
  nc_put_att_text(ncid, NC_GLOBAL, "DateConvertedFromASCII", 
                  strlen(buffer)+1, buffer);
  }


  /* Time segments.  Will be updated later.
   */
  nc_put_att_text(ncid, NC_GLOBAL, "TimeInterval", DEFAULT_TI_LENGTH, buffer);


  /* First dimension is time dimension.
   * Second is Rate in Hz.
   * Third is Vector Length.
   */
  ndims = 1;
  dims[0] = TimeDim;


  /* Time Variables, here to keep Gary/WINDS happy.
   */
  nc_def_var(ncid, "base_time", NC_INT, 0, 0, &baseTimeID);
  strcpy(buffer, "Seconds since Jan 1, 1970.");
  nc_put_att_text(ncid, baseTimeID, "long_name", strlen(buffer)+1, buffer);

  nc_def_var(ncid, "time_offset", NC_FLOAT, 1, dims, &timeOffsetID);
  strcpy(buffer, "Seconds since base_time.");
  nc_put_att_text(ncid, timeOffsetID, "long_name", strlen(buffer)+1, buffer);


  /* Create Time variables.
   */
  for (i = 0; i < 3; ++i)
    {
    nc_def_var(ncid, time_vars[i], NC_FLOAT, ndims, dims, &varid[i]);

    p = VarDB_GetUnits(time_vars[i]);
    nc_put_att_text(ncid, varid[i], "units", strlen(p)+1, p);
    p = VarDB_GetTitle(time_vars[i]);
    nc_put_att_text(ncid, varid[i], "long_name", strlen(p)+1, p);
    nc_put_att_int(ncid, varid[i], "OutputRate", NC_INT, 1, &orate);
    nc_put_att_float(ncid, varid[i], "MissingValue", NC_FLOAT, 1, &missing_val);
    }



  /* For each variable:
   *	- Set dimensions
   *	- define variable
   *	- Set attributes
   *	- Set data_pointer
   */
  while (strlen(fgets(buffer, BUFFSIZE, fp)) < 2)
    ;

  if (isdigit(buffer[0]))
    {
    fprintf(stderr, "plainASCII: no variable names, fatal.\n");
    exit(1);
    }

  /* Attempt to skip title for 'time' column.
   */
  if ((p = strtok(buffer, " \t\n")) == buffer)
    p = strtok(NULL, " \t\n");

  nVariables = 3;

  do
    {
    nc_def_var(ncid, p, NC_FLOAT, ndims, dims, &varid[nVariables]);

    p1 = VarDB_GetUnits(p);
    nc_put_att_text(ncid,varid[nVariables],"units", strlen(p1)+1, p1);
    p1 = VarDB_GetTitle(p);
    nc_put_att_text(ncid,varid[nVariables],"long_name",strlen(p1)+1, p1);
    nc_put_att_int(ncid,varid[nVariables], "OutputRate", NC_INT, 1, &orate);
    nc_put_att_float(ncid,varid[nVariables],"MissingValue",NC_FLOAT,1,&missing_val);

    ++nVariables;
    }
  while ((p = strtok(NULL, " \t\n")) );

  nVariables -= 3;

  nc_enddef(ncid);

  ReleaseVarDB();

}	/* END CREATEPLAINNETCDF */

/* END PLAIN.C */
