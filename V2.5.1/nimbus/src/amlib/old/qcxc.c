/*******       Dynamic Pressure  (Default) (mb)                         QCXC 
  	Input:
  		qcc - derived dynamic pressure
  	Output:
  		qcxc - derived generic dynamic pressure (mb)
*/

#include "nimbus.h"
#include "amlib.h"

/* -------------------------------------------------------------------- */
void sqcxc(varp)
DERTBL	*varp;
{
	PutSample(varp, GetSample(varp, 0));

}
