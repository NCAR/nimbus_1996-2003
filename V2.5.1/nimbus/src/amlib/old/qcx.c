/*
-------------------------------------------------------------------------
OBJECT NAME:	qcx.c

FULL NAME:	

ENTRY POINTS:	sqcx()

STATIC FNS:	none

DESCRIPTION:	Sets QCX to whatever QC the user wants.  QCX is then fed
		into otehr variables for their calculations.

INPUT:		

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1993
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"

/* -------------------------------------------------------------------- */
void sqcx(varp)
DERTBL	*varp;
{
	PutSample(varp, GetSample(varp, 0));

}	/* END SQCX */

/* END QCX.C */
