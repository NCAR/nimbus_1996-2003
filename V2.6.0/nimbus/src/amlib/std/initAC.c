/*
-------------------------------------------------------------------------
OBJECT NAME:	initAC.c

FULL NAME:	Initialize Aircraft Dependencies

ENTRY POINTS:	InitAircraftDependencies()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		none

OUTPUT:		Globals for AMLIB corrections

REFERENCES:	none

REFERENCED BY:	main(), dninp.c/input.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "raf.h"
#include "nimbus.h"
#include "amlib.h"

#define XMPHMS	0.44704    /* conversion factor mph to meters/sec */

extern int	Aircraft;

NR_TYPE	recfb, recff, recfkp, recfrh, recfrn, recfw, recfra;

NR_TYPE	(*pcorPSF)(), (*pcorQCF)(), (*pcorPSFD)(), (*pcorQCR)(), (*pcorQCW)(),
	(*pcorPSW)(), (*pcorPSB)(), (*pcorQCB)();

NR_TYPE	pcorw8(),pcorf8(),pcorw2(),pcorr2(),pcorb7(),pcorf7(),pcorr1(),pcorf1(),
	pcorf1_2(), pcorf1_3();

/* reference airspeed on J-W liquid water content converted from MPH
 * to m/s
 */
NR_TYPE	jwref;

NR_TYPE	tfher1,tfher2;

/* -------------------------------------------------------------------- */
InitAircraftDependencies()
{
	/* Set up default values first, then AC dependant.
	 */

	/* TFHER = function to compute deiced Rosemount total temperature.
	 * ERROR (C) as a function of Z, where Z is mach number adjusted by
	 * the density ratio (STP is reference with dry air assumed).
	 * Dual element model 102DB1CB: Electra uses SN34263
	 * Dual element model 102DB1CB:Kingair uses SN34260
	 * Single element model 102U: Sabreliner uses SN27581
	 * TFHER(Z) = 10.**(-1.7841*ALOG10(Z)-1.4025)    (308)
	 * TFHER(Z) = 10.**(-1.7841*ALOG10(Z)-1.4025)    (312)
	 * TFHER(Z) = 10.**(-1.7244*ALOG10(Z)-1.5989)    (307)
	 * The function is included in the subroutine.  Only the constants
	 * are set here 
	 */
	tfher1 = -1.7841;
	tfher2 = -1.4025;

	/* RECRN = recovery factor for reverse flow temp */
	recfrn = 0.625;

	/* RECFB = Rosemount 102E2AL (boom) recovery factor */
	recfb = 0.95;

	/* RECFF = Rosemount 102E2AL (fuselage) recovery factor */
	recff = 0.985;

	/* RECFKP = Fast response temp recovery factor  */
	recfkp = 0.80;

	/* RECFRH= Rosemount 102E2AL (Heated) recovery factor */
	recfrh = 0.98;

	/* RECFW = Rosemount 102E2AL (wing) recovery factor */
	recfw = 0.95;

	/* RECFRA = recovery factor for radome true air speed - depends on
	 * TT used use 0.95 for now until search code is installed
	 */
	recfra = 0.95;


	/* Set up aircraft dependant values.
	 */
	switch (Aircraft)
		{
		case KINGAIR:
			jwref	= 230 * XMPHMS;

			pcorQCW	= pcorw2;
			pcorPSW	= pcorw2;
			pcorPSF	= pcorr2;
			pcorQCF	= pcorr2;
			pcorQCR	= pcorr2;
			pcorPSFD= pcorr2;

			break;

		case ELECTRA:
			jwref	= 300 * XMPHMS;
			recfrn	= 0.65;

			pcorQCW	= pcorw8;
			pcorPSW	= pcorw8;
			pcorQCF	= pcorf8;
			pcorPSF	= pcorf8;
			pcorQCR	= pcorf8;
			pcorPSFD= pcorf8;

			break;

		case NOAA_G4:
			jwref	= 1 * XMPHMS;
			recfrn	= 0.65;

			pcorPSFD= pcorf1;
			pcorQCR	= pcorr1;
			pcorQCF	= pcorf1_2;
			pcorPSF	= pcorf1_3;
			pcorQCW	= NULL;
			pcorPSW	= NULL;

			break;

		case B57:
			pcorQCB	= pcorb7;
			pcorPSB	= pcorb7;

		case C130: case 300: case 600:
			jwref	= 1 * XMPHMS;
			recfrn	= 0.65;

			pcorPSFD= pcorf1;
			pcorQCR	= pcorr1;
			pcorQCF	= pcorf1_2;
			pcorPSF	= pcorf1_3;
			pcorQCW	= NULL;
			pcorPSW	= NULL;

			break;

		case SABRELINER:
			recff	= 0.95;
			jwref	= 200 * XMPHMS;
			tfher1	= -1.7244;
			tfher2	= -1.5989;

			pcorQCB	= pcorb7;
			pcorPSB	= pcorb7;
			pcorQCF	= pcorf7;
			pcorPSF	= pcorf7;
			pcorQCR	= pcorf7;
			pcorPSFD= pcorf7;

			break;

		case SAILPLANE:
			pcorQCW	= pcorw2;
			pcorPSW	= pcorw2;
			pcorPSF	= pcorr2;
			pcorQCF	= pcorr2;
			pcorQCR	= pcorr2;
			pcorPSFD= pcorr2;
			break;

		default:
			fprintf(stderr, "Unknown aircraft [%d] encountered.\n",
								Aircraft);
			exit(1);
		}

}	/* END INITAIRCRAFTDEPENDANCIES */

/* Electra ------------------------------------------------------------ */
NR_TYPE pcorw8(q)
NR_TYPE q;
{
/*	return(0.517 - 0.0202 * q); */
	return(1.23 - 0.0846 * q);
}

NR_TYPE pcorf8(q)
NR_TYPE q;
{
	return(0.366 - 0.0182 * q);
}

/* KingAir ------------------------------------------------------------ */
NR_TYPE pcorw2(q)
NR_TYPE q;
{
	return(-0.3112 - 0.0339 * q);
}

NR_TYPE pcorr2(q)
NR_TYPE q;
{
	return(-1.781 + (0.0655 - 0.00025 * q) * q);
}

/* SaberLiner --------------------------------------------------------- */
NR_TYPE pcorb7(q)
NR_TYPE q;
{
	return(0.9404 + q * (0.000239 * q - 0.078));
}

NR_TYPE pcorf7(q)
NR_TYPE q;
{
	return(-0.8901 + q * (0.0460 + 0.000075 * q));
}

/* C130 --------------------------------------------------------------- */
NR_TYPE pcorf1(q)		/* For PSFD */
NR_TYPE q;
{
/*	return(2.14);	From C130/Electra intercomp 6/95	*/
	return(3.66 - q * 0.01882);
}

NR_TYPE pcorr1(q)		/* For QCR */
NR_TYPE q;
{
/*	return(1.80);	From C130/Electra intercomp 6/95	*/	
	return(3.47 - q * 0.02865);
}

NR_TYPE pcorf1_3(q)	/* For PSF */
NR_TYPE q;
{
/*	return(1.26);	From C130/Electra intercomp 6/95	*/
	return(3.66 - q * 0.01882);
}

NR_TYPE pcorf1_2(q)	/* For QCF */
NR_TYPE q;
{

/*	return(0.40 + 0.0229 * q);	From C130/Electra intercomp 6/95 */
	return(2.59 - q * 0.01276);	
}

/* END INITAC.C */
