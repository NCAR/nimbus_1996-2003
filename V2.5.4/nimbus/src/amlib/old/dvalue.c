/*******   D-VALUE                                                      DVALUE
 **    Surface Altitude (m)
 	Input:
 		hgme - raw radar altimeter
 		palt - derived pressure altitude
 	Output:
 		dvalue - derived surface altitude
*/

#include "nimbus.h"
#include "amlib.h"

/* -------------------------------------------------------------------- */
void sdvalue(varp)
DERTBL	*varp;
{
	NR_TYPE	palt, hgme;

	hgme = GetSample(varp, 0);
	palt = GetSample(varp, 1);

	PutSample(varp, hgme - palt);
}
