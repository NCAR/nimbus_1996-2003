/*******       Static Pressure  (Default) (mb)                         PSX 
  	Input:
  		ps - raw static pressure
  	Output:
  		psx - raw generic static pressure (mb)
*/

#include "nimbus.h"
#include "amlib.h"

/* -------------------------------------------------------------------- */
void spsx(varp)
DERTBL	*varp;
{
	PutSample(varp, GetSample(varp, 0));

}
