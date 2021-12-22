/*
-------------------------------------------------------------------------
OBJECT NAME:	spctrm.c

FULL NAME:		Compute Spectrum

ENTRY POINTS:	Spectrum()

STATIC FNS:		none

DESCRIPTION:	Power Spectral Density estimation via the Welch averaging
				periodogram.

INPUT:		

OUTPUT:		

REFERENCES:		fft.c

REFERENCED BY:	otto.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1995
-------------------------------------------------------------------------
*/

#include <math.h>

#include "define.h"
#include "spec.h"

/* -------------------------------------------------------------------- */
void Spectrum(	float data[],		/* Input data				*/
				double Pxx[],		/* Output data				*/
				int K,				/* K overlapping segments	*/
				int M,				/* 2M points per segment	*/
				double (*window)())	/* Window function			*/
{
	int		i, seg, segP, Base, twoM;
	double	KU, Wss, *currentSegment, *imaginary, *Window;

	twoM = M << 1;

	currentSegment = (double *)GetMemory(sizeof(double) * twoM);
	imaginary = (double *)GetMemory(sizeof(double) * twoM);


	/* Figure out 2^Base.
	 */
	for (i = 0; i < 32; ++i)
		if ((0x00000001 << i) & twoM)
			Base = i;


	/* Ouptut will consist of M + 1 points.
	 */
	for (i = 0; i <= M; ++i)
		Pxx[i] = 0.0;


	/* Generate Window & compute Window Squared and Summed.
	 */
	Window = (double *)GetMemory(sizeof(double) * twoM);

	for (i = 0, Wss = 0.0; i < twoM; ++i)
		{
		Window[i] = (*window)(i, twoM);
		Wss += Window[i] * Window[i];
		}

	KU = K * Wss * twoM;


	/* Do Segments.
	 */
	for (segP = seg = 0; seg < K; ++seg, segP += M)
		{
		for (i = 0; i < twoM; ++i)
			{
			currentSegment[i] = data[segP+i] * Window[i];
			imaginary[i] = 0.0;
			}

		fft(currentSegment, imaginary, Base, 1);

		Pxx[0] +=	(currentSegment[0] * currentSegment[0]) +
					(imaginary[0] * imaginary[0]);

		for (i = 1; i < M; ++i)
			Pxx[i] +=	(currentSegment[i] * currentSegment[i]) +
						(imaginary[i] * imaginary[i]) +
						(currentSegment[twoM-i] * currentSegment[twoM-i]) +
						(imaginary[twoM-i] * imaginary[twoM-i]);

		Pxx[M] +=	(currentSegment[M] * currentSegment[M]) +
					(imaginary[M] * imaginary[M]);
		}


	/* Normalize.
	 */
	for (i = 0; i <= M; ++i)
		Pxx[i] /= KU;

	free(Window);
	free(imaginary);
	free(currentSegment);

}	/* END SPECTRUM */

/* END SPCTRM.C */
