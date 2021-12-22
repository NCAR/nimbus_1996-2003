/*
-------------------------------------------------------------------------
OBJECT NAME:	fssp.c

FULL NAME:	Derived computations for the PMS1D FSSP-100 probe

ENTRY POINTS:	scfssp()
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

#define MAX_FSSP	4

static const int FIRST_BIN = 1;
static const int LAST_BIN = BINS_16;

static const double PLWFAC = 0.000001;
static const double DBZFAC = 1.0e6;

static float	sa, dof, bdia, tau1, tau2, *fssp_csiz, DENS;

static NR_TYPE	total_concen[MAX_FSSP], dbar[MAX_FSSP], plwc[MAX_FSSP],
		disp[MAX_FSSP], dbz[MAX_FSSP];

/* -------------------------------------------------------------------- */
void cfsspInit()
{
	dof = (GetDefaultsValue("DOF"))[0];
	bdia = (GetDefaultsValue("BDIA"))[0];
	tau1 = (GetDefaultsValue("TAU1"))[0];
	tau2 = (GetDefaultsValue("TAU2"))[0];
	DENS = (GetDefaultsValue("DENS"))[0];

	sa = dof * bdia;
	fssp_csiz = GetDefaultsValue("FSSP_CSIZ");

}	/* END CFSSPINIT */

/* -------------------------------------------------------------------- */
void scfssp(varp)
DERTBL	*varp;
{
	register	i;
	NR_TYPE		*actual;
	NR_TYPE		tas;		/* True Air Speed	*/
	NR_TYPE		act;		/* Activity		*/
	NR_TYPE		fbmfr;		/* Beam Fraction	*/
	NR_TYPE		frange;
	float		vol, sv[16];	/* Sample Volume	*/
	float		*dia;
	NR_TYPE		*concentration;

	actual	= GetVector(varp, 0, 16);
	tas	= GetSample(varp, 1);
	act	= GetSample(varp, 2);
	fbmfr	= GetSample(varp, 3);
	frange	= GetSample(varp, 4);

	if (FeedBack == HIGH_RATE_FEEDBACK)
		{
		if (SampleOffset >= varp->OutputRate)
			return;

		tas /= varp->OutputRate;
		act *= varp->OutputRate;
		concentration = &HighRateData[varp->HRstart +
						(SampleOffset * varp->Length)];
		}
	else
		concentration = &AveragedData[varp->LRstart];

	vol = tas * sa * fbmfr * (1.0 - (act * 0.71));

	for (i = FIRST_BIN; i < LAST_BIN; ++i)
		sv[i] = vol;

	dia = &fssp_csiz[(int)frange << 4];

#define PLWC
#define DBZ

#include "pms1d_cv"

	concentration[0] = 0.0;

}	/* END SCFSSP */

/* -------------------------------------------------------------------- */
void sfact(varp)
DERTBL	*varp;
{
	NR_TYPE	fstrob, freset;

	/* This function is for the old interface card only.  Activity is
	 * a housekeeping param on the new interface card.
	 */
	if (FeedBack == HIGH_RATE_FEEDBACK && SampleOffset >= varp->OutputRate)
		return;
	
	freset	= GetSample(varp, 0);
	fstrob	= GetSample(varp, 1);

	PutSample(varp, (fstrob*tau1 + freset*tau2));

}	/* END SFACT */

/* -------------------------------------------------------------------- */
void sfbmfr(varp)
DERTBL	*varp;
{
	register	i;
	NR_TYPE		*afssp;
	NR_TYPE		fstrob, fbmfr = 0.0, total_actual = 0.0;

	if (FeedBack == HIGH_RATE_FEEDBACK && SampleOffset >= varp->OutputRate)
		return;
	
	afssp	= GetVector(varp, 0, 16);
	fstrob	= GetSample(varp, 1);

	for (i = FIRST_BIN; i < LAST_BIN; ++i)
		total_actual += afssp[i];

	if (fstrob != 0.0)
		fbmfr = total_actual / fstrob;

	if (fstrob < 1.0 || total_actual <= 0.0)
		fbmfr = 0.4;

	PutSample(varp, fbmfr);

}	/* END SFBMFR */

/* -------------------------------------------------------------------- */
void sconcf(varp)
DERTBL	*varp;
{
	PutSample(varp, total_concen[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdispf(varp)
DERTBL	*varp;
{
	PutSample(varp, disp[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void splwcf(varp)
DERTBL	*varp;
{
	PutSample(varp, plwc[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbzf(varp)
DERTBL	*varp;
{
	PutSample(varp, dbz[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbarf(varp)
DERTBL	*varp;
{
	PutSample(varp, dbar[varp->ProbeCount]);
}

/* END FSSP.C */
