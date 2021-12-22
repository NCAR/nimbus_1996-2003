/*
-------------------------------------------------------------------------
OBJECT NAME:	licor.c

FULL NAME:	LI-COR 6262

ENTRY POINTS:	sxco2c()
		sxlh2oc()
		sxmrli()

STATIC FNS:	none

DESCRIPTION:	Calculations for Chem folks

INPUT:		

REFERENCES:	none

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1994
-------------------------------------------------------------------------
*/

#include <math.h>

#include "nimbus.h"
#include "amlib.h"

static NR_TYPE	C2T, *CO2_CAL, H2T, *H2O_CAL, CAL_FACTOR, OFFSET;

/* -------------------------------------------------------------------- */
void LICORinit(DERTBL *varp)
{
	C2T = (GetDefaultsValue("C2T"))[0];
	CO2_CAL = GetDefaultsValue("CO2_CAL");
	CAL_FACTOR = (GetDefaultsValue("CAL_FACTOR"))[0];
	OFFSET = (GetDefaultsValue("CO2_OFFSET"))[0];

	H2T = (GetDefaultsValue("H2T"))[0];
	H2O_CAL = GetDefaultsValue("H2O_CAL");

}	/* LICORINIT */

/* -------------------------------------------------------------------- */
void sxco2c(DERTBL *varp)
{
	NR_TYPE	co2c, co2p, co2s, co2t, h2oc, x, y, w;

	co2p	= GetSample(varp, 0);
	co2s	= GetSample(varp, 1);
	co2t	= GetSample(varp, 2);
	h2oc	= GetSample(varp, 3);

	w = h2oc / 1000.0 * 0.5 + 1.0;
	x = co2s / w;
	y = (co2t + 273.15) / C2T;

	co2c = x*(CO2_CAL[0] + x*(CO2_CAL[1] + x*(CO2_CAL[2] + x*(CO2_CAL[3] + x*CO2_CAL[4]))));

	co2c = w * CAL_FACTOR * co2c * y * pow(1013.0 / co2p, 1.261) - OFFSET;

	PutSample(varp, co2c);

}	/* END SXCO2C */

/* -------------------------------------------------------------------- */
void sxlh2oc(DERTBL *varp)
{
	NR_TYPE	h2oc, co2p, co2t, h2os, x, y, x2;

	co2p	= GetSample(varp, 0);
	co2t	= GetSample(varp, 1);
	h2os	= GetSample(varp, 2);

	x = h2os * 1.013e6 / co2p;
	y = (co2t + 273.15) / H2T;
	x2 = x * x;

	PutSample(varp, ((H2O_CAL[0] * x) + (H2O_CAL[1] * x2) + (H2O_CAL[2] * x2 * x)) * y);

}	/* END SXH2OC */

/* -------------------------------------------------------------------- */
void sxmrli(DERTBL *varp)
{
	NR_TYPE	h2oc, m;

	h2oc = GetSample(varp, 0);

	m = (28.84 * (1.0 - h2oc / 1000.0)) + (18.01 * h2oc / 1000.0);

	PutSample(varp, 18.01 * h2oc / m);

}	/* END SXMRLI */

/* END LICOR.C */
