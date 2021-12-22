/*
-------------------------------------------------------------------------
OBJECT NAME:	rhola.c

FULL NAME:	Absolute humidity (Lyman-alpha) (g/m3)

ENTRY POINTS:	srhola()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		

OUTPUT:		

REFERENCES:	lyalf.c

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"

#define GL1	0.05
#define STPT	273.0	/* STP temperature reference (oK)		*/
#define CKO2	0.34	/* Absorption coefficient for oxygen		*/
#define CAMP	0.4630	/* LOG amplification factor			*/
#define STPP	1013.0	/* STP pressure reference (mb)			*/
#define CO2P	0.2315	/* Percentage of oxygen in dry air		*/

static NR_TYPE	rholap[2], corc1[2] = {0.0, 0.0};
static int	n1[2] = {0, 0};

static NR_TYPE	XC_1, XC_2, THRSH_1, THRSH_2, *CX_1, *CX_2;

static NR_TYPE compute_rhola();


/* -------------------------------------------------------------------- */
void lymanInit()
{
	XC_1 = (GetDefaultsValue("XC_1"))[0];
	XC_2 = (GetDefaultsValue("XC_2"))[0];
	THRSH_1 = (GetDefaultsValue("THRSH_1"))[0];
	THRSH_2 = (GetDefaultsValue("THRSH_2"))[0];
	CX_1 = GetDefaultsValue("CX_1");
	CX_2 = GetDefaultsValue("CX_2");

}	/* END LYMANINIT */

/* -------------------------------------------------------------------- */
void srhola(varp)
DERTBL	*varp;
{
	NR_TYPE	vla, psxc, atx, qcxc, rhodt;

	vla	= GetSample(varp, 0);
	psxc	= GetSample(varp, 1);
	atx	= GetSample(varp, 2);
	qcxc	= GetSample(varp, 3);
	rhodt	= GetSample(varp, 4);

	PutSample(varp, compute_rhola(0, varp, vla, psxc, atx, qcxc, rhodt,
				XC_1, THRSH_1, CX_1));

}	/* END SRHOLA */

/* -------------------------------------------------------------------- */
void srhola1(varp)
DERTBL	*varp;
{
	NR_TYPE	vla, psxc, atx, qcxc, rhodt;

	vla	= GetSample(varp, 0);
	psxc	= GetSample(varp, 1);
	atx	= GetSample(varp, 2);
	qcxc	= GetSample(varp, 3);
	rhodt	= GetSample(varp, 4);

	PutSample(varp, compute_rhola(1, varp, vla, psxc, atx, qcxc, rhodt,
				XC_2, THRSH_2, CX_2));

}	/* END SRHOLA1 */

/* -------------------------------------------------------------------- */
static NR_TYPE compute_rhola(indx, varp, vla, psxc, atx, qcxc, rhodt, XC, THRSH, CX)
int	indx;
DERTBL	*varp;
NR_TYPE	vla, psxc, atx, qcxc, rhodt;
NR_TYPE	XC, THRSH, CX[];
{
	NR_TYPE	rhola, rflag, rhota, rxpr1, vo2, vh2o, dfdla, del, errlr, ertst;

	rhola = rholap[indx];

	/* Calculate oxygen correction to VLA
	 */
	vo2 = ((CKO2*CO2P*XC*STPT) / (STPP*CAMP)) * psxc / (atx+273.16) ;
	vh2o = vla - vo2;

	if (n1[indx] < 300)
		++n1[indx];


	/* Estimate Lyman-Alpha humidity using empirical 3rd order fit
	 */
	rhota = (CX[0] + vh2o * (CX[1] + vh2o * (CX[2] + vh2o * CX[3]))) / XC;

	/* Take derivative of RHOTA with XC a constant	*/
	rxpr1 = (CX[1] + vh2o * (2.0 * CX[2] + vh2o * (3.0 * CX[3]))) / XC;

	if (rxpr1 < 0.1)
		rxpr1 = 0.1;

	/* Set recouple conditions	*/
	if (n1[indx] == 1)
		{
		rhola = rhota;
		corc1[indx] = 0.0;
		}

	/* Couple system and determine the correction for each time step */
	if ((dfdla = rhola - rhodt) == 0.0)
		del = 0.0;
	else
		{
		NR_TYPE	coup = exp(-GL1 * n1[indx]);
		NR_TYPE	tdel = 1.0 / varp->OutputRate;

		del = fabs((double)1.0 / rxpr1 * fabs((double)dfdla) *
							(coup + 0.001 * tdel));

		if (dfdla < 0.0)
			del = -del;
		}

	corc1[indx] += del;
	vh2o -= corc1[indx];

	/* Calculate corrected Lyman-alpha humidity	*/
	rhola = (CX[0] + vh2o * (CX[1] + vh2o * (CX[2] + vh2o * CX[3]))) / XC;

	/* Calculate mean error and test for recouple	*/
	rflag = 0.0;
	errlr = 100.0 * (rhola-rhodt) / rhodt;
	ertst = (NR_TYPE)fabs(errlr);

	if (n1[indx] < 10)
		ertst = 0.0;

/*	if (ertst > ((rhola < 0.5) ? THRSH * 5.0 : THRSH))
		{
		n1[indx] = 0;
		rflag = 1.0;
		}
*/
	if (qcxc < 25.0)
		{
		n1[indx] = 0;
		rflag = 1.1;
		}

	rholap[indx] = rhola;

	return(rhola);

}	/* END SRHOLA */

/* END RHOLA.C */
