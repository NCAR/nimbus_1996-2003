/*******       Static Pressure  (Default) (mb)                         PSXC 
  	Input:
  		psc - derived static pressure
  	Output:
  		psxc - derived generic static pressure (mb)
*/

#include "nimbus.h"
#include "amlib.h"

/* -------------------------------------------------------------------- */
void spsxc(varp)
DERTBL	*varp;
{
	PutSample(varp, GetSample(varp, 0));

}
