/*******       Dew Point Temperature (Default) (C)                      DPXC 
  	Input:
  		dpc - derived dewpoint temperature
  	Output:
  		dpxc - derived generic dewpoint temperature (C)
*/

#include "nimbus.h"
#include "amlib.h"

/* -------------------------------------------------------------------- */
void sdpxc(varp)
DERTBL	*varp;
{
	PutSample(varp, GetSample(varp, 0));
}
