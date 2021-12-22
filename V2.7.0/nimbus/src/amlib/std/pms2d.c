/*
-------------------------------------------------------------------------
OBJECT NAME:	pms2d.c

FULL NAME:	Derived computations for the PMS2D C & P probes.

ENTRY POINTS:	sc2d()

STATIC FNS:	none

DESCRIPTION:	

DEPENDANCIES:	A2D?	- Actual FSSP data
		TAS	- True Air Speed

OUTPUT:		C2D?	- Concentrations
		CONC2	- Total concentration
		DISP2
		DBAR2
		PLWC2	- Liquid Water Content

REFERENCES:	none

REFERENCED BY:	Compute()

COPYRIGHT:	University Corporation for Atmospheric Research, 2000
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"
#include "pms.h"

static NR_TYPE	total_concen[MAX_PMS2], dbar[MAX_PMS2], plwc[MAX_PMS2];


/* -------------------------------------------------------------------- */
void sTwodInit(DERTBL *varp)
{
  int		i, probeNum;
  char		*p, *serialNumber;
  NR_TYPE	DOF, beamDiameter;

  probeNum = varp->ProbeCount;

  sprintf(buffer, PMS_SPEC_FILE, ProjectDirectory, ProjectNumber);
  InitPMSspecs(buffer);

 
  ReleasePMSspecs();

}	/* END CFSSPINIT */

/* -------------------------------------------------------------------- */
void sTwoD(DERTBL *varp)
{
  int		i, probeNum;
  NR_TYPE	*actual, *concentration, *dia;
  NR_TYPE	tas;		/* True Air Speed	*/
  NR_TYPE	sampleVolume[BINS_32];

  actual	= GetVector(varp, 0, BINS_32);
  tas		= GetSampleFor1D(varp, 1);
  probeNum	= varp->ProbeCount;



}	/* END SC2D */

/* -------------------------------------------------------------------- */
void sconc2(DERTBL *varp)
{
  PutSample(varp, total_concen[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void splwc2(DERTBL *varp)
{
  PutSample(varp, plwc[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbar2(DERTBL *varp)
{
  PutSample(varp, dbar[varp->ProbeCount]);
}

/* END PMS2D.C */
