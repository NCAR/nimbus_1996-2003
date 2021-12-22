/*
-------------------------------------------------------------------------
OBJECT NAME:	tdl.c

FULL NAME:	

ENTRY POINTS:	xltdlmr(), xltdlat(), xltdlps()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	rec_decode.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1999
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"


/* -------------------------------------------------------------------- */
void xltdlmrs(RAWTBL *varp, void *input, NR_TYPE *output)
{
  *output = ntohf(((float *)input)[0]);

}	/* END XLTDLMR */

/* -------------------------------------------------------------------- */
void xltdlmrw(RAWTBL *varp, void *input, NR_TYPE *output)
{
  *output = ntohf(((float *)input)[1]);

}	/* END XLTDLMR */

/* -------------------------------------------------------------------- */
void xltdlps(RAWTBL *varp, void *input, NR_TYPE *output)
{
  *output = ntohf(((float *)input)[2]);

}	/* END XLTDLMR */

/* -------------------------------------------------------------------- */
void xltdlat(RAWTBL *varp, void *input, NR_TYPE *output)
{
  *output = ntohf(((float *)input)[3]);

}	/* END XLTDLMR */

/* END TDL.C */
