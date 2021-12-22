/**************************** XRLWCC *******************************  XRLWCC
 ** Corrected PMS liquid water content (g/m3) - RAF digital controlled probe
 	Input:
 		plwc - raw liquid water content
 		tasx - derived true air speed
 		atx - derived ambient temperature
 		psxc - derived static pressure
 	Output:
 		xrlwcc - corrected PMS liquid water content
*/

#include "nimbus.h"
#include "amlib.h"

static NR_TYPE twire;

NR_TYPE kinglwcc();

/* -------------------------------------------------------------------- */
void xrlwccInit(DERTBL *varp)
{
	twire = (GetDefaultsValue("TWIRE_RAF"))[0];

}	/* END XRLWCCINIT */

/* -------------------------------------------------------------------- */
void sxrlwcc(varp)
DERTBL	*varp;
{
	NR_TYPE plwc,tasx,atx,psxc;

	plwc	= GetSample(varp, 0);
	tasx	= GetSample(varp, 1);
	atx	= GetSample(varp, 2);
	psxc	= GetSample(varp, 3);

	PutSample(varp, kinglwcc(plwc, tasx, atx, psxc, twire));
}
