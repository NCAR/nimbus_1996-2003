/*
-------------------------------------------------------------------------
OBJECT NAME:	pms1d.c

FULL NAME:	PMS-1d related stuff

ENTRY POINTS:	AddPMS1dAttrs()
		AddToPMS1DprobeList()

STATIC FNS:	none

DESCRIPTION:	AddPMS1dAttrs() adds PMSspecs probe parameters to the
		netCDF file variable attributes.  The other function is
		stub for post-processing.

REFERENCES:	none

REFERENCED BY:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1996-2000
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "decode.h"
#include "pms.h"
#include "netcdf.h"

/* -------------------------------------------------------------------- */
void AddPMS1dAttrs(int ncid, RAWTBL *rp)
/* Add PMSspecs to netCDF attributes for probe, they go with accumulations */
{
  int	i, nBins;
  char	*p;
  float	cellSize[64];
  bool	warnMidPoints = FALSE;

  /* If not PMS1D, then bail out. */
  if (rp->ProbeType & 0xff000000 != 0x80000000)
    return;


  sprintf(buffer, PMS_SPEC_FILE, ProjectDirectory, ProjectNumber);
  InitPMSspecs(buffer);

  ncattput(ncid, rp->varid, "SerialNumber", NC_CHAR,
           strlen(rp->SerialNumber)+1, rp->SerialNumber);

  if ((p = GetPMSparameter(rp->SerialNumber, "FIRST_BIN")) ) {
    int	value = atoi(p);
    ncattput(ncid, rp->varid, "FirstBin", NC_INT, 1, &value);
    }

  if ((p = GetPMSparameter(rp->SerialNumber, "LAST_BIN")) ) {
    int	value = atoi(p) - 1;	/* Go from exclusive to inclusive */
    ncattput(ncid, rp->varid, "LastBin", NC_INT, 1, &value);
    }


  if ( (p = GetPMSparameter(rp->SerialNumber, "CELL_SIZE")) )
    {
    strcpy(buffer, p);
    p = strtok(buffer, " \t,");
 
    for (i = 0; p && i < 64; ++i)
      {
      cellSize[i] = atof(p);
      p = strtok(NULL, " \t,");
      }

    ncattput(ncid, rp->varid, "CellSizes", NC_FLOAT, i, cellSize);

    if (cellSize[0] == 0.0)
      warnMidPoints = TRUE;
    }


  nBins = 16;

  switch (rp->ProbeType & 0x00ffffff)
    {
    case PROBE_260X:
      nBins = 64;

    case PROBE_200X:
    case PROBE_200Y:
      {
      float	min = 0.0, max = 0.0, step = 0.0;

      if ((p = GetPMSparameter(rp->SerialNumber, "MIN_RANGE")) )
        min = atof(p);

      if ((p = GetPMSparameter(rp->SerialNumber, "MAX_RANGE")) )
        max = atof(p);

      if ((p = GetPMSparameter(rp->SerialNumber, "RANGE_STEP")) )
        step = atof(p);

      for (i = 0; i < nBins; min += step)
        cellSize[i++] = min;

      ncattput(ncid, rp->varid, "CellSizes", NC_FLOAT, i, cellSize);

      if (cellSize[0] == 0.0)
        warnMidPoints = TRUE;

      if ((p = GetPMSparameter(rp->SerialNumber, "NDIODES")) ) {
        int	value = atoi(p);
        ncattput(ncid, rp->varid, "nDiodes", NC_INT, 1, &value);
        }

      if ((p = GetPMSparameter(rp->SerialNumber, "MAG")) ) {
        float	value = atof(p);
        ncattput(ncid, rp->varid, "Magnification", NC_FLOAT, 1, &value);
        }

      if ((p = GetPMSparameter(rp->SerialNumber, "RESPONSE_TIME")) ) {
        float	value = atof(p);
        ncattput(ncid, rp->varid, "ResponseTime", NC_FLOAT, 1, &value);
        }

      if ((p = GetPMSparameter(rp->SerialNumber, "ARM_DISTANCE")) ) {
        float	value = atof(p);
        ncattput(ncid, rp->varid, "ArmDistance", NC_FLOAT, 1, &value);
        }
      break;
      }
    }


  ncattput(ncid, rp->varid, "CellSizeUnits", NC_CHAR, 12, "micrometers");


  if ((p = GetPMSparameter(rp->SerialNumber, "DOF")) ) {
    float	value = atof(p);
    ncattput(ncid, rp->varid, "DepthOfField", NC_FLOAT, 1, &value);
    }

  if ((p = GetPMSparameter(rp->SerialNumber, "BEAM_DIAM")) ) {
    float	value = atof(p);
    ncattput(ncid, rp->varid, "BeamDiameter", NC_FLOAT, 1, &value);
    }

  if ((p = GetPMSparameter(rp->SerialNumber, "DENS")) ) {
    float	value = atof(p);
    ncattput(ncid, rp->varid, "Density", NC_FLOAT, 1, &value);
    }

  if ((p = GetPMSparameter(rp->SerialNumber, "PLWFAC")) ) {
    float	value = atof(p);
    ncattput(ncid, rp->varid, "PLWfactor", NC_FLOAT, 1, &value);
    }

  if ((p = GetPMSparameter(rp->SerialNumber, "DBZFAC")) ) {
    float	value = atof(p);
    ncattput(ncid, rp->varid, "DBZfactor", NC_FLOAT, 1, &value);
    }

  if ((p = GetPMSparameter(rp->SerialNumber, "SAMPLE_AREA")) ) {
    float	value = atof(p);
    ncattput(ncid, rp->varid, "SampleArea", NC_FLOAT, 1, &value);
    }


  ReleasePMSspecs();


  /* Older projects have a PMSspecs file with mid-points instead of end-points.
   * Warn the user about this, as it will produce incorrect PMS1D results.
   */
  if (warnMidPoints)
    {
    fprintf(stderr, "PMSspecs file contains mid-point, not end-point cell diameters.\n");
    fprintf(stderr, "Nimbus was modified on 9/5/98 to use end-points.  Please fix.\n");
    exit(1);
    }

}

/* -------------------------------------------------------------------- */
void AddToPMS1DprobeList(
	char    probe[],
	char    location[],
	char    serial_num[],
	int     type)
{

/* This stub is called by nimbus.  winput/winds uses the version in pms1d_rt.c
 */


}   /* END ADDTOPMS1DPROBELIST */

/* PMS1D.C */
