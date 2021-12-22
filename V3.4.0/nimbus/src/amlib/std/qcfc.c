/**************************** QCFC *******************************  QCFC
 ** Corrected dynamic pressure (fuselage) (mb)
 	Input:
 		qcf - raw dynamic pressure, fuselage mounted
 	Output:
 		qcfc - corrected dynamic pressure, fuselage
 	Include:
 		pcorf - dynamic pressure correction, fuselage
*/

#include "nimbus.h"
#include "amlib.h"

NR_TYPE (*pcorQCF)(NR_TYPE);

/* -------------------------------------------------------------------- */
void sqcfc(varp)
DERTBL	*varp;
{
  NR_TYPE	qcf, qcfc;

  qcf	= GetSample(varp, 0);
  qcfc	= qcf - (*pcorQCF)(qcf);

  if (qcfc < 10.0)
    qcfc = qcf;

  if (qcfc < 0.001)
    qcfc =  0.001;

  PutSample(varp, qcfc);
}
