/*
-------------------------------------------------------------------------
OBJECT NAME:	fssp.c

FULL NAME:	Derived computations for the PMS1D FSSP-100 probe

ENTRY POINTS:	cfsspInit()
		scs100()	DMT F100
		scfssp()	Original F100
		sfact()
		sfbmfr()

STATIC FNS:	none

DESCRIPTION:	

DEPENDANCIES:	AFSSP	- Actual FSSP data
		TAS	- True Air Speed
		ACT	- Activity
		FBMFR	- Beam Fraction

OUTPUT:		CFSSP	- Concentrations
		CONCF	- Total concentration
		DISPF
		DBARF
		PLWCF	- Liquid Water Content

REFERENCES:	pms1d.c

REFERENCED BY:	Compute()

NOTES:		Calculations taken from Bulletin 24 dated 1/89.

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"
#include "pms.h"

#define MAX_FSSP	4

static int	FIRST_BIN[MAX_FSSP], LAST_BIN[MAX_FSSP], SampleRate;
static double	PLWFAC[MAX_FSSP], DBZFAC[MAX_FSSP];
static NR_TYPE	sa[MAX_FSSP], tau1[MAX_FSSP], tau2[MAX_FSSP], vol[MAX_FSSP],
		fssp_csiz[MAX_FSSP][BINS_40*4+4], DENS[MAX_FSSP],
		tact[MAX_FSSP];

static NR_TYPE	total_concen[MAX_FSSP], dbar[MAX_FSSP], plwc[MAX_FSSP],
		disp[MAX_FSSP], dbz[MAX_FSSP];


/* -------------------------------------------------------------------- */
void cfsspInit(RAWTBL *varp)
{
  int		i, probeNum;
  char		*p, *serialNumber;
  NR_TYPE	DOF, beamDiameter;

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

  if ((p = GetPMSparameter(serialNumber, "DOF")) == NULL) {
    printf("%s: DOF not found.\n", serialNumber); exit(1);
    }
  DOF = atof(p);

  if ((p = GetPMSparameter(serialNumber, "BEAM_DIAM")) == NULL) {
    printf("%s: BEAM_DIAM not found.\n", serialNumber); exit(1);
    }
  beamDiameter = atof(p);

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

  if (varp->name[1] != 'S') /* DMT Mode probe */
    {
    if ((p = GetPMSparameter(serialNumber, "TAU1")) == NULL) {
      printf("%s: TAU1 not found.\n", serialNumber); exit(1);
      }
    tau1[probeNum] = atof(p);

    if ((p = GetPMSparameter(serialNumber, "TAU2")) == NULL) {
      printf("%s: TAU2 not found.\n", serialNumber); exit(1);
      }
    tau2[probeNum] = atof(p);
    }


  if ((p = GetPMSparameter(serialNumber, "CELL_SIZE")) == NULL) {
    sprintf(buffer, "CELL_SIZE_%d", varp->Length-1);
    if ((p = GetPMSparameter(serialNumber, buffer)) == NULL) {
      printf("%s: %s not found.\n", buffer, serialNumber); exit(1);
      }
    }

  strcpy(buffer, p);
  p = strtok(buffer, ", \t\n");

  for (i = 0; i < varp->Length*4; ++i)	/* 4 "ranges"	*/
    {       
    fssp_csiz[probeNum][i] = p ? atof(p) : 0.0;
    p = strtok(NULL, ", \t\n");
    }       

  for (i = varp->Length*4-1; i > 0; --i)
    fssp_csiz[probeNum][i] =
        (fssp_csiz[probeNum][i] + fssp_csiz[probeNum][i-1]) / 2;
 
  ReleasePMSspecs();

  SampleRate = varp->SampleRate;
  sa[probeNum] = DOF * beamDiameter;

}	/* END CFSSPINIT */

/* -------------------------------------------------------------------- */
void scfssp(DERTBL *varp)
{
  int		i, probeNum;
  NR_TYPE	*actual, *concentration, *dia;
  NR_TYPE	tas;		/* True Air Speed	*/
  NR_TYPE	activity;	/* Activity		*/
  NR_TYPE	fbmfr;		/* Beam Fraction	*/
  NR_TYPE	frange;
  NR_TYPE	sampleVolume[BINS_16];

  actual	= GetVector(varp, 0, varp->Length);
  tas		= GetSampleFor1D(varp, 1);
  activity	= GetSample(varp, 2);
  fbmfr		= GetSample(varp, 3);
  frange	= GetSample(varp, 4);
  probeNum	= varp->ProbeCount;

  if (FeedBack == HIGH_RATE_FEEDBACK)
    {
    if (SampleOffset >= SampleRate)
      return;

    concentration = &HighRateData[varp->HRstart +(SampleOffset * varp->Length)];
    tas /= SampleRate;
    activity *= SampleRate;
    }
  else
    {
    concentration = &AveragedData[varp->LRstart];
    }

  vol[probeNum] = tas * sa[probeNum] * fbmfr * (1.0 - (activity * 0.71));

  tact[probeNum] = 0.0;
  for (i = FIRST_BIN[probeNum]; i < LAST_BIN[probeNum]; ++i)
    {
    tact[probeNum] += actual[i];
    sampleVolume[i] = vol[probeNum];
    }

  dia = &fssp_csiz[probeNum][(int)frange * varp->Length];

#define PLWC
#define DBZ

#include "pms1d_cv"

}	/* END SCFSSP */

/* -------------------------------------------------------------------- */
void scs100(DERTBL *varp)
{
  int           i, probeNum;
  NR_TYPE       *actual, *concentration, *dia;
  NR_TYPE       tas;		/* True Air Speed		*/
  NR_TYPE       rejAT;		/* Rejected, Avg Transit	*/
  NR_TYPE       oflow;		/* Lost counts to overflow	*/
  NR_TYPE       frange;
  NR_TYPE       sampleVolume[BINS_40+1];

  actual	= GetVector(varp, 0, varp->Length);
  tas		= GetSampleFor1D(varp, 1);
  rejAT		= GetSample(varp, 2);
  oflow		= GetSample(varp, 3);
  frange	= GetSample(varp, 4);
  probeNum	= varp->ProbeCount;

  if (FeedBack == HIGH_RATE_FEEDBACK)
    {
    if (SampleOffset >= SampleRate)
      return;

    concentration = &HighRateData[varp->HRstart +(SampleOffset * varp->Length)];    tas /= SampleRate;
    }
  else
    {
    concentration = &AveragedData[varp->LRstart];
    }

  tact[probeNum] = 0.0;
  for (i = FIRST_BIN[probeNum]; i < LAST_BIN[probeNum]; ++i)
    tact[probeNum] += actual[i];

  vol[probeNum] = tas * sa[probeNum];

  if (tact[probeNum] > 0)
    vol[probeNum] *= (float)(tact[probeNum]) / (float)(tact[probeNum] + rejAT);

  for (i = FIRST_BIN[probeNum]; i < LAST_BIN[probeNum]; ++i)
    sampleVolume[i] = vol[probeNum];

  dia = &fssp_csiz[probeNum][(int)frange * varp->Length];

#define PLWC
#define DBZ

#include "pms1d_cv"

  if (oflow > 0 && tact[probeNum] > 0)
    {
    NR_TYPE ccc = (tact[probeNum]+oflow) / tact[probeNum];

    for (i = FIRST_BIN[probeNum]; i < LAST_BIN[probeNum]; ++i)
      concentration[i] *= ccc;

    total_concen[probeNum] *= ccc;
    }

}       /* END SCS100 */

/* -------------------------------------------------------------------- */
void sfact(DERTBL *varp)
{
  NR_TYPE	fstrob, freset;

  /* This function is for the old interface card only.  Activity is
   * a housekeeping param on the new interface card.
   */
  if (FeedBack == HIGH_RATE_FEEDBACK && SampleOffset >= varp->OutputRate)
    return;

  freset = GetSample(varp, 0);
  fstrob = GetSample(varp, 1);

  PutSample(varp, (fstrob * tau1[varp->ProbeCount] +
                   freset * tau2[varp->ProbeCount]));

}	/* END SFACT */

/* -------------------------------------------------------------------- */
void sfbmfr(DERTBL *varp)
{
  int		i;
  NR_TYPE	*afssp;
  NR_TYPE	fstrob, fbmfr = 0.0, total_actual = 0.0;

  static NR_TYPE prev_fbmfr = 0.4;

  if (FeedBack == HIGH_RATE_FEEDBACK && SampleOffset >= varp->OutputRate)
    return;

  afssp  = GetVector(varp, 0, 16);
  fstrob = GetSample(varp, 1);

  for (i = FIRST_BIN[varp->ProbeCount]; i < LAST_BIN[varp->ProbeCount]; ++i)
    total_actual += afssp[i];

  if (fstrob != 0.0)
    fbmfr = total_actual / fstrob;

  if (fstrob < 1.0 || total_actual <= 0.0)
    fbmfr = 0.4;

  if (fbmfr < .2 || fbmfr > 1.0) /* If it's out of range, repeat previous */
    fbmfr = prev_fbmfr;

  if (fstrob < 0.0)
    fbmfr = prev_fbmfr;

  prev_fbmfr = fbmfr;

  PutSample(varp, fbmfr);

}	/* END SFBMFR */

/* -------------------------------------------------------------------- */
void sconcf(DERTBL *varp)
{
  PutSample(varp, total_concen[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdispf(DERTBL *varp)
{
  PutSample(varp, disp[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void splwcf(DERTBL *varp)
{
  PutSample(varp, plwc[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbzf(DERTBL *varp)
{
  PutSample(varp, dbz[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbarf(DERTBL *varp)
{
  PutSample(varp, dbar[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void ssvolf(DERTBL *varp)
{
  PutSample(varp, vol[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sactf(DERTBL *varp)
{
  PutSample(varp, tact[varp->ProbeCount]);
}

/* END FSSP.C */
