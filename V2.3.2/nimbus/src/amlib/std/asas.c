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

#define MAX_ASAS	2

static const int FIRST_BIN = 1;
static const int LAST_BIN = BINS_16;

static NR_TYPE	total_concen[MAX_ASAS], disp[MAX_ASAS], dbar[MAX_ASAS];
static NR_TYPE	aact[MAX_ASAS];
static float	*dia;

/* -------------------------------------------------------------------- */
void casasInit()
{
	dia = GetDefaultsValue("ASAS_CSIZ");

}	/* END CASASINIT */

/* -------------------------------------------------------------------- */
void scasas(varp)
DERTBL	*varp;
{
	register	i;
	NR_TYPE		*actual;
	NR_TYPE		*concentration;
	NR_TYPE		pact;		/* Activity		*/
	NR_TYPE		flow;		/* PCAS Flow Rate	*/
	float		sv[BINS_16];	/* Sample Volume	*/

	if (FeedBack == HIGH_RATE_FEEDBACK)
		{
		if (SampleOffset >= varp->OutputRate)
			return;

		concentration = &HighRateData[varp->HRstart +
						(SampleOffset * varp->Length)];
		}
	else
		concentration = &AveragedData[varp->LRstart];


	actual	= GetVector(varp, 0, 16);
	pact	= GetSample(varp, 1);
	flow	= GetSample(varp, 2);

	for (i = FIRST_BIN; i < LAST_BIN; ++i)
		sv[i] = (1.0 - pact) * flow;

#include "pms1d_cv"

	concentration[0] = 0.0;

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

/* END ASAS.C */
