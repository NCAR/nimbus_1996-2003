/*******       Equivalent Potential Temperature (K)                     thetae
	Bolton (1980) parameterization

 	Input:
 		atx - derived ambient temperature
 		dpxc - derived dew point temperature
 		psxc - derived static pressure
 		mr - derived mixing ration
 	Output:
 		thetae - derived equivalent potential temperature
*/
 
#include "nimbus.h"
#include "amlib.h"

double esubt();

/* -------------------------------------------------------------------- */
void sthetae(varp)
DERTBL	*varp;
{
	NR_TYPE	dpxc, psxc, atx, mr;
	NR_TYPE	edpc, theta, tlcl, exparg;

	atx	= GetSample(varp, 0);
	dpxc	= GetSample(varp, 1);
	psxc	= GetSample(varp, 2);
	mr	= GetSample(varp, 3);

	if ((atx += 273.16) <= 0.0)
		atx = 0.0001;

	if ((edpc = (NR_TYPE)esubt(dpxc)) <= 0.0)
		edpc = 0.0001;

	tlcl = (2840.0 / (3.5 * log((double)atx) - log((double)edpc) - 4.805))
								+ 55.0;
	if (tlcl <= 0.0)
		tlcl = 0.0001;

	theta = atx * pow((double)1000.0 / psxc, (double)0.28571);

	exparg = ((3.376 / tlcl) - 0.00254) * (mr * (1.0 + (0.00081 * mr)));
	if (exparg > 741.0)
		exparg = 0.0001;

	PutSample(varp, theta * exp((double)exparg));

}
