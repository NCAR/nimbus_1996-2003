/*******AKDF  ATTACK ANGLE OF THE BOOM (deg)                         AKDF
 
 	Input:
 		adif - raw boom differential pressure
 		qcb - raw boom dynamic pressure
		xmach2 - derived mach number
 	Output:
 		akdf - attack angle of the boom (deg)
*/

#include "nimbus.h"
#include "amlib.h"

/* -------------------------------------------------------------------- */
void sakdf(varp)
DERTBL	*varp;
{
	NR_TYPE	qcb,adif;
	NR_TYPE	akdf;
	NR_TYPE	gr,divr;
	double	xmach2;

	qcb	= GetSample(varp, 0);
	adif	= GetSample(varp, 1);
	xmach2	= GetSample(varp, 2);

	if (qcb < 0.001)
		qcb = 0.001;

	gr   = 0.086577797 - 0.03560256 * sqrt(xmach2) + 0.00006143 * xmach2;
	divr = gr * qcb;

	if (divr == 0)
		divr = 0.00079;

	akdf = adif / divr;

/*	if (akdf > 20.0)
		akdf = 20.0;

	if (akdf < -20.0)
		akdf = -20.0;		*/

	PutSample(varp, akdf);

}	/* END SAKDF */

/* END AKDF.C */
