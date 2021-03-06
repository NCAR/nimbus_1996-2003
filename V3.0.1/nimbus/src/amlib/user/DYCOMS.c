/*
-------------------------------------------------------------------------
OBJECT NAME:	ACEASIA.c

FULL NAME:	ACE-Asis

ENTRY POINTS:	sfunc()

STATIC FNS:  

DESCRIPTION:	Template for AMLIB std & user directories.

INPUT:    

OUTPUT:    

REFERENCES:  

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 2001
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"

static const float	f = 0.52,
			C1 = 1.0,
			C2 = 0.299,
			C3 = 1.0,
			C4 = 0.297,
			C5 = 24.9;

/* -------------------------------------------------------------------- */
void sCing(DERTBL *varp)
{
  NR_TYPE	F, B, cF, cB, out;

  F = GetSample(varp, 0);
  B = GetSample(varp, 1);
  cF = GetSample(varp, 2);
  cB = GetSample(varp, 3);

  out = f + ((cF * C3) - (cB * C4)) * ((1 - f)) / ((F * C1) + (B * C2));

  PutSample(varp, out);

}

/* -------------------------------------------------------------------- */
void sCinsig(DERTBL *varp)
{
  NR_TYPE	F, B, cF, cB, out;

  F = GetSample(varp, 0);
  B = GetSample(varp, 1);

  out = C5 * ((F * C1) + (B * C2)) / (1.0 - f);

  PutSample(varp, out);

}

/* END DYCOMS.C */
