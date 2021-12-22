/*
-------------------------------------------------------------------------
OBJECT NAME:	asas.c

FULL NAME:	Compute PMS1D ASAS/PCAS derived paramters

ENTRY POINTS:	scasas()

STATIC FNS:	none

DESCRIPTION:	

DEPENDANCIES:	none

OUTPUT:		

REFERENCES:	pms1d.c (ComputeConcentrations())

REFERENCED BY:	Compute()

NOTES:		Calculations taken from Bulletin 24 dated 1/89.

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"
#include "pms.h"

#define MAX_ASAS	2

static int FIRST_BIN[MAX_ASAS], LAST_BIN[MAX_ASAS];

static NR_TYPE	total_concen[MAX_ASAS], disp[MAX_ASAS], dbar[MAX_ASAS];
static NR_TYPE	aact[MAX_ASAS], pvol[MAX_ASAS];
static NR_TYPE	cell_size[MAX_ASAS][BINS_16];

static int	SampleRate;

/* -------------------------------------------------------------------- */
void casasInit(RAWTBL *varp)
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

  if ((p = GetPMSparameter(serialNumber, "CELL_SIZE")) == NULL) {
    printf("%s: CELL_SIZE not found.\n", serialNumber); exit(1);
    }

  strcpy(buffer, p);
  p = strtok(buffer, ", \t\n");

  for (i = 0; i < BINS_16; ++i)
    {
    cell_size[probeNum][i] = p ? atof(p) : 0.0;
    p = strtok(NULL, ", \t\n");
    }

  for (i = BINS_16-1; i > 0; --i)
    cell_size[probeNum][i] =
	(cell_size[probeNum][i] + cell_size[probeNum][i-1]) / 2;

  ReleasePMSspecs();

  SampleRate = varp->SampleRate;

}	/* END CASASINIT */

/* -------------------------------------------------------------------- */
void scasas(varp)
DERTBL	*varp;
{
  int		i, probeNum;
  NR_TYPE	*actual, *concentration, activity, *dia;
  NR_TYPE	flow;		/* PCAS Flow Rate	*/
  NR_TYPE	sampleVolume[BINS_16];

  actual	= GetVector(varp, 0, BINS_16);
  activity	= GetSample(varp, 1);
  flow		= GetSample(varp, 2);
  probeNum	= varp->ProbeCount;
  dia		= cell_size[probeNum];

  if (FeedBack == HIGH_RATE_FEEDBACK)
    {
    if (SampleOffset >= SampleRate)
      return;

    concentration = &HighRateData[varp->HRstart +(SampleOffset * varp->Length)];
    flow /= SampleRate;
    activity *= SampleRate;
    }
  else
    concentration = &AveragedData[varp->LRstart];


  for (i = FIRST_BIN[probeNum]; i < LAST_BIN[probeNum]; ++i)
    sampleVolume[i] = (1.0 - activity) * flow;

#define VOLUME

#include "pms1d_cv"

}	/* END SCASAS */

/* -------------------------------------------------------------------- */
void spflwc(varp)
DERTBL	*varp;
{
  NR_TYPE	flow, psx, atx, flowc;

  flow	= GetSample(varp, 0);
  psx	= GetSample(varp, 1);
  atx	= GetSample(varp, 2);

  flowc = flow * (1013.25 / psx) * (atx + 273.15) / 294.15;

  PutSample(varp, flowc);
}

/* -------------------------------------------------------------------- */
void saact(varp)
DERTBL	*varp;
{
  PutSample(varp, aact[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sconca(varp)
DERTBL	*varp;
{
  PutSample(varp, total_concen[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdispa(varp)
DERTBL	*varp;
{
  PutSample(varp, disp[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbara(varp)
DERTBL	*varp;
{
  PutSample(varp, dbar[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void spvolp(varp)
DERTBL	*varp;
{
  PutSample(varp, pvol[varp->ProbeCount]);
}

/* END ASAS.C */
