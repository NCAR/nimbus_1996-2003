/*
-------------------------------------------------------------------------
OBJECT NAME:	time.c

FULL NAME:	Translate time to new record format

ENTRY POINTS:	xlhour(), xlmin(), xlsec()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	rec_decode.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"


/* -------------------------------------------------------------------- */
void xlhour(RAWTBL *varp, void *input, NR_TYPE *output)
{
	*output = (NR_TYPE)((Hdr_blk *)input)->hour;

}	/* END XLHOUR */

/* -------------------------------------------------------------------- */
void xlmin(RAWTBL *varp, void *input, NR_TYPE *output)
{
	*output = (NR_TYPE)((Hdr_blk *)input)->minute;

}	/* END XLMIN */

/* -------------------------------------------------------------------- */
void xlsec(RAWTBL *varp, void *input, NR_TYPE *output)
{
	*output = (NR_TYPE)((Hdr_blk *)input)->sec;

}	/* END XLSEC */

/* END TIME.C */
