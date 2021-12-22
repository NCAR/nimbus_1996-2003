/*
-------------------------------------------------------------------------
OBJECT NAME:	260x.c

FULL NAME:	Compute derived paramters for PMS1D 260X probe.

ENTRY POINTS:	sc260x()

STATIC FNS:	none

DESCRIPTION:	

OUTPUT:		

REFERENCES:	pms1d.c (ComputePMS1DParams(), ComputeDOF(),
			ComputeConcentrations())

REFERENCED BY:	Compute()

NOTES:		Calculations taken from Bulletin 24 dated 1/89.

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-8
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"
#include "pms.h"

#define MAX_260X	2

static int	FIRST_BIN[MAX_260X], LAST_BIN[MAX_260X], SampleRate;
static NR_TYPE	responseTime[MAX_260X], armDistance[MAX_260X], DENS[MAX_260X],
		resolution[MAX_260X];
static double	PLWFAC[MAX_260X], DBZFAC[MAX_260X];

static NR_TYPE	total_concen[MAX_260X], dbar[MAX_260X], plwc[MAX_260X],
		disp[MAX_260X], dbz[MAX_260X];

static NR_TYPE	radius[MAX_260X][BINS_64], cell_size[MAX_260X][BINS_64],
		esw[MAX_260X][BINS_64];	/* Effective Sample Width	*/

void    ComputePMS1DParams(NR_TYPE radius[], NR_TYPE esw[], NR_TYPE cell_size[],
		float minRange, float resolution, int nDiodes, int length),

	ComputeDOF(NR_TYPE radius[], NR_TYPE tas, NR_TYPE dof[],
		int FirstBin, int LastBin, float RES, NR_TYPE RESPONSE_TIME);


/* -------------------------------------------------------------------- */
void c260xInit(RAWTBL *varp)
{
  int	i, probeNum;
  int	nDiodes;
  char	*p, *serialNumber;
  float	minRange;

  serialNumber = varp->SerialNumber;
  probeNum = varp->ProbeCount;

  sprintf(buffer, PMS_SPEC_FILE, ProjectDirectory, ProjectNumber);
  InitPMSspecs(buffer);

  if ((p = GetPMSparameter(serialNumber, "FIRST_BIN")) == NULL) {
    printf("%s: FIRST_BIN not found.\n", serialNumber); exit(1);
    }
  FIRST_BIN[probeNum] = atoi(p);

  if ((p = GetPMSparameter(serialNumber, "LAST_BIN")) == NULL) {
    printf("%s: LAST_BIN not found.\n", serialNumber); exit(1);
    }
  LAST_BIN[probeNum] = atoi(p);

  if ((p = GetPMSparameter(serialNumber, "NDIODES")) == NULL) {
    printf("%s: NDIODES not found.\n", serialNumber); exit(1);
    }
  nDiodes = atoi(p);

  if ((p = GetPMSparameter(serialNumber, "MIN_RANGE")) == NULL) {
    printf("%s: MIN_RANGE not found.\n", serialNumber); exit(1);
    }
  minRange = atof(p);

  if ((p = GetPMSparameter(serialNumber, "RANGE_STEP")) == NULL) {
    printf("%s: RANGE_STEP not found.\n", serialNumber); exit(1);
    }
  resolution[probeNum] = atof(p);

  if ((p = GetPMSparameter(serialNumber, "RESPONSE_TIME")) == NULL) {
    printf("%s: RESPONSE_TIME not found.\n", serialNumber); exit(1);
    }
  responseTime[probeNum] = atof(p);

  if ((p = GetPMSparameter(serialNumber, "ARM_DISTANCE")) == NULL) {
    printf("%s: ARM_DISTANCE not found.\n", serialNumber); exit(1);
    }
  armDistance[probeNum] = atof(p);

  if ((p = GetPMSparameter(serialNumber, "DENS")) == NULL) {
    printf("%s: DENS not found.\n", serialNumber); exit(1);
    }
  DENS[probeNum] = atof(p);

  if ((p = GetPMSparameter(serialNumber, "PLWFAC")) == NULL) {
    printf("%s: PLWFAC not found.\n", serialNumber); exit(1);
    }
  PLWFAC[probeNum] = atof(p);

  if ((p = GetPMSparameter(serialNumber, "DBZFAC")) == NULL) {
    printf("%s: DBZFAC not found.\n", serialNumber); exit(1);
    }
  DBZFAC[probeNum] = atof(p);

  ReleasePMSspecs();

  SampleRate = varp->SampleRate;

  ComputePMS1DParams(radius[probeNum], esw[probeNum], cell_size[probeNum],
	minRange, resolution[probeNum], nDiodes, varp->Length);

}	/* END CONSTRUCTOR */

/* -------------------------------------------------------------------- */
void sc260x(varp)
DERTBL	*varp;
{
  int		i, probeNum;
  NR_TYPE	*actual, tas, *concentration, *dia;
  NR_TYPE	dof[BINS_64],	/* Depth Of Field	*/
		sampleVolume[BINS_64];

  actual	= GetVector(varp, 0, BINS_64);
  tas		= GetSampleFor1D(varp, 1);
  probeNum	= varp->ProbeCount;
  dia		= cell_size[probeNum];

  ComputeDOF(radius[probeNum], tas, dof, FIRST_BIN[probeNum],
	LAST_BIN[probeNum], resolution[probeNum], responseTime[probeNum]);

  if (FeedBack == HIGH_RATE_FEEDBACK)
    {
    if (SampleOffset >= SampleRate)
      return;

    tas /= SampleRate;
    concentration = &HighRateData[varp->HRstart + (SampleOffset*varp->Length)];
    }
  else
    concentration = &AveragedData[varp->LRstart];


  for (i = FIRST_BIN[probeNum]; i < LAST_BIN[probeNum]; ++i)
    {
    if (dof[i] > armDistance[probeNum])
      dof[i] = armDistance[probeNum];

    sampleVolume[i] = tas * (dof[i] * esw[probeNum][i]) * 0.001;
    }

#define PLWC
#define DBZ

#include "pms1d_cv"

}	/* END SC260X */

/* -------------------------------------------------------------------- */
void sconc6(varp)
DERTBL	*varp;
{
  PutSample(varp, total_concen[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdisp6(varp)
DERTBL	*varp;
{
  PutSample(varp, disp[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void splwc6(varp)
DERTBL	*varp;
{
  PutSample(varp, plwc[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbz6(varp)
DERTBL	*varp;
{
  PutSample(varp, dbz[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbar6(varp)
DERTBL	*varp;
{
  PutSample(varp, dbar[varp->ProbeCount]);
}

/* END 260X.C */
