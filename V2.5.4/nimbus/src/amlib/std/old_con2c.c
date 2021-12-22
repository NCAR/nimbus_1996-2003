/*******CON2C  (N/LTR) *(from 2D-C probe)***************************** CON2C
 	Input:
 		sdwrc - raw "shadow or" counts
 		tasx - derived true air speed
 	Output:
 		con2c1 - derived concentration
*/

#include "nimbus.h"
#include "amlib.h"

static NR_TYPE	C1_EAW, C2_EAW;

/* -------------------------------------------------------------------- */
void conc2cInit(DERTBL *varp)
{
	C1_EAW = (GetDefaultsValue("C1_EAW"))[0];
	C2_EAW = (GetDefaultsValue("C2_EAW"))[0];

}	/* END CON2CINIT */

/* -------------------------------------------------------------------- */
void scon2c(varp)
DERTBL	*varp;
{
	NR_TYPE	sdwrc, tasx, EAW;

	sdwrc	= GetSample(varp, 0);
	tasx	= GetSample(varp, 1);

	if (varp->name[5] == '1')
		EAW = C1_EAW;
	else
		EAW = C2_EAW;

	PutSample(varp, 1000.0 * (sdwrc / (tasx * EAW * 61.0)));
}
