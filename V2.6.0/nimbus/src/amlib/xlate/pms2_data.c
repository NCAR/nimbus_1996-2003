/*
-------------------------------------------------------------------------
OBJECT NAME:	pms2_data.c

FULL NAME:	Translate PMS2D records to look like 1D histograms

ENTRY POINTS:	xlP16bin(), xlP16v2bin() xlP260bin(), xlP32v2bin(),
		xlP300bin(), xlP64v2bin()

STATIC FNS:	none

DESCRIPTION:	This sums up the data bins from SampleRate to 1hz.

INPUT:		

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	rec_decode.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1999-2000
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"


/* -------------------------------------------------------------------- */
void xlTwoD(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int		i, j, msec;
  ulong		*p;
  static P2d_rec	rec;


}	/* END XLTWOD */

/* -------------------------------------------------------------------- */
void xlTwoDG(RAWTBL *varp, void *input, NR_TYPE *np)
{

}	/* END XLTWODG */

/* -------------------------------------------------------------------- */
void xlTwoDH(RAWTBL *varp, void *input, NR_TYPE *np)
{

}	/* END XLTWODH */

/* END PMS2_DATA.C */
