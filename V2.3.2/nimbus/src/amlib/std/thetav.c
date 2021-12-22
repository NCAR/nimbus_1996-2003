/*******       VIRTUAL POTENTIAL TEMPERATURE (K)                        THETAV
 	Input:
 		tvir - derived virtual temperature
 		psxc - derived static pressure
 	Output:
 		thetav - derived virtual potential temperature
*/
 
#include "nimbus.h"
#include "amlib.h"

double esubt();

/* -------------------------------------------------------------------- */
void sthetav(varp)
DERTBL	*varp;
{
	NR_TYPE	thetav = 0.0, atx, tvir, psxc, dpxc, edpc, sphum;

	psxc	= GetSample(varp, 0);
	atx	= GetSample(varp, 1);
	dpxc	= GetSample(varp, 2);

	edpc	= (NR_TYPE)esubt(dpxc);
	sphum	= psxc - 0.378*edpc;

	if (sphum != 0.0)
		sphum = 622.0 * edpc / sphum;

	tvir = 1.0 - 0.6e-3 * sphum;

	if (tvir != 0.0)
		tvir = (atx + 273.16) / tvir - 273.16;

	if (psxc != 0.0)
		thetav = (tvir + 273.16) * pow( (double)(1000.0 / psxc),
						(double)0.28571);

	PutSample(varp, thetav);

}
