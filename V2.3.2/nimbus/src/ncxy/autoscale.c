/*
-------------------------------------------------------------------------
OBJECT NAME:	autoscale.c

FULL NAME:		Auto Scale Axies

DESCRIPTION:	These procedures are responsible for auto scaling the
				X and Y axis.

INPUT:			Byte containing bits set for which axies to scale

OUTPUT:			Globals ymin, ymax affected.

AUTHOR:			websterc@ncar
-------------------------------------------------------------------------
*/

#include "define.h"
#include <math.h>

static void	scaleaxis();
static NR_TYPE	RoundBig(), expt();

/* -------------------------------------------------------------------- */
void AutoScale(axis)
char	axis;
{
	if (axis & SCALE_XAXIS && vpX)
		{
		xmin = VarStats[X].min;
		xmax = VarStats[X].max;
		scaleaxis(&xmin, &xmax);
		SetXdialog();
		}

	if (axis & SCALE_YAXIS && vpY)
		{
		ymin = VarStats[Y].min;
		ymax = VarStats[Y].max;
		scaleaxis(&ymin, &ymax);
		SetYdialog();
		}

}	/* END AUTOSCALE */

/* -------------------------------------------------------------------- */
static void scaleaxis(min, max)
NR_TYPE	*min, *max;
{
	NR_TYPE	tmpmin, tmpmax, range, d;

	/* min better not equal max */
	if (*min == *max)
		*max += 1.0;

	range = RoundBig(*max - *min, FALSE);
	d = RoundBig(range / numtics, TRUE);	/* tic mark spacing	*/

	tmpmin = floor(*min / d) * d;
	tmpmax = ceil(*max / d) * d;

	*min = tmpmin;
	*max = tmpmax;

}	/* END SCALEAXIS */

/* -------------------------------------------------------------------- */
static NR_TYPE RoundBig(x, round)
NR_TYPE	x;
int	round;
{
	int	exp;
	NR_TYPE	f, y;

	exp = floor(log10(x));
	f = x / expt(10.0, exp);     /* fraction between 1 and 10 */

	if (round)
		if (f < 1.5)
			y = 1.0;
		else
			if (f < 3.0)
				y = 2.0;
			else
				if (f < 7.0)
					y = 5.0;
				else
					y = 10.0;
	else
		if (f <= 1.0)
			y = 1.0;
		else
			if (f <= 2.0)
				y = 2.0;
			else
				if (f <= 5.0)
					y = 5.0;
				else
					y = 10.0;

	return(y * expt(10.0, exp));

}	/* END ROUNDBIG */

/* -------------------------------------------------------------------- */
static NR_TYPE expt(a, n)
NR_TYPE		a;
register int	n;
{
	NR_TYPE	x;

	x = 1.0;

	if (n > 0)
		for (; n > 0; n--)
			x *= a;
	else
		for (; n < 0; n++)
			x /= a;

	return(x);

}	/* END EXPT */

/* END AUTOSCALE.C */
