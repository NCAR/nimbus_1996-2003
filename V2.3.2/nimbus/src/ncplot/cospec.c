/*
-------------------------------------------------------------------------
OBJECT NAME:	cospec.c

FULL NAME:		Compute Co-Spectrum

ENTRY POINTS:	CoSpectrum()

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
void CoSpectrum(float data1[],		/* Input data				*/
				float data2[],		/* Input data				*/
				double Pxx[],		/* Output CoSpectrum		*/
				double Qxx[],		/* Output Quadrature		*/
				int K,				/* K overlapping segments	*/
				int M,				/* 2M points per segment	*/
				double (*window)())	/* Window function			*/
{
	int		i, seg, segP, Base, twoM;
	double	KU, Wss, *Window;
	double	*currentSegment1, *imaginary1, *currentSegment2, *imaginary2;

	twoM = M << 1;

	currentSegment1 = (double *)GetMemory(sizeof(double) * twoM);
	currentSegment2 = (double *)GetMemory(sizeof(double) * twoM);
	imaginary1 = (double *)GetMemory(sizeof(double) * twoM);
	imaginary2 = (double *)GetMemory(sizeof(double) * twoM);


	/* Figure out 2^Base.
	 */
	for (i = 0; i < 32; ++i)
		if ((0x00000001 << i) & twoM)
			Base = i;


	for (i = 0; i <= M; ++i)
		Pxx[i] = Qxx[i] = 0.0;


	/* Generate Window & compute Window Squared and Summed.
	 */
	Window = (double *)GetMemory(sizeof(double) * twoM);

	for (i = 0, Wss = 0.0; i < twoM; ++i)
		{
		Window[i] = (*window)(i, twoM);
		Wss += Window[i] * Window[i];
		}

	KU = Wss * K * twoM;


	/* Do Segments.
	 */
	for (segP = seg = 0; seg < K; ++seg, segP += M)
		{
		for (i = 0; i < twoM; ++i)
			{
			currentSegment1[i] = data1[segP+i] * Window[i];
			currentSegment2[i] = data2[segP+i] * Window[i];
			imaginary1[i] = imaginary2[i] = 0.0;
			}

		fft(currentSegment1, imaginary1, Base, 1);
		fft(currentSegment2, imaginary2, Base, 1);

printf("Real 1       Real 2       Imagin 1     Imagin 2\n");
for (i = 0; i < twoM; ++i)
  printf("%e  %e  %e  %e\n", currentSegment1[i], currentSegment2[i],
		imaginary1[i], imaginary2[i]);


		Pxx[0] +=	(currentSegment1[0] * currentSegment2[0]) +
					(imaginary1[0] * imaginary2[0]);

		Qxx[0] +=	(imaginary1[0] * currentSegment2[0]) -
					(currentSegment1[0] * imaginary2[0]);

		for (i = 1; i < M; ++i)
			{
			Pxx[i] +=	(currentSegment1[i] * currentSegment2[i]) +
						(imaginary1[i] * imaginary2[i]) +
						(currentSegment1[twoM-i] * currentSegment2[twoM-i]) +
						(imaginary1[twoM-i] * imaginary2[twoM-i]);

			Qxx[i] +=	(imaginary1[i] * currentSegment2[i]) -
						(currentSegment1[i] * imaginary2[i]) -
						(imaginary1[twoM-i] * currentSegment2[twoM-i]) +
						(currentSegment1[twoM-i] * imaginary2[twoM-i]);
			}

		Pxx[M] +=	(currentSegment1[M] * currentSegment2[M]) +
					(imaginary1[M] * imaginary2[M]);

		Qxx[M] +=	(-imaginary1[M] * currentSegment2[M]) +
					(currentSegment1[M] * imaginary2[M]);
		}


	/* Normalize.
	 */
	for (i = 0; i <= M; ++i)
		{
		Pxx[i] /= KU;
		Qxx[i] /= -KU;		/* (-1) introduced by Al Cooper		*/
		}

	free(Window);
	free(imaginary2);
	free(imaginary1);
	free(currentSegment2);
	free(currentSegment1);

}	/* END COSPECTRUM */

/* END COSPEC.C */
