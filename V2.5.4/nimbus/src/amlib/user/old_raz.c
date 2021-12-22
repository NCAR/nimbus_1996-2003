/*
-------------------------------------------------------------------------
OBJECT NAME:	raz.c

FULL NAME:	

ENTRY POINTS:	srange(), sazimuth()

STATIC FNS:	none

DESCRIPTION:	Functions for Radius and Distance from a fixed object.
		Used initially for SCMS project, fixed point was CP-2
		radar 7/95.

INPUT:		lat, lon

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1995
-------------------------------------------------------------------------
*/

#include <math.h>

#include "nimbus.h"
#include "amlib.h"

#define RAD	0.01745329

static NR_TYPE	startLat, startLon, r;

/* -------------------------------------------------------------------- */
void razInit(DERTBL *varp)
{
	startLat = (GetDefaultsValue("RAZ_START_LAT"))[0];
	startLon = (GetDefaultsValue("RAZ_START_LON"))[0];

}	/* END INITRAZ */

/* -------------------------------------------------------------------- */
void srange(DERTBL *varp)
{
	NR_TYPE	lat, lon, x, y, d;

	lat = GetSample(varp, 0);
	lon = GetSample(varp, 1);

	x = (lon - startLon) * cos((double)(RAD*lat)) * 60.0;

	y = (lat - startLat) * 60.0;

	d = sqrt(x*x + y*y);
	r = atan(x / y) * 57.29577952;

	if (x < 0.0 && y >= 0.0)
		r += 360.0;
	else
	if (x < 0.0 && y < 0.0)
		r += 180.0;
	else
	if (x >= 0.0 && y < 0.0)
		r += 180.0;

	PutSample(varp, d);

}	/* END SRANGE */

/* -------------------------------------------------------------------- */
void sazimuth(DERTBL *varp)
{
	PutSample(varp, r);

}	/* EMD SAZIMUTH */

/* END RAZ.C */
