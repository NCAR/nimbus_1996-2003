/*******       ATTACK (Deg) ********************************          ATTACK 
  	Input:
  		akangl - derived attack angle
  	Output:
  		attack - derived generic attack angle (Deg)
*/

#include "nimbus.h"
#include "amlib.h"

/* -------------------------------------------------------------------- */
void sattack(varp)
DERTBL	*varp;
{
	PutSample(varp, GetSample(varp, 0));
}
