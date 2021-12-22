/*
-------------------------------------------------------------------------
OBJECT NAME:	f300.c

FULL NAME:	Derived computation sfor the PMS1D FSSP-300 probe

ENTRY POINTS:	scf300()
		sconc3()
		sdbar3()

STATIC FNS:	none

DESCRIPTION:	

DEPENDANCIES:	AFSSP	- Actual FSSP data
		TAS	- True Air Speed
		ACT	- Activity
		FBMFR	- Beam Fraction

OUTPUT:		CF300	- Concentrations
		CONCF	- Total concentration
		DBARF

REFERENCES:	pms1d.c

REFERENCED BY:	Compute()

NOTES:		Calculations taken from Bulletin 24 dated 1/89.

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"
#include "pms.h"

#define MAX_F300	4

static int FIRST_BIN[MAX_F300], LAST_BIN[MAX_F300], SampleRate[MAX_F300];

static NR_TYPE	total_concen[MAX_F300], dbar[MAX_F300], disp[MAX_F300];
static NR_TYPE	cell_size[MAX_F300][32], pvol[MAX_F300], SAMPLE_AREA[MAX_F300];

/* -------------------------------------------------------------------- */
void cf300Init(RAWTBL *varp)
{
  int	i, probeNum;
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

  if ((p = GetPMSparameter(serialNumber, "SAMPLE_AREA")) == NULL)
    SAMPLE_AREA[probeNum] = 0.065;	/* Sample Area taken from cal in 1994 */
  else
    SAMPLE_AREA[probeNum] = atof(p);

  if ((p = GetPMSparameter(serialNumber, "CELL_SIZE")) == NULL) {
    printf("%s: CELL_SIZE not found.\n", serialNumber); exit(1);
    }

  strcpy(buffer, p);
  p = strtok(buffer, ", \t\n");

  for (i = 0; i < varp->Length; ++i)
    {
    cell_size[probeNum][i] = p ? atof(p) : 0.0;
    p = strtok(NULL, ", \t\n");
    }

  for (i = varp->Length-1; i > 0; --i)
    cell_size[probeNum][i] =
        (cell_size[probeNum][i] + cell_size[probeNum][i-1]) / 2;
 
  ReleasePMSspecs();

  SampleRate[probeNum] = varp->SampleRate;

}	/* END CF300INIT */

/* -------------------------------------------------------------------- */
void scf300(varp)
DERTBL	*varp;
{
  int		i, probeNum;
  NR_TYPE	*actual, *concentration, *dia;
  NR_TYPE	tas, vol, sampleVolume[BINS_40+1];

  actual	= GetVector(varp, 0, varp->Length);
  tas		= GetSampleFor1D(varp, 1);
  probeNum	= varp->ProbeCount;
  dia		= cell_size[probeNum];

  if (FeedBack == HIGH_RATE_FEEDBACK)
    {
    if (SampleOffset >= SampleRate[probeNum])
      return;

    concentration = &HighRateData[varp->HRstart +(SampleOffset * varp->Length)];
    tas /= SampleRate[probeNum];
    }
  else
    concentration = &AveragedData[varp->LRstart];


  vol = tas * SAMPLE_AREA[probeNum];

  for (i = FIRST_BIN[probeNum]; i < LAST_BIN[probeNum]; ++i)
    sampleVolume[i] = vol;

#define VOLUME

#include "pms1d_cv"

}	/* END SCF300 */

/* -------------------------------------------------------------------- */
void sconc3(varp)
DERTBL	*varp;
{
  PutSample(varp, total_concen[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbar3(varp)
DERTBL	*varp;
{
  PutSample(varp, dbar[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void spvol3(varp)
DERTBL	*varp;
{
  PutSample(varp, pvol[varp->ProbeCount]);
}

/* END F300.C */
