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

static const NR_TYPE shadowLevel = 0.55;


static int      FIRST_BIN[MAX_PMS2], LAST_BIN[MAX_PMS2];
static NR_TYPE  responseTime[MAX_PMS2], armDistance[MAX_PMS2], DENS[MAX_PMS2],
                resolution[MAX_PMS2];
static double   PLWFAC[MAX_PMS2], DBZFAC[MAX_PMS2];

static NR_TYPE  total_concen[MAX_PMS2], dbar[MAX_PMS2], plwc[MAX_PMS2],
                disp[MAX_PMS2], dbz[MAX_PMS2], tact[MAX_PMS2];

static NR_TYPE  radius[MAX_PMS2][BINS_64], cell_size[MAX_PMS2][BINS_64],
                eaw[MAX_PMS2][BINS_64]; /* Effective Sample Width       */

void    ComputePMS1DParams(NR_TYPE radius[], NR_TYPE eaw[], NR_TYPE cell_size[],                float minRange, float resolution, int nDiodes, int length),

        ComputeDOF(NR_TYPE radius[], NR_TYPE tas, NR_TYPE dof[], NR_TYPE shadowLevel, int FirstBin, int LastBin, float RES, NR_TYPE RESPONSE_TIME);


/* -------------------------------------------------------------------- */
void sTwodInit(RAWTBL *varp)
{
  int		i, j, length, probeNum, nDiodes, minRange;
  char		*p, *serialNumber;
  NR_TYPE	DOF, beamDiameter;


  /* This function unfortunalatly only gets called once, yet we need to
   * initialaize two probes (sort of) the 1D version and the 2D version.
   * This function is called from the A1D? xlTwodInit, not A2D?...
   */


  serialNumber = varp->SerialNumber;
  probeNum = varp->ProbeCount << 1;

  sprintf(buffer, PMS_SPEC_FILE, ProjectDirectory, ProjectNumber);
  InitPMSspecs(buffer);

  /* Perform twice, once for 1DC, and again for 2DC.
   */
  for (j = 0; j < 2; ++j, ++probeNum)
    {
    if ((p = GetPMSparameter(serialNumber, "FIRST_BIN")) == NULL) {
      printf("%s: FIRST_BIN not found.\n", serialNumber); exit(1);
      }
    FIRST_BIN[probeNum] = atoi(p);

    if ((p = GetPMSparameter(serialNumber, "LAST_BIN")) == NULL) {
      printf("%s: LAST_BIN not found.\n", serialNumber); exit(1);
      }
    LAST_BIN[probeNum] = atoi(p);

    if ((p = GetPMSparameter(serialNumber, "MIN_RANGE")) == NULL) {
      printf("%s: MIN_RANGE not found.\n", serialNumber); exit(1);
      }
    minRange = atoi(p);

    if ((p = GetPMSparameter(serialNumber, "RANGE_STEP")) == NULL) {
      printf("%s: RANGE_STEP not found.\n", serialNumber); exit(1);
      }
    resolution[probeNum] = atof(p);

    if ((p = GetPMSparameter(serialNumber, "NDIODES")) == NULL) {
      printf("%s: NDIODES not found.\n", serialNumber); exit(1);
      }
    nDiodes = atoi(p);

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

printf("sTwodInit: %s %d:\n", varp->name, probeNum);
    /* 1DC/P has length 32, 2DC/P has length 64.
     */
    length = varp->Length;
    if (j > 0)
      length += 32;

    ComputePMS1DParams(radius[probeNum], eaw[probeNum], cell_size[probeNum],
        minRange, resolution[probeNum], nDiodes, length);

    if (j == 0)  /* 2DC only (not 1DC). */
      {
printf("    2DC EAW for %s %d:\n", varp->name, probeNum);
      if (varp->name[3] == 'C')		/* EAW is fixed .8 for 2DC */
        for (i = 0; i < length; ++i)
          eaw[probeNum][i] = 0.8;
      else
      if (varp->name[3] == 'P')		/* EAW is fixed 6.4 for 2DP */
        for (i = 0; i < length; ++i)
          eaw[probeNum][i] = 6.4;
      }
    }

  ReleasePMSspecs();
printf(">>>>>>> WARNING: TWOD: DISP contains TACT, TAS HARD 33.8!!! <<<<<<<\n");

}	/* END STWODINIT */

/* -------------------------------------------------------------------- */
void sTwoD(DERTBL *varp)
{
  int		i, j, probeNum;
  NR_TYPE	*actual, *concentration, *dia;
  NR_TYPE	tas;		/* True Air Speed	*/
  NR_TYPE	sampleVolume[BINS_64];
  NR_TYPE	dof[BINS_64];

  actual	= GetVector(varp, 0, varp->Length);
  tas		= GetSampleFor1D(varp, 1);
  probeNum	= varp->ProbeCount;
//tas = 33.8;
  dia           = cell_size[probeNum];
  concentration = &AveragedData[varp->LRstart];
  tact[probeNum]= 0.0;

if(strncmp(varp->name, "XXXC2DC", 4) == 0)
{
tas = 0.0;
for (i = 0; i < 8; i++, tas += 20.0)
{
  ComputeDOF(radius[probeNum], tas, dof, shadowLevel, FIRST_BIN[probeNum],
        LAST_BIN[probeNum], resolution[probeNum], responseTime[probeNum]);

for (j = 1; j < 32; ++j)
  {
  if (dof[j] > armDistance[probeNum])
    dof[j] = armDistance[probeNum];
  printf("%f %f\n", radius[probeNum][j]*2000, dof[j]);
  }
}

exit(1);
}

  ComputeDOF(radius[probeNum], tas, dof, shadowLevel, FIRST_BIN[probeNum],
        LAST_BIN[probeNum], resolution[probeNum], responseTime[probeNum]);

if (tas > 100 && varp->name[3] == 'C')
  printf("%s %d -------\n", varp->name, varp->ProbeCount);

  for (i = FIRST_BIN[probeNum]; i < LAST_BIN[probeNum]; ++i)
    {
    tact[probeNum] += actual[i];

    if (dof[i] > armDistance[probeNum])
      dof[i] = armDistance[probeNum];

    sampleVolume[i] = tas * (dof[i] * eaw[probeNum][i]) * 0.001;
if (tas > 100 && varp->name[3] == 'C')
  printf("%f %f %f\n", sampleVolume[i], dof[i], eaw[probeNum][i]);
    }
if (tas > 100 && varp->name[3] == 'C') exit(1);

#define PLWC
#define DBZ

#include "pms1d_cv"

}	/* END STWOD */

/* -------------------------------------------------------------------- */
void sconc2(varp)
DERTBL  *varp;
{
  PutSample(varp, total_concen[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdisp2(varp)
DERTBL  *varp;
{
  PutSample(varp, tact[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void splwc2(varp)
DERTBL  *varp;
{
  PutSample(varp, plwc[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbz2(varp)
DERTBL  *varp;
{
  PutSample(varp, dbz[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbar2(varp)
DERTBL  *varp;
{
  PutSample(varp, dbar[varp->ProbeCount]);
}

/* END PMS2D.C */
