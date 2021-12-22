/*
-------------------------------------------------------------------------
OBJECT NAME:	pms1_hskp.c

FULL NAME:	Translate PMS1Dv2 houskeeping variables to Nimbus record
		format

ENTRY POINTS:	xlpms_hskp0(), xlpms_hskp1(), xlpms_hskp2(), xlpms_hskp3(),
		xlpms_hskp4(), xlpms_hskp5(), xlpms_hskp6(), xlpms_hskp7()

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

/* These #defines are not used, they are here for documentation
 */
/* FSSP housekeeping indices	*/
#define FSTB	0
#define FRST	1
#define FACT	2
#define FSST	3
#define FTMP	4
#define FANV	5
#define FSIG	6
#define FREF	7

/* PCASP houskeeping indices	*/
#define PACT	0
#define PSTT	1
#define PTMP	4
#define PREF	5
#define PFLW	6
#define PBRS	7

/* 260X houskeeping indices	*/
#define P6RST	0
#define P6TMP	4
#define P6E01	5
#define P6E64	6


/* -------------------------------------------------------------------- */
void xlpms16_hskp0(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P16v2_blk	*p = (P16v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[0]), varp->cof);

}	/* END XLPMS_HSKP0 */

/* -------------------------------------------------------------------- */
void xlpms16_hskp1(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P16v2_blk	*p = (P16v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[1]), varp->cof);

}	/* END XLPMS_HSKP1 */

/* -------------------------------------------------------------------- */
void xlpms16_hskp2(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P16v2_blk	*p = (P16v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[2]), varp->cof);

}	/* END XLPMS_HSKP2 */

/* -------------------------------------------------------------------- */
void xlpms16_hskp3(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P16v2_blk	*p = (P16v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[3]), varp->cof);

}	/* END XLPMS_HSKP3 */

/* -------------------------------------------------------------------- */
void xlpms16_hskp4(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P16v2_blk	*p = (P16v2_blk *)input;

  for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[4]) * varp->SampleRate, varp->cof);

}	/* END XLPMS_HSKP4 */

/* -------------------------------------------------------------------- */
void xlpms16_hskp5(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P16v2_blk	*p = (P16v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[5]) * varp->SampleRate, varp->cof);

}	/* END XLPMS_HSKP5 */

/* -------------------------------------------------------------------- */
void xlpms16_hskp6(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P16v2_blk	*p = (P16v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[6]) * varp->SampleRate, varp->cof);

}	/* END XLPMS_HSKP6 */

/* -------------------------------------------------------------------- */
void xlpms16_hskp7(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P16v2_blk	*p = (P16v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[7]) * varp->SampleRate, varp->cof);

}	/* END XLPMS_HSKP7 */


/* -------------------------------------------------------------------- */
void xlpms32_sum(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i, j, val;
  P32v2_blk	*p = (P32v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    {
    np[i] = 0.0;
    for (j = 0; j < 8; ++j)
      {
      val = ntohs(p[i].aux[j]);
      if (val > 16000)
        val -=  0x3ffe;
      np[i] += (NR_TYPE)val;
      }
    }

}	/* END XLPMS_HSKP0 */

/* -------------------------------------------------------------------- */
void xlpms32_hskp1(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int   i, val;
  P32v2_blk     *p = (P32v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    {
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[1]), varp->cof);
   if (np[i] > 16000)
      np[i] -= 16382;
    }

}       /* END XLPMS_HSKP1 */

/* -------------------------------------------------------------------- */
void xlpms32_hskp2(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int   i, val;
  P32v2_blk     *p = (P32v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    {
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[2]), varp->cof);
    if (np[i] > 16000)
      np[i] -= 16382;
    }

}       /* END XLPMS_HSKP2 */

/* -------------------------------------------------------------------- */
void xlpms64_hskp0(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P64v2_blk	*p = (P64v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[0]), varp->cof);

}	/* END XLPMS_HSKP0 */

/* -------------------------------------------------------------------- */
void xlpms64_hskp1(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P64v2_blk	*p = (P64v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[1]), varp->cof);

}	/* END XLPMS_HSKP1 */

/* -------------------------------------------------------------------- */
void xlpms64_hskp2(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P64v2_blk	*p = (P64v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[2]), varp->cof);

}	/* END XLPMS_HSKP2 */

/* -------------------------------------------------------------------- */
void xlpms64_hskp3(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P64v2_blk	*p = (P64v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[3]), varp->cof);

}	/* END XLPMS_HSKP3 */

/* -------------------------------------------------------------------- */
void xlpms64_hskp4(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P64v2_blk	*p = (P64v2_blk *)input;

  for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[4]) * varp->SampleRate, varp->cof);

}	/* END XLPMS_HSKP4 */

/* -------------------------------------------------------------------- */
void xlpms64_hskp5(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P64v2_blk	*p = (P64v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[5]) * varp->SampleRate, varp->cof);

}	/* END XLPMS_HSKP5 */

/* -------------------------------------------------------------------- */
void xlpms64_hskp6(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P64v2_blk	*p = (P64v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[6]) * varp->SampleRate, varp->cof);

}	/* END XLPMS_HSKP6 */

/* -------------------------------------------------------------------- */
void xlpms64_hskp7(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  P64v2_blk	*p = (P64v2_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)SecondPoly((NR_TYPE)ntohs(p[i].aux[7]) * varp->SampleRate, varp->cof);

}	/* END XLPMS_HSKP7 */

/* -------------------------------------------------------------------- */
void xlrejDOF(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  DMT100_blk	*p = (DMT100_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)ntohl(p[i].rejDOF);

}

/* -------------------------------------------------------------------- */
void xlrejAT(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  DMT100_blk	*p = (DMT100_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)ntohl(p[i].rejAvgTrans);

}

/* -------------------------------------------------------------------- */
void xloFlow(RAWTBL *varp, void *input, NR_TYPE *np)
{
  int	i;
  DMT100_blk	*p = (DMT100_blk *)input;

  for (i = 0; i < varp->SampleRate; ++i)
    np[i] = (NR_TYPE)ntohl(p[i].ADCoverflow);

}

/* END PMS1_HSKP.C */
