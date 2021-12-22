/*
-------------------------------------------------------------------------
OBJECT NAME:	akrd.c

FULL NAME:	Attack Angle of the Radome

ENTRY POINTS:	sakrd()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	ComputeDerived()

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "raf.h"
#include "nimbus.h"
#include "amlib.h"

extern int	Aircraft;

/* -------------------------------------------------------------------- */
void sakrd(varp)
DERTBL	*varp;
{
	NR_TYPE	qcxc, adifr, pitch, thedot, pfactor;
	NR_TYPE	akrd, ratio;

	adifr	= GetSample(varp, 0);
	qcxc	= GetSample(varp, 1);
	ratio	= adifr / qcxc;


	switch (Aircraft)
		{
		case C130:

	if ( ratio >= -0.25)	
		akrd = (( ratio + 0.3898 ) / (0.06630));

	else
		akrd = 9.7576 + ratio * (40.7313 + ratio * 40.54222);
/*		akrd = ((ratio + 0.3837) / (0.06367));
		akrd = (( ratio + 0.4031) / (0.07895));		*/

/*			akrd = -19.4426 * pow(ratio, 3.0) -
				5.990 * pow(ratio, 2.0) +
				15.0 * ratio + 5.755; */

/*			akrd = (((adifr / qcxc) + 0.3953) / (0.07549));
			akrd = (((adifr / qcxc) + 0.37467) / (0.06297)); */

			break;

		case ELECTRA:
			akrd = ((adifr / qcxc) + 0.4095) / 0.07155;
			break;

		case KINGAIR:
			akrd = ((adifr / qcxc) + 0.01414) / 0.08485;
			break;

		case SABRELINER:
		case B57:
			{
			double	xmach2;

			xmach2 = GetSample(varp, 2);
			akrd = adifr / (qcxc * (0.06773-0.01578*sqrt(xmach2) +
							0.01843*xmach2));
			}
			break;

		default:
			akrd = 0.0;
		}

	PutSample(varp, akrd);

}	/* END SAKRD */

/* END AKRD.C */
