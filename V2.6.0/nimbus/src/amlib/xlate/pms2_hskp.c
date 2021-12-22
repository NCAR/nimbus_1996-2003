/*
-------------------------------------------------------------------------
OBJECT NAME:	pms2_hskp.c

FULL NAME:	Translate PMS2D houskeeping variables to Nimbus record
		format

ENTRY POINTS:	xl2d_hskp0(), xl2d_hskp1(), xl2d_hskp2(), xl2d_hskp3(),
		xl2d_hskp4(), xl2d_hskp5(), xl2d_hskp6(), xl2d_hskp7()

STATIC FNS:	none

DESCRIPTION:	

REFERENCES:	none

REFERENCED BY:	rec_decode.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1997
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"


/* -------------------------------------------------------------------- */
void xl2d_hskp0(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P2d_house	*p = (P2d_house *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].hdata[0]), varp->cof);

}	/* END XLPMS_HSKP0 */

/* -------------------------------------------------------------------- */
void xl2d_hskp1(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P2d_house	*p = (P2d_house *)input;

  for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].hdata[1]), varp->cof);

}	/* END XLPMS_HSKP1 */

/* -------------------------------------------------------------------- */
void xl2d_hskp2(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P2d_house	*p = (P2d_house *)input;

  for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].hdata[2]), varp->cof);

}	/* END XLPMS_HSKP2 */

/* -------------------------------------------------------------------- */
void xl2d_hskp3(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P2d_house	*p = (P2d_house *)input;

  for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].hdata[3]), varp->cof);

}	/* END XLPMS_HSKP3 */

/* -------------------------------------------------------------------- */
void xl2d_hskp4(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P2d_house	*p = (P2d_house *)input;

  for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].hdata[4]), varp->cof);

}	/* END XLPMS_HSKP4 */

/* -------------------------------------------------------------------- */
void xl2d_hskp5(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P2d_house	*p = (P2d_house *)input;

  for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].hdata[5]), varp->cof);

}	/* END XLPMS_HSKP5 */

/* -------------------------------------------------------------------- */
void xl2d_hskp6(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P2d_house	*p = (P2d_house *)input;

  for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].hdata[6]), varp->cof);

}	/* END XLPMS_HSKP6 */

/* -------------------------------------------------------------------- */
void xl2d_hskp7(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P2d_house	*p = (P2d_house *)input;

  for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].hdata[7]), varp->cof);

}	/* END XLPMS_HSKP7 */

/* -------------------------------------------------------------------- */
void xl2d_shadow(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P2d_house	*p = (P2d_house *)input;

  for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)ntohs(p[i].hdata[8]);

}	/* END XLPMS_HSKP7 */

/* END PMS2_HSKP.C */
