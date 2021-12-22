/*******       AMBIENT TEMPERATURE (Default) (C)                      ATX 
  	Input:
  		ambt - derived ambient temperature to be used as default
  	Output:
  		atx - derived ambient temperature (C)
*/

#include "nimbus.h"
#include "amlib.h"

/* -------------------------------------------------------------------- */
void satx(varp)
DERTBL	*varp;
{
	PutSample(varp, GetSample(varp, 0));
}
