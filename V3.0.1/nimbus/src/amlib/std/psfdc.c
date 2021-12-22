/*********************** PSFDC ************************************  PSFDC
 ** Corrected static pressure (fuselage digital) (mb)
 	Input:
 		psfd - raw digital fuselage static pressure
 		qcx  - default raw dynamic pressure
 	Output:
 		psfdc - corrected static pressure
 	Include:
 		pcord - dynamic correction for digital static pressure
*/

#include "nimbus.h"
#include "amlib.h"

NR_TYPE (*pcorPSFD)();

/* -------------------------------------------------------------------- */
void spsfdc(varp)
DERTBL	*varp;
{
  NR_TYPE	psfd, qcx, psfdc;

  psfd	= GetSample(varp, 0);
  qcx	= GetSample(varp, 1);

  psfdc = psfd + (*pcorPSFD)(qcx);

  if (qcx < 10.0)
    psfdc = psfd;

  if (psfdc < 50.0)
    psfdc = 50.0;

  PutSample(varp, psfdc);

}	/* END SPSFDC */

/* END PSFDC.C */
