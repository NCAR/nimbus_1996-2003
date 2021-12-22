/*******CON2P  (N/LTR) *(from 2D-P probe)***************************** CON2P
 	Input:
 		sdwrp - raw "shadow or" counts
 		tasx - derived true air speed
 	Output:
 		con2p1 - derived concentration
*/

#include "nimbus.h"
#include "amlib.h"

static NR_TYPE	P1_EAW, P2_EAW;

/* -------------------------------------------------------------------- */
void conc2pInit()
{
	P1_EAW = (GetDefaultsValue("P1_EAW"))[0];
	P2_EAW = (GetDefaultsValue("P2_EAW"))[0];

}	/* END CON2PINIT */

/* -------------------------------------------------------------------- */
void scon2p(varp)
DERTBL	*varp;
{
	NR_TYPE	sdwrp, tasx, EAW;

	sdwrp	= GetSample(varp, 0);
	tasx	= GetSample(varp, 1);
	
	if (varp->name[5] == '1')
		EAW = P1_EAW;
	else
		EAW = P2_EAW;

	PutSample(varp, 1000.0 * (sdwrp / (EAW * 261.0 * tasx)));
}
