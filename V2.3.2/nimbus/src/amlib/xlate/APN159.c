/*
-------------------------------------------------------------------------
OBJECT NAME:	APN159.c

FULL NAME:	Translate APN159 analog/synchro variables.

ENTRY POINTS:	xlhgme(), xlchgme()

STATIC FNS:	none

DESCRIPTION:	

REFERENCES:	angcr()

REFERENCED BY:	rec_decode.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "raf.h"
#include "nimbus.h"
#include "amlib.h"

static NR_TYPE	chgme[250];	/* 250 = Max Rate	*/

NR_TYPE angcr();


/* -------------------------------------------------------------------- */
void xlhgme(RAWTBL *varp, void *p, NR_TYPE *output)
{
	int	i;
	short	*input = (short *)p;
	NR_TYPE	hgme;
	float	angle, start;

	angle	= 14.4;
	start	= 0.0;

	for (i = 0; i < varp->SampleRate; ++i)
		{
		hgme	= ~input[i * varp->ADSoffset] * RESOLV * 0.04;
		hgme	= angcr(hgme, chgme[i], angle, start);

		output[i] = hgme * 84.666667;
		}

}	/* END XLHGME */

/* -------------------------------------------------------------------- */
void xlchgme(RAWTBL *varp, void *p, NR_TYPE *output)
{
	int	i;
	short	*input = (short *)p;

	for (i = 0; i < varp->SampleRate; ++i)
		output[i] = chgme[i] = ~input[i * varp->ADSoffset] * RESOLV;

}	/* END XLCHGME */

/* END APN159.C */
