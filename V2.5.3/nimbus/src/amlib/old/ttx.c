/*******       TOTAL TEMPERATURE (Default) (C)                      TTX 
  	Input:
  		tt - raw total temperature 
  	Output:
  		ttx - raw generic total temperature (C)
*/

#include "nimbus.h"
#include "amlib.h"

/* -------------------------------------------------------------------- */
void sttx(varp)
DERTBL	*varp;
{
	PutSample(varp, GetSample(varp, 0));

}
