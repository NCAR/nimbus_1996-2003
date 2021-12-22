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
void xlpms_hskp0(RAWTBL *varp, void *input, NR_TYPE *np)
{
	int	i;
	P16v2_blk	*p = (P16v2_blk *)input;

	for (i = 0; i < varp->SampleRate; ++i)
		np[i] = (NR_TYPE)SecondPoly((NR_TYPE)p[i].aux[0], varp->cof);

}	/* END XLPMS_HSKP0 */

/* -------------------------------------------------------------------- */
void xlpms_hskp1(RAWTBL *varp, void *input, NR_TYPE *np)
{
	int	i;
	P16v2_blk	*p = (P16v2_blk *)input;

	for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
		np[i] = (NR_TYPE)SecondPoly((NR_TYPE)p[i].aux[1], varp->cof);

}	/* END XLPMS_HSKP1 */

/* -------------------------------------------------------------------- */
void xlpms_hskp2(RAWTBL *varp, void *input, NR_TYPE *np)
{
	int	i;
	P16v2_blk	*p = (P16v2_blk *)input;

	for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
		np[i] = (NR_TYPE)SecondPoly((NR_TYPE)p[i].aux[2], varp->cof);

}	/* END XLPMS_HSKP2 */

/* -------------------------------------------------------------------- */
void xlpms_hskp3(RAWTBL *varp, void *input, NR_TYPE *np)
{
	int	i;
	P16v2_blk	*p = (P16v2_blk *)input;

	for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
		np[i] = (NR_TYPE)SecondPoly((NR_TYPE)p[i].aux[3], varp->cof);

}	/* END XLPMS_HSKP3 */

/* -------------------------------------------------------------------- */
void xlpms_hskp4(RAWTBL *varp, void *input, NR_TYPE *np)
{
	int	i;
	P16v2_blk	*p = (P16v2_blk *)input;

	for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
		np[i] = (NR_TYPE)SecondPoly((NR_TYPE)p[i].aux[4], varp->cof);

}	/* END XLPMS_HSKP4 */

/* -------------------------------------------------------------------- */
void xlpms_hskp5(RAWTBL *varp, void *input, NR_TYPE *np)
{
	int	i;
	P16v2_blk	*p = (P16v2_blk *)input;

	for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
		np[i] = (NR_TYPE)SecondPoly((NR_TYPE)p[i].aux[5], varp->cof);

}	/* END XLPMS_HSKP5 */

/* -------------------------------------------------------------------- */
void xlpms_hskp6(RAWTBL *varp, void *input, NR_TYPE *np)
{
	int	i;
	P16v2_blk	*p = (P16v2_blk *)input;

	for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
		np[i] = (NR_TYPE)SecondPoly((NR_TYPE)p[i].aux[6], varp->cof);

}	/* END XLPMS_HSKP6 */

/* -------------------------------------------------------------------- */
void xlpms_hskp7(RAWTBL *varp, void *input, NR_TYPE *np)
{
	int	i;
	P16v2_blk	*p = (P16v2_blk *)input;

	for (i = 0, np[0] = 0.0; i < varp->SampleRate; ++i)
		np[i] = (NR_TYPE)SecondPoly((NR_TYPE)p[i].aux[7], varp->cof);

}	/* END XLPMS_HSKP7 */

/* END PMS1_HSKP.C */
