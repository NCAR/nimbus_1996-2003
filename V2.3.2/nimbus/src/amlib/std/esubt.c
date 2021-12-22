/*
-------------------------------------------------------------------------
OBJECT NAME:	esubt.c

FULL NAME:	

ENTRY POINTS:	esubt()

STATIC FNS:	none

DESCRIPTION:	ESUBT function implements vapor presr eqn computed by method
		of Paul R. Lowe (Journal of Applied Meteorology, Vol 16,
		PP 100-103, 1977)

INPUT:		Temperature

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	mr.c rhola.c rhum.c thetae.c thetav.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include <math.h>

double esubt(temperature)
double	temperature;
{
	if (temperature < -50.0)
		return(	4.4685 + temperature *
			(0.27347 + temperature *
			(6.83811E-3 + temperature *
			(8.7094E-5 + temperature *
			(5.63513E-7 + temperature * 1.47796E-9)))) );
	else
		return(	6.107799961 + temperature *
			(0.4436518521 + temperature *
			(0.01428945805 + temperature *
			(2.650648471E-4 + temperature *
			(3.031240396E-6 + temperature *
			(2.034080948E-8 + temperature * 6.136820929E-11))))) );

}	/* END ESUBT */

/* END ESUBT.C */
