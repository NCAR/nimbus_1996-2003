/*******       AMBIENT TEMPERATURE (BOOM ROSEMOUNT) (C)                 ATB 
                   REQUIRES --- TTB, RECFB, XMACH2, AMBT FCN.
  	Input:
  		ttb - raw total temperature
  		xmach2 - derived mach number
  	Output:
  		atb - derived ambient temperature (C)
  	Include:
  		recfb - recovery factor 
  		ambtf - ambient temperature function
*/

#include "nimbus.h"
#include "amlib.h"

extern NR_TYPE ambt();
extern NR_TYPE recfb;

/* -------------------------------------------------------------------- */
void satb(varp)
DERTBL	*varp;
{
	NR_TYPE	ttb, xmach2, atb;

	ttb	= GetSample(varp, 0);
	xmach2	= GetSample(varp, 1);
	
	if (ttb < -273.15)
		ttb  = -273.15;

	PutSample(varp, ambt(ttb, recfb, xmach2));

}	/* END SATB */

/* END SATB.C */
