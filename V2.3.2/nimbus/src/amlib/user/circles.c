/*
-------------------------------------------------------------------------
OBJECT NAME:	circles.c

FULL NAME:	Flying circles

ENTRY POINTS:	sptdrdst(), sptdrtim(), sptdrrad()

STATIC FNS:	none

DESCRIPTION:	How to fly a large circle.

INPUT:		

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1995
-------------------------------------------------------------------------
*/

#include <math.h>
#include "nimbus.h"
#include "amlib.h"

#define MAX_CIRCLE	3

static int	reset = TRUE;

static NR_TYPE	heading,	/* Current heading shoud be		*/
		neg_heading,	/* Opposite direction			*/
		distance,	/* Distance of circumference covered	*/
		Circles[MAX_CIRCLE], circleSize;


/* -------------------------------------------------------------------- */
void initCircles()
{
	int	i;
	NR_TYPE	*mins;

	mins = GetDefaultsValue("CIRCLES");

	for (i = 0; i < MAX_CIRCLE; ++i)
		Circles[i] = 360.0 / mins[i] * 60.0;

}	/* END INITCIRCLES */

/* -------------------------------------------------------------------- */
void ResetCircle(num)
int	num;
{
	reset = TRUE;
	circleSize = Circles[num];

}	/* END RESETCIRCLE */

/* -------------------------------------------------------------------- */
void scircP(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	thdg;

	thdg = GetSample(varp, 0);

	if (reset)	/* It's been reset	*/
		{
		heading = neg_heading = thdg;
		distance = 0.0;
		reset = FALSE;
		}
	else
		{
		heading = heading + circleSize;
		neg_heading = neg_heading - circleSize;
		}


	if (neg_heading < 0.0)
		neg_heading += 360.0;
	else
	if (neg_heading >= 360.0)
		neg_heading -= 360.0;
		
	if (heading < 0.0)
		heading += 360.0;
	else
	if (heading >= 360.0)
		heading -= 360.0;
		
	PutSample(varp, heading);

}	/* END SPTDRDST */

/* -------------------------------------------------------------------- */
void scircN(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	PutSample(varp, neg_heading);

}	/* END SPTDRRAD */

/* -------------------------------------------------------------------- */
void scircR(varp)
DERTBL	*varp;		/* Information about current variable	*/
{
	NR_TYPE	radius, tasx, thdg;
	static NR_TYPE	prevTHDG;

	tasx = GetSample(varp, 0);
	thdg = GetSample(varp, 1);

	distance += tasx;

	radius = distance / (thdg - prevTHDG);

	PutSample(varp, radius);

	prevTHDG = thdg;

}	/* END SPTDRRAD */

/* END PRCLTRK1.C */
