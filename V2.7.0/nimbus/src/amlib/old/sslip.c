/*******       SIDESLIP (Deg) ********************************        SIDESLIP
  	Input:
  		ssangl - derived sideslip angle
  	Output:
  		sslip - derived generic sideslip angle (Deg)
*/

#include "nimbus.h"
#include "amlib.h"

/* -------------------------------------------------------------------- */
void ssslip(varp)
DERTBL	*varp;
{
	PutSample(varp, GetSample(varp, 0));

}
