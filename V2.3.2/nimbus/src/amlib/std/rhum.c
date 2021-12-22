/*******       RELATIVE HUMIDITY (PER CENT)                             RHUM
                  REQUIRES --- EDPC, ATX, ESUBT FUNCTION
 	Input:
 		atx - derived ambient temperature
 		edpc - derived vapor pressure
 	Output:
 		rhum - derived relative humidity
*/

#include "nimbus.h"
#include "amlib.h"

double esubt();

/* -------------------------------------------------------------------- */
void srhum(varp)
DERTBL	*varp;
{
	NR_TYPE	atx,edpc,rhum;

	atx	= GetSample(varp, 0);
	edpc	= GetSample(varp, 1);
	rhum	= (NR_TYPE)esubt(atx);

	if (rhum != 0.0)
		rhum = 100.0 * edpc / rhum;

	PutSample(varp, rhum);

}	/* END SRHUM */

/* END RHUM.C */
