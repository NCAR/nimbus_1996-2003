/*
-------------------------------------------------------------------------
OBJECT NAME:	gust.c

FULL NAME:	Gust calculations

ENTRY POINTS:	swi(), sui(), svi(), sux(), svy()

STATIC FNS:	

DESCRIPTION:	Template for AMLIB std & user directories.

INPUT:		

OUTPUT:		WI, UI, VI, UX, VY

REFERENCES:	

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1993
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"
#include "gust.h"

#define MAX_PROBES	4

static const NR_TYPE	THDG_TEST	= 180.0 * M_PI / 180.0;
static const NR_TYPE	PITCH_TEST	= 22.5 * M_PI / 180.0;

static NR_TYPE	ui, vi, ux, vy;

NR_TYPE GetBoomLength();


/* -------------------------------------------------------------------- */
void swi(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	wi, attack, sslip, tas, thdg, vew, vns, pitch, roll, wp3;

	NR_TYPE	tas_dab, delph, thedot, delth,
		a, b, c, d, e, f, g, h, ab, p, r, s, t, psidot, bvns, bvew,
		cs, ss, ch, sh, cr, sr, ca, sa, cb, sb, ta, tb;

	static bool	firstTime[nFeedBackTypes] = { TRUE, TRUE };
	static float	boomln;
	static NR_TYPE	pitch0[MAX_PROBES][nFeedBackTypes],
			thdg0[MAX_PROBES][nFeedBackTypes],
			DELT[nFeedBackTypes];

	tas	= GetSample(varp, 0);
	vew	= GetSample(varp, 1);
	vns	= GetSample(varp, 2);
	pitch	= GetSample(varp, 3) * DEG_RAD;
	roll	= GetSample(varp, 4) * DEG_RAD;
	thdg	= GetSample(varp, 5) * DEG_RAD;
	attack	= GetSample(varp, 6) * DEG_RAD;
	sslip	= GetSample(varp, 7) * DEG_RAD;
	wp3	= GetSample(varp, 8);


	if (firstTime[FeedBack])
		{
		if (FeedBack == LOW_RATE_FEEDBACK)
			DELT[FeedBack]	= 1.0;
		else
			DELT[FeedBack]	= 1.0 / ProcessingRate;

		pitch0[varp->ProbeCount][FeedBack]	= pitch;
		thdg0[varp->ProbeCount][FeedBack]	= thdg;

		boomln = GetBoomLength();

		firstTime[FeedBack] = FALSE;
		}

	/* Coordinate transformation
	 */
	cs	= (NR_TYPE)cos((double)thdg);
	ss	= (NR_TYPE)sin((double)thdg);
	ch	= (NR_TYPE)cos((double)pitch);
	sh	= (NR_TYPE)sin((double)pitch);
	cr	= (NR_TYPE)cos((double)roll);
	sr	= (NR_TYPE)sin((double)roll);
/*	ca	= (NR_TYPE)cos((double)attack);
	sa	= (NR_TYPE)sin((double)attack);
	cb	= (NR_TYPE)cos((double)sslip);
	sb	= (NR_TYPE)sin((double)sslip);
*/	ta	= (NR_TYPE)tan((double)attack);
	tb	= (NR_TYPE)tan((double)sslip);

	tas_dab	= -tas * 1.0 / (NR_TYPE)sqrt((double)1.0 + ta*ta + tb*tb);
/*	a	= cb * sh;
	b	= -sb * ch * sr;
	c	= -ch * cr;
	d	= cb * cs * ch;
	g	= cb * ss * ch;
*/	e	= -tb * (ss * cr - cs * sh * sr);
	f	= cs * sh * cr + ss * sr;
	h	= tb * (cs * cr + ss * sh * sr);
	p	= ss * sh * cr - cs * sr;
	ab	= sh - tb * ch * sr - ta * ch * cr;


	/* Nose boom angular velocity correction terms
	 */
	delph	= pitch - pitch0[varp->ProbeCount][FeedBack];

	if ((NR_TYPE)fabs((double)delph) >= PITCH_TEST)
		{
		if (delph > 0)
			pitch0[varp->ProbeCount][FeedBack] += PITCH_TEST * 2.0;

		if (delph < 0)
			pitch0[varp->ProbeCount][FeedBack] -= PITCH_TEST * 2.0;
		}

	thedot	= (pitch - pitch0[varp->ProbeCount][FeedBack]) / DELT[FeedBack];
	delth	= thdg - thdg0[varp->ProbeCount][FeedBack];

	if ((NR_TYPE)fabs((double)delth) >= THDG_TEST)
		{
		if (delth > 0)
			thdg0[varp->ProbeCount][FeedBack] += THDG_TEST * 2.0;

		if (delth < 0)
			thdg0[varp->ProbeCount][FeedBack] -= THDG_TEST * 2.0;
		}

	psidot	= (thdg - thdg0[varp->ProbeCount][FeedBack]) / DELT[FeedBack];
	bvns	= boomln * (psidot * ss * ch + thedot * cs * sh);
	bvew	= boomln * (thedot * sh * ss - psidot * cs * ch);

	pitch0[varp->ProbeCount][FeedBack]	= pitch;
	thdg0[varp->ProbeCount][FeedBack]	= thdg;


	/* Compute wind vector components WRT geographic coordinates
	 */
	r  = ss * ch + h + ta * p;
	s  = cs * ch + e + ta * f;
	t  = wp3 + boomln * thedot * ch;

	ui = tas_dab * r + (vew - bvew);
	vi = tas_dab * s + (vns - bvns);
	ux =  ui * ss + vi * cs;
	vy = -ui * cs + vi * ss;

	wi = tas_dab * ab + t;

	PutSample(varp, wi);

}	/* END SWI */

/* -------------------------------------------------------------------- */
void sui(varp)
DERTBL	*varp;
{
	PutSample(varp, ui);
}

/* -------------------------------------------------------------------- */
void svi(varp)
DERTBL	*varp;
{
	PutSample(varp, vi);
}

/* -------------------------------------------------------------------- */
void sux(varp)
DERTBL	*varp;
{
	PutSample(varp, ux);
}

/* -------------------------------------------------------------------- */
void svy(varp)
DERTBL	*varp;
{
	PutSample(varp, vy);
}

/* END GUST.C */
