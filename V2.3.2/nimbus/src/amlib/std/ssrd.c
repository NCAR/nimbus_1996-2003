/*******SSRD  SIDESLIP ANGLE OF THE RADOME (deg)                       SSRD
 
        Input:
                bdifr - raw radome differential pressure
                qcxc - corrected dynamic pressure for radome calcs
                xmach2 - derived mach number (Sabreliner only)
        Output:
                ssrd - attack angle of the radome (deg)
*/

#include "raf.h"
#include "nimbus.h"
#include "amlib.h"

extern int	Aircraft;

/* -------------------------------------------------------------------- */
void sssrd(varp)
DERTBL	*varp;
{
	NR_TYPE	qcxc, bdifr;
	NR_TYPE	ssrd, xmach;
	double	xmach2;

	bdifr	= GetSample(varp, 0);
	qcxc	= GetSample(varp, 1);
	xmach2	= GetSample(varp, 2);

	switch (Aircraft)
		{
		case ELECTRA:
			ssrd = ((bdifr / qcxc) + 0.0375) / 0.06577;
			break;

		case C130:
			ssrd = ((bdifr / qcxc) - 0.070) / 0.079;
			break;

		case KINGAIR:
			ssrd = ((bdifr / qcxc) - 0.002825) / 0.07448;
			break;

		case SABRELINER:
		case B57:
			xmach = sqrt(xmach2);
			ssrd = ((bdifr / qcxc) + 0.0241628 * xmach - 0.0754196)
			/ (0.030640 + 0.0664595 * xmach - 0.0881156 * xmach2);
			break;

		default:
			ssrd = 0.0;
		}

	PutSample(varp, ssrd);

}	/* END SSSRD */

/* END SSRD.C */
