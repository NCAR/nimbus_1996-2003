/*
-------------------------------------------------------------------------
OBJECT NAME:	regret.c

FULL NAME:		Regressions.

ENTRY POINTS:	LinearLeastSquare()
				PolyLeastSquare()

STATIC FNS:		none

DESCRIPTION:	

INPUT:		

OUTPUT:		

REFERENCES:		none

REFERENCED BY:	spctrm.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1995
-------------------------------------------------------------------------
*/

#include <math.h>
#include <string.h>


/* -------------------------------------------------------------------- */
void LinearLeastSquare(float in[], float out[], int M)
{
	int		i, j;
	double	Ymean, Xmean, SXX, SXY, B0, B1;


	/* Compute x bar and y bar.
	 */
	Xmean = (double)M / 2.0;

	for (i = 0, Ymean = 0.0; i < M; ++i)
		Ymean += in[i];

	Ymean /= M;


	/* Compute SXX.
	 */
	for (i = 0, SXX = 0.0; i < M; ++i)
		SXX += i * i;

	SXX -= M * Xmean * Xmean;


	/* Compute SXY.
	 */
	for (i = 0, SXY = 0.0; i < M; ++i)
		SXY += i * in[i];

	SXY -= M * Xmean * Ymean;


	B1 = SXY / SXX;
	B0 = Ymean - B1 * Xmean;


	/* y[i] = B0 + B1 * x[i]
	 */
	for (i = 0; i < M; ++i)
		out[i] = B0 + B1 * i;

/*fprintf(stderr, "Ybar=%lf, Xbar=%lf, Slope=%lf, Yinter=%lf\n", Ymean, Xmean, B1, B0); */

}	/* END LINEARLEASTSQUARE */

/* END REGRET.C */
