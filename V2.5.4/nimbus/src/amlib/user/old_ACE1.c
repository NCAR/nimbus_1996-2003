/*
-------------------------------------------------------------------------
OBJECT NAME:	func.c

FULL NAME:	

ENTRY POINTS:	sfunc()

STATIC FNS:	

DESCRIPTION:	Template for AMLIB std & user directories.

INPUT:		

OUTPUT:		

REFERENCES:	

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1995
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"

static NR_TYPE *CNDIV;

/* -------------------------------------------------------------------- */
void initCNs()
{
	CNDIV = GetDefaultsValue("CNDIV");

}

/* -------------------------------------------------------------------- */
void sdiv1(DERTBL *varp)
{
	NR_TYPE	x;

	x = GetSample(varp, 0);

	PutSample(varp, x / CNDIV[0] * 60);

}	/* END SFUNC */

/* -------------------------------------------------------------------- */
void sdiv2(DERTBL *varp)
{
	NR_TYPE	x;

	x = GetSample(varp, 0);

	PutSample(varp, x / CNDIV[1] * 60);

}	/* END SFUNC */

/* -------------------------------------------------------------------- */
void sdiv3(DERTBL *varp)
{
	NR_TYPE	x;

	x = GetSample(varp, 0);

	PutSample(varp, x / CNDIV[2] * 60);

}	/* END SFUNC */

/* -------------------------------------------------------------------- */
void sxfcn(DERTBL *varp)
{
	NR_TYPE	x;

	x = GetSample(varp, 0);

	PutSample(varp, 0.001312 * x + 0.034);

}	/* END SFUNC */

/* -------------------------------------------------------------------- */
void sfudge(DERTBL *varp)
{
	NR_TYPE	o3fp, o3ff, o3fv, fudge;

	o3fp = GetSample(varp, 0);
	o3ff = GetSample(varp, 1);

	fudge = -32767.0;

	if (o3fp > 1.0)
		o3fv = o3ff * 1013.25 / o3fp;
	else
		o3fv = -32767.0;	/* Missing Value	*/

	if (o3fv > 0.0)
		fudge = 1.269e+18 * ((1.422e-15 / o3fv ) + (o3fv / 1.211e+20));

	PutSample(varp, fudge);

}

/* END ACE1.C */
