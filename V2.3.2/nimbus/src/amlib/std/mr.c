/*******       MIXING RATIO (G/KG)                                      MR
                  REQUIRES --- PSXC, EDPC(DPXC)
 	Input:
 		psxc - derived static pressure
 		dpxc - derived dew point
 	Output:
 		mr - mixing ratio
*/
 
#include "nimbus.h"
#include "amlib.h"

static NR_TYPE edpc;

double	esubt();

/* -------------------------------------------------------------------- */
void smr(varp)
DERTBL	*varp;
{
	NR_TYPE	mr, dpxc, psxc;

	psxc = GetSample(varp, 0);
	dpxc = GetSample(varp, 1);
	edpc = (NR_TYPE)esubt(dpxc);

	if (psxc != edpc)
		mr = 622.0 * edpc / (psxc - edpc);
	else
		mr = 0.0;

	PutSample(varp, mr);

}

/* -------------------------------------------------------------------- */
void sedpc(varp)
DERTBL	*varp;
{
	PutSample(varp, edpc);
}
