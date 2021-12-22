/*
-------------------------------------------------------------------------
OBJECT NAME:	zero.c

FULL NAME:	Zero

ENTRY POINTS:	szero()
		sone()

STATIC FNS:	none

DESCRIPTION:	Return constants 0 & 1.

INPUT:		none

OUTPUT:		0.0

REFERENCES:	none

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1994
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "decode.h"
#include "amlib.h"


/* -------------------------------------------------------------------- */
void smissval(varp)
DERTBL	*varp;
{
	PutSample(varp, MISSING_VALUE);

}	/* END SMISSVAL */

/* -------------------------------------------------------------------- */
void sone(varp)
DERTBL	*varp;
{
	PutSample(varp, 1.0);

}	/* END SONE */

/* -------------------------------------------------------------------- */
void szero(varp)
DERTBL	*varp;
{
	PutSample(varp, 0.0);

}	/* END SZERO */

/* END ZERO.C */
