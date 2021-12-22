/*
-------------------------------------------------------------------------
OBJECT NAME:	spline.c

FULL NAME:	Produce Zero Points for Spline Data Set

ENTRY POINTS:	main()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		Nimbus netCDF file.

OUTPUT:		ASCII text file containing averages of each "zero" period.

REFERENCES:	libraf.a, libnetcdf.a

REFERENCED BY:	User

COMPILE:
 gcc -O2 -I/home/local/include spline.c -o spline -lnetcdf -lraf -lm

COPYRIGHT:	University Corporation for Atmospheric Research, 1994
-------------------------------------------------------------------------
*/

#include <math.h>
#include <netcdf.h>
#include <stdio.h>

#include "constants.h"		/* Check /home/local/include		*/

#define MAX_ZEROES	200

struct
	{
	float	x;
	float	y;
	} zero[MAX_ZEROES];

int	nRecords, zeroCnt = 0;

char	buffer[1024];		/* Generic volatile string space */

float	spline();
double	atof();

/* -------------------------------------------------------------------- */
main()
{
	int	i;
	int	InputFile;
	int	TimeDimension, modeID, valID;
	char	InputFileName[512], OutputFileName[512];
	char	varName[25], modeName[25];
	float	*mode, *data, *nzero, *corr, *cal;
	float	splineCoeff[MAX_ZEROES], slope, intercept;

	/* static so I can use aggregate initializer.
	 */
	static long	start[3] = {0, 0, 0};
	static long	count[3] = {1, 1, 1};

	ncopts = 0;

	/* Qeury user for various inforamtion and Open files.
	 */
	printf("Enter netCDF file name: ");
	gets(InputFileName);

	if ((InputFile = ncopen(InputFileName, NC_WRITE)) == ERR)
		{
		fprintf(stderr, "Can't open %s.\n", InputFileName);
		return(1);
		}

	printf("Enter variable name: ");
	strupr(gets(varName));

	printf("Enter mode variable name: ");
	strupr(gets(modeName));


	/* Init netCDF file.
	 */
	ncinquire(InputFile, NULL, NULL, NULL, &TimeDimension);
	ncdiminq(InputFile, TimeDimension, NULL, &nRecords);

	if ((modeID = ncvarid(InputFile, modeName)) == ERR)
		{
		printf("Variable [%s] does not exist in file.\n", modeName);
		return(1);
		}

	if ((valID = ncvarid(InputFile, varName)) == ERR)
		{
		printf("Variable [%s] does not exist in file.\n", varName);
		return(1);
		}


	/* Read all data at once and then scan array, and compute zero's
	 */
	printf("\nReading data...");

	data = (float *)GetMemory(sizeof(float) * nRecords);
	mode = (float *)GetMemory(sizeof(float) * nRecords);

	count[0] = nRecords;
	ncvarget(InputFile, modeID, start, count, (void *)mode);
	ncvarget(InputFile, valID, start, count, (void *)data);

	putchar('\n');

	computeZeroes(varName, data, mode);

	free((char *)mode);


	/* Ask user to delete any zero's
	 */
	for (i = 0; i < zeroCnt; ++i)
		printf("%02d %3.0f %f\n", i, zero[i].x, zero[i].y);

	do
		{
		printf("Enter index of zero to delete (Enter for none): ");
		gets(buffer);

		if (strlen(buffer) < 1)
			break;

		--zeroCnt;

		for (i = atoi(buffer); i < zeroCnt; ++i)
			memcpy(	(char *)&zero[i],
				(char *)&zero[i+1],
				sizeof(zero[0]));
		}
	while (1);


	/* Perform spline fit.
	 */
	printf("\nPerforming spline fit...");
	splineFit(splineCoeff);

	printf("\nEnter intercept: ");
	intercept = atof(gets(buffer));

	printf("Enter slope: ");
	slope = atof(gets(buffer));

	printf("\nGenerating zero, corr, & cal...");

	nzero= (float *)GetMemory(sizeof(float) * nRecords);
	corr = (float *)GetMemory(sizeof(float) * nRecords);
	cal  = (float *)GetMemory(sizeof(float) * nRecords);

	for (i = 0; i < nRecords; ++i)
		{
		nzero[i]= spline(splineCoeff, (float)i);
		corr[i]	= data[i] - nzero[i];
		cal[i]	= intercept + slope * corr[i];
		}


	/* Create new variable in netcdf file and output data.
	 */
	{
	int	zeroID, corrID, calID;

	strcpy(buffer, varName);
	strcat(buffer, "ZRO");
	if ((zeroID = ncvarid(InputFile, buffer)) == ERR)
		{
		fprintf(stderr, "Place holder %s does not exist.\n", buffer);
		return(1);
		}

	strcpy(buffer, varName);
	strcat(buffer, "COR");
	if ((corrID = ncvarid(InputFile, buffer)) == ERR)
		{
		fprintf(stderr, "Place holder %s does not exist.\n", buffer);
		return(1);
		}

	strcpy(buffer, varName);
	strcat(buffer, "CAL");
	if ((calID = ncvarid(InputFile, buffer)) == ERR)
		{
		fprintf(stderr, "Place holder %s does not exist.\n", buffer);
		return(1);
		}

	printf("\nWriting data...");
	ncvarput(InputFile, zeroID, start, count, (void *)nzero);
	ncvarput(InputFile, corrID, start, count, (void *)corr);
	ncvarput(InputFile, calID, start, count, (void *)cal);
	ncclose(InputFile);
	}

	free((char *)cal);
	free((char *)corr);
	free((char *)nzero);
	free((char *)data);

	putchar('\n');
	return(0);

}	/* END MAIN */

/* -------------------------------------------------------------------- */
computeZeroes(varName, data, mode)
char	varName[];
float	*data, *mode;
{
	register int	i;
	int		nToSkip, nCnt = 0;
	FILE		*OutputFile;
	float		ZeroModeValue, zeroSum = 0.0;
	char		OutputFileName[512];
	bool		beenAveraging = FALSE;

	printf("\nEnter mode zero value: ");
	ZeroModeValue = atof(gets(buffer));

	printf("Enter # points to skip at transition: ");
	nToSkip = atoi(gets(buffer));


	strcpy(OutputFileName, varName);
	strcat(OutputFileName, "_zero.pts");

	if ((OutputFile = fopen(OutputFileName, "w+")) == NULL)
		{
		fprintf(stderr, "Can't open %s.\n", OutputFileName);
		return(1);
		}
	else
		printf("\nOutput going to file '%s'.\n", OutputFileName);


	for (i = 0; i < nRecords; ++i)
		{
		if (mode[i] < ZeroModeValue-0.5 || mode[i] > ZeroModeValue+0.5)
			{
			if (beenAveraging)
				{
				if (nCnt > 10)
					{
					zero[zeroCnt].x = i - (nCnt / 2);
					zero[zeroCnt++].y = zeroSum / nCnt;

					fprintf(OutputFile, "%d %f\n",
							i, zeroSum / nCnt);
					}

				beenAveraging = FALSE;
				zeroSum = 0.0;
				nCnt = 0;
				}
			}
		else
			{
			if (beenAveraging == FALSE)
				{
				i += nToSkip;
				beenAveraging = TRUE;
				}

			zeroSum += data[i];
			++nCnt;
			}
		}

	fclose(OutputFile);

}	/* END COMPUTEZEROES */

/* -------------------------------------------------------------------- */
#define YP1	0.0
#define YPN	0.0

splineFit(coff)
float	coff[];
{
	register int	i;
	float		sig, p, qn, un, u[MAX_ZEROES];

	coff[0] = -0.5;
	u[0] =	(3.0 / (zero[1].x - zero[0].x)) *
		((zero[1].y - zero[0].y) / (zero[1].x - zero[0].x) - YP1);


	for (i = 1; i < zeroCnt - 1; ++i)
		{
		sig = (zero[i].x - zero[i-1].x) / (zero[i+1].x - zero[i-1].x);
		p = sig * coff[i-1] + 2.0;
		coff[i] = (sig - 1.0) / p;

		u[i] =	(zero[i+1].y - zero[i].y) / (zero[i+1].x - zero[i].x) -
			(zero[i].y - zero[i-1].y) / (zero[i].x - zero[i-1].x);

		u[i] =	(6.0 * u[i] / (zero[i+1].x - zero[i-1].x) -
			sig * u[i-1]) / p;
		}

	qn = 0.5;
	un =	(3.0 / (zero[zeroCnt-1].x - zero[zeroCnt-2].x)) *
		(YPN - (zero[zeroCnt-1].y - zero[zeroCnt-2].y) /
		(zero[zeroCnt-1].x - zero[zeroCnt-2].x));

	coff[zeroCnt-1] = (un - qn * u[zeroCnt-2]) / (qn * coff[zeroCnt-2]+1.0);

	for (i = zeroCnt-2; i >= 0; --i)
		coff[i] = coff[i] * coff[i+1] + u[i];

}	/* END SPLINEFIT */

/* -------------------------------------------------------------------- */
float spline(coff, x)
float	coff[];
float	x;
{
	float	y, h, a, b;
	int	k, klo, khi;

	klo = 0;
	khi = zeroCnt - 1;

	while (khi - klo > 1)
		{
		k = (khi + klo) >> 1;

		if (zero[k].x > x)
			khi = k;
		else
			klo = k;
		}

	if ((h = zero[khi].x - zero[klo].x) == 0.0)
		{
		fprintf(stderr, "Bad input.\n");
		exit(1);
		}

	a = (zero[khi].x - x) / h;
	b = (x - zero[klo].x) / h;

	y = a * zero[klo].y + b * zero[khi].y + ((pow(a, 3.0) - a) *
		coff[klo] + (pow(b, 3.0) - b) * coff[khi]) * (h*h) / 6.0;

	return(y);

}	/* END SPLINE */

/* END SPLINE.C */
