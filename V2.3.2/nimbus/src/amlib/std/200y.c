/*
-------------------------------------------------------------------------
OBJECT NAME:	200y.c

FULL NAME:	Compute derived paramters for PMS1D 200Y probe.

ENTRY POINTS:	sc200y()

STATIC FNS:	none

DESCRIPTION:	

OUTPUT:		

REFERENCES:	pms1d.c (ComputePMS1DParams(), ComputeDOF(),
			 ComputeConcentrations())

REFERENCED BY:	Compute()

NOTES:		Calculations taken from Bulletin 24 dated 1/89.

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"

#define MAX_200Y	2

static const int FIRST_BIN = 1;
static const int LAST_BIN = BINS_16;

static const int NDIODES = 24;
static const int RESOLUTION = 300;
static const NR_TYPE MAG = 0.667;
static const NR_TYPE RESPONSE_TIME = 0.35;
static const NR_TYPE ARM_DISTANCE = 263.0;	/* in mm	*/

static const NR_TYPE DENS = 1.0;
static const double PLWFAC = 0.000000001;

static NR_TYPE	total_concen[MAX_200Y], dbar[MAX_200Y], plwc[MAX_200Y],
		disp[MAX_200Y];

static float	radius[BINS_16],
		dia[BINS_16],
		esw[BINS_16];	/* Effective Sample Width	*/

void	ComputePMS1DParams(), ComputeDOF();


/* -------------------------------------------------------------------- */
void c200yInit()
{
	register int	i;

	ComputePMS1DParams(radius, esw, RESOLUTION, MAG, NDIODES,
					FIRST_BIN, LAST_BIN);

	for (i = FIRST_BIN; i < LAST_BIN; ++i)
		dia[i] = (float)i * RESOLUTION;

}	/* END CONSTRUCTOR */

/* -------------------------------------------------------------------- */
void sc200y(varp)
DERTBL	*varp;
{
	register	i;
	NR_TYPE		*actual;
	NR_TYPE		tasx;		/* True Air Speed		*/
	NR_TYPE		*concentration;
	float		dof[BINS_16],	/* Depth Of Field		*/
			sv[BINS_16];	/* Sample Volume		*/

	/* NOTE: -1 index offset to compensate for undersizeing.
	 */
	actual	= GetVector(varp, 0, 16) - 1;
	tasx	= GetSample(varp, 1);

	if (FeedBack == HIGH_RATE_FEEDBACK)
		{
		if (SampleOffset >= varp->OutputRate)
			return;

		tasx /= varp->OutputRate;
		concentration = &HighRateData[varp->HRstart +
						(SampleOffset * varp->Length)];
		}
	else
		concentration = &AveragedData[varp->LRstart];


	ComputeDOF(radius, tasx, dof, FIRST_BIN, LAST_BIN, RESPONSE_TIME);

	for (i = FIRST_BIN; i < LAST_BIN; ++i)
		{
		if (dof[i] > ARM_DISTANCE)
			dof[i] = ARM_DISTANCE;

		sv[i] = tasx * (dof[i] * esw[i]);
		}

#define PLWC

#include "pms1d_cv"

	concentration[0] = 0.0;

}	/* END SC200Y */

/* -------------------------------------------------------------------- */
void sconcy(varp)
DERTBL	*varp;
{
	PutSample(varp, total_concen[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdispy(varp)
DERTBL	*varp;
{
	PutSample(varp, disp[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void splwcy(varp)
DERTBL	*varp;
{
	PutSample(varp, plwc[varp->ProbeCount]);
}

/* -------------------------------------------------------------------- */
void sdbary(varp)
DERTBL	*varp;
{
	PutSample(varp, dbar[varp->ProbeCount]);
}

/* END 200Y.C */
