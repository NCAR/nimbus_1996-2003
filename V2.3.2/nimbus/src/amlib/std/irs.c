/*
-------------------------------------------------------------------------
OBJECT NAME:	irs.c

FULL NAME:	Compute derived winds variables for Honeywell Intertial

ENTRY POINTS:	swp3(), shi3()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		Variables from the decoded Honeywell IRS block.

OUTPUT:		WP3, HI3

REFERENCES:	none

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"
#include "gust.h"
#include "raf.h"

#define GRAVITY	9.8063	/* Gravity in Michigan	*/

/* 200 second time constants.
*/
#define C1	0.09425
#define C2	0.002961
#define C3	0.00003101
 
/* Honeywell Manual Time constants.
#define C1	0.15
#define C2	0.0075
#define C3	0.000125
*/
  
static NR_TYPE	hi3[nFeedBackTypes];

extern int	Aircraft;


/* -------------------------------------------------------------------- */
void swp3(varp)
DERTBL	*varp;		/* WS variable pointer		*/
{
	NR_TYPE	lat, vew, vns, palt, acins, WP3;
	NR_TYPE	gfact, vcorac, acz;

	static bool	firstTime[nFeedBackTypes] = { TRUE, TRUE };
	static NR_TYPE	hx[nFeedBackTypes], hxx[nFeedBackTypes],
			wp3[nFeedBackTypes], deltaT[nFeedBackTypes],
			vzc[nFeedBackTypes];

	lat	= GetSample(varp, 0) * DEG_RAD;
	vew	= GetSample(varp, 1);
	vns	= GetSample(varp, 2);
	palt	= GetSample(varp, 3);
	acins	= GetSample(varp, 4);


	if (firstTime[FeedBack])
		{
		if (FeedBack == LOW_RATE_FEEDBACK)
			deltaT[FeedBack] = 1.0;
		else
			deltaT[FeedBack] = 1.0 / ProcessingRate;

		hx[FeedBack]	= hxx[FeedBack] = wp3[FeedBack] = 0.0;
		vzc[FeedBack]	= 0;
		hi3[FeedBack]	= palt;

		firstTime[FeedBack] = FALSE;
		}


	WP3 = wp3[FeedBack];

	/* Computation of vertical component of coriolis acceleration
	 */
	vcorac = -2.0 * OMEGAE * vew * (NR_TYPE)cos((double)lat) +
			(vew * vew + vns * vns) / EARTH_RADIUS;

	/* Computation of local gravity estimate, ACZ
	 */
	gfact = 9.780356 * (1.0 - 0.31391116e-6 * palt) *
		(1.0 + 0.0052885 * pow(sin((double)lat), (double)2.0));

	acz		= acins + (GRAVITY - gfact) + vcorac;
	vzc[FeedBack]	= vzc[FeedBack] + acz * deltaT[FeedBack];
	wp3[FeedBack]	= wp3[FeedBack] +
			(acz - C2 * hx[FeedBack] - C3 * hxx[FeedBack]) *
			deltaT[FeedBack];

	WP3 += wp3[FeedBack];

	/* 3rd order vertical velocity damping using PALT as reference
	 */
	hi3[FeedBack]	= hi3[FeedBack] + (wp3[FeedBack] - C1 * hx[FeedBack])
			* deltaT[FeedBack];
	hx[FeedBack]	= hi3[FeedBack] - palt;
	hxx[FeedBack]	= hxx[FeedBack] + hx[FeedBack] * deltaT[FeedBack];

	/* We use an average of the previous computation and this one to
	 * center the value in the time period.
	 */
	PutSample(varp, WP3 / 2.0);

}	/* END SWP3 */

/* -------------------------------------------------------------------- */
void shi3(varp)
DERTBL	*varp;
{
	PutSample(varp, hi3[FeedBack]);
}

/* END IRS.C */
