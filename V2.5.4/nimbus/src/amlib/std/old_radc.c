/*
:set ts=4
-------------------------------------------------------------------------
OBJECT NAME:	radc.c

FULL NAME:		Radiation Data Corrections

ENTRY POINTS:	radcInit(), sradcf(), sradc()

STATIC FNS:		

DESCRIPTION:	Calculate aircraft attitude corrections for hemispheric
				 radiation data

INPUT:			PITCH ROLL THF SOLAZ SOLZE

OUTPUT:			RADCF RADC 

REFERENCES:		

REFERENCED BY:	compute.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1997

ORIG. AUTHOR:	Ron Ruth
-------------------------------------------------------------------------
 */

#include <stdio.h>
#include <math.h>
#include "nimbus.h"
#include "amlib.h"
#include "decode.h"

static NR_TYPE	PMAX =  6.0;	/* radiation correction pitch max threshold */
static NR_TYPE	PMIN = -6.0;	/* radiation correction pitch min threshold */
static NR_TYPE	RMAX =  6.0;	/* radiation correction roll max threshold */
static NR_TYPE	RMIN = -6.0;	/* radiation correction roll min threshold */
static NR_TYPE	SWDIFF = 0.34;	/* Short-wave diffuse radiation fraction */
static NR_TYPE	SWDIRF = 0.66;	/* Short-wave direct radiation fraction */
/* const double	DEG_RAD = M_PI/180.0;	* in file "const.c" */

static NR_TYPE	radcf;	/* radiation correction factor */

/* -------------------------------------------------------------------- */
void radcInit(DERTBL *varp)
{
	float	tmp;
	if ((tmp = (GetDefaultsValue("RAD_PITCH_MAX"))[0]) != 0.0)
		PMAX = tmp;
	if ((tmp = (GetDefaultsValue("RAD_PITCH_MIN"))[0]) != 0.0)
		PMIN = tmp;
	if ((tmp = (GetDefaultsValue("RAD_ROLL_MAX"))[0]) != 0.0)
		RMAX = tmp;
	if ((tmp = (GetDefaultsValue("RAD_ROLL_MIN"))[0]) != 0.0)
		RMIN = tmp;
	if ((tmp = (GetDefaultsValue("SWDIFF"))[0]) != 0.0)
		SWDIFF = tmp;
	if ((tmp = (GetDefaultsValue("SWDIRF"))[0]) != 0.0)
		SWDIRF = tmp;

/*  Temporary diagnostic output 
	printf (" default values used by radc.c:\n");
	printf ("   RAD_PITCH_MAX = %f\n",PMAX);
	printf ("   RAD_PITCH_MIN = %f\n",PMIN);
	printf ("    RAD_ROLL_MAX = %f\n",RMAX);
	printf ("    RAD_ROLL_MIN = %f\n",RMIN);
	printf ("          SWDIFF = %f\n",SWDIFF);
	printf ("          SWDIRF = %f\n",SWDIRF);
 */
 
}	/* END RADCINIT  */

/* -------------------------------------------------------------------- */
void sradcf(DERTBL *varp)
/*  Radiation correction factor */
{

	double	pitch, roll, thdg, solze, solaz;
	double	tr, tp, tz, amt, cf1, cf2;

	pitch = (double)GetSample(varp, 0);			/* Aircraft pitch (deg) */
	roll  = (double)GetSample(varp, 1);			/* Aircraft roll (deg) */
	thdg  = (double)GetSample(varp, 2)*DEG_RAD;	/* Aircraft true heading (rad)*/
	solze = (double)GetSample(varp, 3);			/* Solar zenith angle (rad) */
	solaz = (double)GetSample(varp, 4);			/* Solar azimuth angle (rad) */

/* Check pitch and roll thresholds */
	if ( pitch > (double)PMAX || roll > (double)RMAX ||
		 pitch < (double)PMIN || roll < (double)RMIN )
	{
		radcf = MISSING_VALUE;
	}
	else
	{
/* Aircraft attitude correction factor */
    	tr  = tan((roll)*DEG_RAD);
    	tp  = tan((pitch)*DEG_RAD);
		tz  = tan(solze);
/*  Transform solar azimuth into aircraft coordinates */
    	amt = M_PI - solaz - thdg;
		cf1 = sqrt(1.0 + tr*tr + tp*tp);
		cf2 =  1.0 + tr*tz*sin(amt) - tp*tz*cos(amt);
		if (cf2 != 0.0)
			radcf = (NR_TYPE)(cf1/cf2);
		else
			radcf = MISSING_VALUE;
	}
	PutSample(varp, radcf);
/*	printf (" PutSample radiation correction factor = %f\n",radcf); */

}   /* END RADCF  */

/* -------------------------------------------------------------------- */
void sradc(DERTBL *varp)
{

	NR_TYPE rad;	/* Uncorrected radiation value */

/* Output a corrected value only of radcf is valid. */
	if (radcf != MISSING_VALUE)
	{
		rad = GetSample(varp, 0);
		PutSample(varp, (rad*SWDIFF + rad*SWDIRF*radcf));
	}
	else
	{
		PutSample(varp, MISSING_VALUE);
	}

}	/* END SRADC */
