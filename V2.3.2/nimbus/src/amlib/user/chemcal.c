/*
-------------------------------------------------------------------------
OBJECT NAME:	chemcal.c

FULL NAME:	Chemistry Calculations.

ENTRY POINTS:	so3fsc()
		sxeco2c()
		sxnoym()
		sxnom()

STATIC FNS:	none

DESCRIPTION:	Calculations for Greg Kok.

INPUT:		

REFERENCES:	none

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1993,1994
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"

static float	o3fsm_cal, *xnoym_cal, *xnom_cal, xno2c_cal, *xo3f1t_cal;

/* -------------------------------------------------------------------- */
void kokInit()
{
	o3fsm_cal = (GetDefaultsValue("O3FSM_CAL"))[0];
	xnoym_cal = GetDefaultsValue("XNOYM_CAL");
	xnom_cal  = GetDefaultsValue("XNOM_CAL");
	xno2c_cal = (GetDefaultsValue("XNO2C_CAL"))[0];
	xo3f1t_cal= GetDefaultsValue("XO3F1T_CAL");

}	/* KOKINIT */

/* -------------------------------------------------------------------- */
void so3fc(varp)
DERTBL	*varp;
{
	NR_TYPE	o3_sig, edpc, psfdc, fudge;

	o3_sig	= GetSample(varp, 0);
	edpc	= GetSample(varp, 1);
	psfdc	= GetSample(varp, 2);
	fudge	= GetSample(varp, 3);

	PutSample(varp, o3fsm_cal * o3_sig * (1.0 + 3.43 * edpc / psfdc)*fudge);

}	/* END SXO3FC */

/* -------------------------------------------------------------------- */
void sxeco2c(varp)
DERTBL	*varp;
{
	NR_TYPE	xeco2c, xecco2, xecco2p, xecco2t, x, y;

	xecco2	= GetSample(varp, 0);
	xecco2p	= GetSample(varp, 1);
	xecco2t	= GetSample(varp, 2);

	x = xecco2 * 1013.0 / xecco2p * 1000.0;
	y = (xecco2t + 273.16) / 308.11;

	PutSample(varp, x * (0.1362 + 2.008e-5 * x + 2.029e-9 * x * x) * y);

}	/* END SXECO2C */

/* -------------------------------------------------------------------- */
void sxnoym(varp)
DERTBL	*varp;
{
	NR_TYPE	xnoy;

	xnoy = GetSample(varp, 0);

	PutSample(varp, xnoym_cal[0] + (xnoym_cal[1] * xnoy));

}	/* END SXNOYM */

/* -------------------------------------------------------------------- */
void sxnom(varp)
DERTBL	*varp;
{
	NR_TYPE	xno;

	xno = GetSample(varp, 0);

	PutSample(varp, xnom_cal[0] + (xnom_cal[1] * xno));

}	/* END SXNOM */

/* -------------------------------------------------------------------- */
void sxno2(varp)
DERTBL	*varp;
{
	NR_TYPE	lums;

	lums	= GetSample(varp, 0);

	PutSample(varp, 0.0035 + (5.881 - 0.2667 * lums) * lums);

}	/* END SXNO2 */

/* -------------------------------------------------------------------- */
void sxno2c(varp)
DERTBL	*varp;
{
	NR_TYPE	xno2, psfd, pcor;

	xno2	= GetSample(varp, 0);
	psfd	= GetSample(varp, 1);

	if (psfd >= 0.0)
		pcor = xno2c_cal / psfd;
	else
		pcor = 0.0;

	PutSample(varp, xno2 * pcor);

}	/* END SXNO2C */

/* -------------------------------------------------------------------- */
void sxo3fdg(varp)
DERTBL	*varp;
{
	PutSample(varp,
		1.0 - (GetSample(varp, 0) - xo3f1t_cal[0]) * xo3f1t_cal[1]);

}	/* END SXO3FDG */

/* END CHEMCAL.C */
