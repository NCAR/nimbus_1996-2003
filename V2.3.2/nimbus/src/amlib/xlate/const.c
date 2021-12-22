/*
-------------------------------------------------------------------------
OBJECT NAME:	const.c

FULL NAME:	Constants

ENTRY POINTS:	none

STATIC FNS:	none

DESCRIPTION:	Globals file containing misc constants used by multiple
		routines.

COPYRIGHT:	University Corporation for Atmospheric Research, 1996
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"

#include <math.h>

const NR_TYPE FTMTR  = 0.3048;
const NR_TYPE MPS2   = 9.7959;
const NR_TYPE KTS2MS = 0.514791;
const NR_TYPE FTMIN  = 0.00508;

const double RAD_DEG	= 180.0 / M_PI;
const double DEG_RAD	= M_PI / 180.0;

/* Resolver/synchro conversion equation
 */
const NR_TYPE RESOLV	= 180.0 / 8192.0;

/* END CONST.C */
