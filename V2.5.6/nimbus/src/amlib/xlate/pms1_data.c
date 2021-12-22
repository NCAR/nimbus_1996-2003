/*
-------------------------------------------------------------------------
OBJECT NAME:	pms1_data.c

FULL NAME:	Translate PMS1D to new record format (including Version 2)

ENTRY POINTS:	xlP16bin(), xlP16v2bin() xlP260bin(), xlP32v2bin(),
		xlP300bin(), xlP64v2bin()

STATIC FNS:	none

DESCRIPTION:	This sums up the data bins from SampleRate to 1hz.

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
void xlP16bin(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i, j;
  P16_blk *p = (P16_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    for (j = 0; j < varp->Length; ++j)
      np[(i*varp->Length)+j] = ntohs(p[i].bins[j]);

}	/* END XLP16BIN */

/* -------------------------------------------------------------------- */
void xlP16v2bin(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i, j;
  P16v2_blk *p = (P16v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    for (j = 0; j < varp->Length; ++j)
      np[(i*varp->Length)+j] = ntohs(p[i].bins[j]);

}	/* END XLP16V2BIN */

/* -------------------------------------------------------------------- */
void xlP260bin(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i, j;
  P260_blk *p = (P260_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    for (j = 0; j < varp->Length; ++j)
      np[(i*varp->Length)+j] = ntohs(p[i].bins[j]);

}	/* END XLP260BIN */

/* -------------------------------------------------------------------- */
void xlP32v2bin(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i, j;
  P32v2_blk *p = (P32v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    for (j = 0; j < varp->Length; ++j)
      np[(i*varp->Length)+j] = ntohs(p[i].bins[j]);

}	/* END XLP32V2BIN */

/* -------------------------------------------------------------------- */
void xlP300bin(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i, j;
  P300_blk *p = (P300_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    for (j = 0; j < varp->Length; ++j)
      np[(i*varp->Length)+j] = ntohs(p[i].bins[j]);

}	/* END XLP300BIN */

/* -------------------------------------------------------------------- */
void xlP64v2bin(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i, j;
  P64v2_blk *p = (P64v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    for (j = 0; j < varp->Length; ++j)
      np[(i*varp->Length)+j] = ntohs(p[i].bins[j]);

}	/* END XLP64V2BIN */

/* END PMS1_DATA.C */
