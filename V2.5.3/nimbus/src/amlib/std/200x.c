/*
-------------------------------------------------------------------------
OBJECT NAME:	200x.c

FULL NAME:	Compute derived paramters for PMS1D 200X probe.

ENTRY POINTS:	sc200x(), sc200cInit()

STATIC FNS:	none

DESCRIPTION:	

OUTPUT:		

REFERENCES:	pms1d.c (ComputePMS1DParams(), ComputeDOF(),
		ComputeConcentrations())

REFERENCED BY:	ComputeDerived()

NOTES:		Calculations taken from Bulletin 24 dated 1/89.

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"
#include "pms.h"

#define MAX_200X	1

static int	FIRST_BIN[MAX_200X], LAST_BIN[MAX_200X], SampleRate;
static NR_TYPE	responseTime, armDistance;
static double	DENS[MAX_200X], PLWFAC[MAX_200X];

static NR_TYPE	total_concen[MAX_200X], dbar[MAX_200X], plwc[MAX_200X],
		disp[MAX_200X];

static NR_TYPE	radius[BINS_16],
		dia[BINS_16],
		esw[BINS_16];	/* Effective Sample Width	*/


void	ComputePMS1DParams(NR_TYPE radius[], NR_TYPE esw[], int resolution,
		NR_TYPE MAG, int nDiodes, int Firstbin, int LastBin),
	ComputeDOF(NR_TYPE radius[], NR_TYPE tasx, NR_TYPE dof[],
		int FirstBin, int LastBin, NR_TYPE RESPONSE_TIME);

/* -------------------------------------------------------------------- */
void c200xInit(RAWTBL *varp)
{
	int	i, probeNum;
	int	nDiodes, resolution, MAG;
	char	*p, *serialNumber;

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

	if ((p = GetPMSparameter(serialNumber, "RANGE_STEP")) == NULL) {
		printf("%s: RANGE_STEP not found.\n", serialNumber); exit(1);
		}
	resolution = atoi(p);

	if ((p = GetPMSparameter(serialNumber, "MAG")) == NULL) {
		printf("%s: MAG not found.\n", serialNumber); exit(1);
		}
	MAG = atof(p);

	if ((p = GetPMSparameter(serialNumber, "RESPONSE_TIME")) == NULL) {
		printf("%s: RESPONSE_TIME not found.\n", serialNumber); exit(1);
		}
	responseTime = atof(p);

	if ((p = GetPMSparameter(serialNumber, "ARM_DISTANCE")) == NULL) {
		printf("%s: ARM_DISTANCE not found.\n", serialNumber); exit(1);
		}
	armDistance = atof(p);

	if ((p = GetPMSparameter(serialNumber, "DENS")) == NULL) {
		printf("%s: DENS not found.\n", serialNumber); exit(1);
		}
	DENS[probeNum] = atof(p);

	if ((p = GetPMSparameter(serialNumber, "PLWFAC")) == NULL) {
		printf("%s: PLWFAC not found.\n", serialNumber); exit(1);
		}
	PLWFAC[probeNum] = atof(p);

	ReleasePMSspecs();

	SampleRate = varp->SampleRate;

	ComputePMS1DParams(radius, esw, resolution, MAG, nDiodes,
		FIRST_BIN[probeNum], LAST_BIN[probeNum]);

	for (i = FIRST_BIN[probeNum]; i < LAST_BIN[probeNum]; ++i)
		dia[i] = (NR_TYPE)i * resolution;

}	/* END CONSTRUCTOR */

/* -------------------------------------------------------------------- */
void sc200x(DERTBL *varp)
{
  int		i, probeNum;
  NR_TYPE	*actual, *concentration, tasx;
  NR_TYPE	dof[BINS_16],	/* Depth Of Field		*/
		sv[BINS_16];	/* Sample Volume		*/

  /* NOTE: -1 index offset to compensate for undersizeing.
   */
  actual	= GetVector(varp, 0, 16) - 1;
  tasx		= GetSampleFor1D(varp, 1);
  probeNum	= varp->ProbeCount;

  if (FeedBack == HIGH_RATE_FEEDBACK)
    {
    if (SampleOffset >= SampleRate)
      return;

    tasx /= SampleRate;
    concentration = &HighRateData[varp->HRstart + (SampleOffset *varp->Length)];
    }
  else
    concentration = &AveragedData[varp->LRstart];


  ComputeDOF(radius, tasx, dof, FIRST_BIN[probeNum], LAST_BIN[probeNum], responseTime);

  for (i = FIRST_BIN[probeNum]; i < LAST_BIN[probeNum]; ++i)
    {
    if (dof[i] > armDistance)
      dof[i] = armDistance;

    sv[i] = tasx * (dof[i] * esw[i]);
    }

#define PLWC

#include "pms1d_cv"

}	/* END SC200X */

/* -------------------------------------------------------------------- */
void sconcx(DERTBL *varp)
{
  PutSample(varp, total_concen[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdispx(DERTBL *varp)
{
  PutSample(varp, disp[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void splwcx(DERTBL *varp)
{
  PutSample(varp, plwc[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbarx(DERTBL *varp)
{
  PutSample(varp, dbar[varp->ProbeCount]);
}

/* END 200X.C */
