/*    AMBT = ambient temperature calculation function (C)                AMBT */

#include "nimbus.h"
#include "amlib.h"

NR_TYPE ambt(t,r,xm2)
NR_TYPE	t, r, xm2;
{
	return((t+273.16)/(1.0+0.2*r*xm2) - 273.16);
}
