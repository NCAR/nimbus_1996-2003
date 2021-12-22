/*********************** RHODT *****************************         RHODT
 ** Absolute humidity (thermoelec) (g/m3)
    All moisture variables are computed with respect to a plane water
    surface.
 
 	Input:
 		atx - derived ambient temperature
 		dpxc - derived dew point temperature
 	Output:
 		rhodt - derived absolute humidity
*/

#include "nimbus.h"
#include "amlib.h"

double esubt();

/* -------------------------------------------------------------------- */
void srhodt(varp)
DERTBL	*varp;
{
	NR_TYPE	atx, dpxc;
	NR_TYPE	edpc;

	atx	= GetSample(varp, 0);
	dpxc	= GetSample(varp, 1);
	edpc	= (NR_TYPE)esubt(dpxc);

	PutSample(varp, 216.68 * edpc / (atx + 273.16));
}
