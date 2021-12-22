/*
-------------------------------------------------------------------------
OBJECT NAME:	spec.h

FULL NAME:		Spectral Header File

DESCRIPTION:	Prototypes for Spectral Density and FFT functions.

COPYRIGHT:		University Corporation for Atmospheric Research, 1995
-------------------------------------------------------------------------
*/


#ifndef SPEC_H
#define SPEC_H

void Spectrum(float data[], double Pxx[], int K, int M, double (*window)());
void CoSpectrum(float data1[], float data2[], double Pxx[], double Qxx[], int K, int M, double (*window)());

void fft(double *real, double *imaginary, int power, int direction);

void DetrendLinear(float in[], float out[], int N);
void DetrendMean(float in[], float out[], int N);
void DetrendNone(float in[], float out[], int N);

void LinearLeastSquare(float in[], float out[], int M);

double Bartlett(int j, int N);
double Blackman(int j, int N);
double Hamming(int j, int N);
double Hanning(int j, int N);
double Parzen(int j, int N);
double Square(int j, int N);
double Welch(int j, int N);

#endif SPEC_H
