/*
-------------------------------------------------------------------------
OBJECT NAME:	palt.c

FULL NAME:	NACA pressure altitude (M)

ENTRY POINTS:	spalt()
		spaltf()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "raf.h"
#include "nimbus.h"
#include "amlib.h"

static NR_TYPE ASTG = 1013.246; /* Default altimeter setting (mb)	*/
static NR_TYPE SFCT = 288.15; /* Default mean surface temp setting (K)*/

extern int	Aircraft;

/* -------------------------------------------------------------------- */
void initPalt(DERTBL *varp)
{
	float	tmp;

	if ((tmp = (GetDefaultsValue("ASTG"))[0]) > 1.0)
		ASTG = tmp;

}	/* END INITPALT */

/* -------------------------------------------------------------------- */
void spalt(varp)
DERTBL	*varp;
{
	NR_TYPE	psxc, palt;

	psxc = GetSample(varp, 0);

	switch (Aircraft)
		{
		case B57:
			palt = psxc / ASTG;

			if (palt >= 0.223198605)
				palt = 44308.0 * (1.0 - pow(palt, 0.190284));
			else
				palt = 11000.0 + 14600.0 * log10(0.223198605 / palt);
			break;

		default:
			palt = 153.77 * SFCT *
			(1.0 - pow((double)psxc / ASTG, 0.190284));
		}

	PutSample(varp, palt);

}	/* END SPALT */

/* -------------------------------------------------------------------- */
void spaltf(varp)
DERTBL	*varp;
{
	PutSample(varp, GetSample(varp, 0) * 3.2808399);

}	/* END SPALTF */

/* END PALT.C */
