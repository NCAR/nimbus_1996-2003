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

#define MAX_F300	2

static int FIRST_BIN[MAX_F300], LAST_BIN[MAX_F300];

static NR_TYPE	total_concen[MAX_F300], dbar[MAX_F300], disp[MAX_F300];
static NR_TYPE	cell_size[MAX_F300][32];

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

	if ((p = GetPMSparameter(serialNumber, "CELL_SIZE")) == NULL) {
		printf("%s: CELL_SIZE not found.\n", serialNumber); exit(1);
		}

	strcpy(buffer, p);
	p = strtok(buffer, ", \t\n");

	for (i = 0; i < BINS_32; ++i)
		{
		cell_size[probeNum][i] = p ? atof(p) : 0.0;
		p = strtok(NULL, ", \t\n");
		}

	ReleasePMSspecs();

}	/* END CF300INIT */

/* -------------------------------------------------------------------- */
void scf300(varp)
DERTBL	*varp;
{
	int	i, probeNum;
	NR_TYPE	*actual, *concentration, *dia;
	NR_TYPE	tas;		/* True Air Speed	*/
	NR_TYPE	vol, sv[32];	/* Sample Volume	*/

	actual	= GetVector(varp, 0, 32);
	tas	= GetSample(varp, 1);
	probeNum= varp->ProbeCount;
	dia	= cell_size[probeNum];

	if (FeedBack == HIGH_RATE_FEEDBACK)
		{
		if (SampleOffset >= varp->OutputRate)
			return;

		tas /= varp->OutputRate;
		concentration = &HighRateData[varp->HRstart +
						(SampleOffset * varp->Length)];
		}
	else
		concentration = &AveragedData[varp->LRstart];


	vol = tas * 0.065;	/* Sample Area taken from cal in 1994 */

	for (i = FIRST_BIN[probeNum]; i < LAST_BIN[probeNum]; ++i)
		sv[i] = vol;

#include "pms1d_cv"

	concentration[0] = 0.0;

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

/* END F300.C */
