/*
-------------------------------------------------------------------------
OBJECT NAME:	detrend.c

FULL NAME:		Detrending Functions

ENTRY POINTS:	Linear()
				Mean()

STATIC FNS:		none

DESCRIPTION:	

INPUT:		

OUTPUT:		

REFERENCES:		none

REFERENCED BY:	spctrm.c

COPYRIGHT:		Public Domain
-------------------------------------------------------------------------
*/

#include <math.h>
#include <string.h>

#include "define.h"
#include "spec.h"


/* -------------------------------------------------------------------- */
void DetrendNone(float in[], float out[], int M)
{
	memcpy((char *)out, (char *)in, M * sizeof(float));

}	/* END DETRENDNONE */

/* -------------------------------------------------------------------- */
void DetrendMean(float in[], float out[], int M)
{
	int		i, missCnt = 0;
	float	mean;
	double	sum;

	sum = 0.0;

	for (i = 0; i < M; ++i)
		if (in[i] != MISSING_VALUE)
			sum += in[i];
		else
			++missCnt;

	mean = sum / (M - missCnt);

	for (i = 0; i < M; ++i)
		if (in[i] != MISSING_VALUE)
			out[i] = in[i] - mean;
		else
			out[i] = 0.0;

}	/* END DETRENDMEAN */

/* -------------------------------------------------------------------- */
void DetrendLinear(float in[], float out[], int M)
{
	int		i;

	LinearLeastSquare(in, out, M);

	for (i = 0; i < M; ++i)
		if (in[i] != MISSING_VALUE)
			out[i] = in[i] - out[i];
		else
			out[i] = 0.0;

}	/* END DETRENDLINEAR */

/* END DETREND.C */
