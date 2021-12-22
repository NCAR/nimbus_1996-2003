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

#define MAX_F300	2

static const int FIRST_BIN = 1;
static const int LAST_BIN = BINS_32;

static NR_TYPE	total_concen[MAX_F300], dbar[MAX_F300], disp[MAX_F300];

static NR_TYPE	*dia;

/* -------------------------------------------------------------------- */
void cf300Init()
{
	dia = GetDefaultsValue("F300_CSIZ");

}	/* END CF300INIT */

/* -------------------------------------------------------------------- */
void scf300(varp)
DERTBL	*varp;
{
	register	i;
	NR_TYPE		*actual;
	NR_TYPE		*concentration;
	NR_TYPE		tas;		/* True Air Speed	*/
	float		vol, sv[32];	/* Sample Volume	*/

	actual	= GetVector(varp, 0, 32);
	tas	= GetSample(varp, 1);

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

	for (i = FIRST_BIN; i < LAST_BIN; ++i)
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
