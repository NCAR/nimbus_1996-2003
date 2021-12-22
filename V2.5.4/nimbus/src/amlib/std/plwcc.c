/**************************** PLWCC *******************************  PLWCC
 ** Corrected PMS liquid water content (g/m3)
   Input:
     plwc - raw liquid water content
     tasx - derived true air speed
     atx - derived ambient temperature
     psxc - derived static pressure
   Output:
     plwcc - corrected PMS liquid water content
*/

#include "nimbus.h"
#include "amlib.h"
#include "raf.h"
 
/*  Values from /home/local/proj/Defaults on 23 April 1998           RLR */
static NR_TYPE  twire[2] = {155.0, 155.0}; /* Wire temperature (K)       */
static NR_TYPE  tasFac[2] = {1.0, 1.0};   /* True airspeed factor (none) */

NR_TYPE  kinglwcc();

/* -------------------------------------------------------------------- */
void plwccInit(DERTBL *varp)
{
  NR_TYPE  *tmp;
  if ((tmp = GetDefaultsValue("TWIRE_PMS", varp->name)) == NULL)
  {
    sprintf(buffer, "Value set to %f in AMLIB function plwccInit.\n", twire[0]);
    LogMessage(buffer);
  }
  else
    twire[0] = tmp[0];

  if ((tmp = GetDefaultsValue("TWIRE_PMS1", varp->name)) == NULL)
  {
    sprintf(buffer, "Value set to %f in AMLIB function plwccInit.\n", twire[1]);
    LogMessage(buffer);
  }
  else
    twire[1] = tmp[0];

  if ((tmp = GetDefaultsValue("TWIRE_TASFAC", varp->name)) == NULL)
  {
    sprintf(buffer, "Value set to %f in AMLIB function plwccInit.\n", tasFac[0]);
    LogMessage(buffer);
  }
  else
    tasFac[0] = tmp[0];

  if ((tmp = GetDefaultsValue("TWIRE_TASFAC1", varp->name)) == NULL)
  {
    sprintf(buffer, "Value set to %f in AMLIB function plwccInit.\n", tasFac[1]);
    LogMessage(buffer);
  }
  else
    tasFac[1] = tmp[0];

}  /* END PLWCCINIT */

/* -------------------------------------------------------------------- */
void splwcc(varp)
DERTBL  *varp;
{
  NR_TYPE  plwc, tasx, atx, psxc;

  plwc  = GetSample(varp, 0);
  tasx  = GetSample(varp, 1) * tasFac[0];
  atx  = GetSample(varp, 2);
  psxc  = GetSample(varp, 3);
 
  PutSample(varp, kinglwcc(plwc, tasx, atx, psxc, twire[0]));

}  /* END SPLWCC */

/* -------------------------------------------------------------------- */
void splwcc1(varp)
DERTBL  *varp;
{
  NR_TYPE  plwc, tasx, atx, psxc;

  plwc  = GetSample(varp, 0);
  tasx  = GetSample(varp, 1) * tasFac[1];
  atx  = GetSample(varp, 2);
  psxc  = GetSample(varp, 3);
 
  PutSample(varp, kinglwcc(plwc, tasx, atx, psxc, twire[1]));

}  /* END SPLWCC1 */

/* END PLWCC.C */
