/*
-------------------------------------------------------------------------
OBJECT NAME:	cnctr.c

FULL NAME:	CN Counter

ENTRY POINTS:	sconcn(), sxicnx(), sfcnc()

STATIC FNS:	none

DESCRIPTION:	Computation for the CN Counter, see Cindy Towhy for in
		depth information.

INPUT:		

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	compute.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1993-95
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"


static NR_TYPE	DIV;

/* -------------------------------------------------------------------- */
void cnctrInit(DERTBL *varp)
{
	DIV = (GetDefaultsValue("DIV"))[0];

}	/* END CNCTRINIT */

/* -------------------------------------------------------------------- */
void sconcn(varp)
DERTBL	*varp;
{
	NR_TYPE	concn, fcnc, cnts;

	cnts	= GetSample(varp, 0);
	fcnc	= GetSample(varp, 1);

	if (FeedBack == HIGH_RATE_FEEDBACK)
		cnts *= HIGH_RATE;

	concn	= cnts * DIV / (fcnc * 1000.0 / 60.0);

	/* Particle concentration corrected for coincidence
	 * 4.167E-6 is time in view volume (.25 microseconds)
	 * conversion factor for FCNC to cm3/s.
	 */
	concn *= (NR_TYPE)exp((double)(4.167e-6 * concn * fcnc));

	PutSample(varp, concn);

}	/* END SCONCN */

/* -------------------------------------------------------------------- */
void scFlow(varp)
DERTBL	*varp;
{
	NR_TYPE	flowc, flow, pressure, temperature;

	flow		= GetSample(varp, 0);
	pressure	= GetSample(varp, 1);
	temperature	= GetSample(varp, 2);

	if (pressure <= 0.0)
		pressure = 0.0001;

	/* Corrected sample flow rate in vlpm
	 */
	flowc = flow * (1013.25 / pressure) * ((temperature + 273.15) / 294.26);

	if (flowc <= 0.0)
		flowc = 0.0001;

	PutSample(varp, flowc);

}	/* END SCFLOW */

/* END CNCTR.C */
