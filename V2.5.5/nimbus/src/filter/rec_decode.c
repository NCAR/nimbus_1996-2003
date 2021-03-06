/*
-------------------------------------------------------------------------
OBJECT NAME:	rec_decode.c

FULL NAME:	Decode ADS data Record

ENTRY POINTS:	DecodeADSrecord()
		DecodeADSrecordForRealTime()

DESCRIPTION:	Translates ADS data records into the internal format.
		The internal format is all one data type for ease of
		processing.  The 2 functions are essentially identical,
		the difference is that the one ForRealTime also puts the
		data into the arrays bits[] and volts[], for display
		purposes.

INPUT:		ADS logical record

OUTPUT:		New logical record

REFERENCES:	libxlate.a

REFERENCED BY:	lrloop.c hrloop.c winputops.c

NOTE:		If you chnage one, make sure the other does/doesn't need
		the same change.

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-97
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "decode.h"

extern ushort	*bits;
extern NR_TYPE	*volts;


/* -------------------------------------------------------------------- */
void DecodeADSrecord(
	short	lr[],	/* ADS Logical Record	*/
	NR_TYPE	nlr[])	/* New Logical Record	*/
{
  int		i, j, pos;
  SDITBL	*sp;		/* Current SDI Variable Pointer	*/
  RAWTBL	*rp;		/* Current Raw Variable Pointer	*/
  short		*lrp;		/* ADS Logical Record Pointer	*/


  /* Cast SDI variables into new record
   */
  for (i = 0; (sp = sdi[i]); ++i)
    {
    lrp	= &lr[sp->ADSstart];
    pos	= sp->SRstart;

    if (Mode == REALTIME)
      {
      bits[sp->LRstart] = ntohs(lrp[0]);
      volts[sp->LRstart] = (NR_TYPE)((short)ntohs(lrp[0]) - sp->convertOffset) * sp->convertFactor;
      }

    if (sp->type[0] == 'C')
      {
      if (strcmp(sp->type, "C24") == 0)
        {
        for (j = 0; j < sp->SampleRate; ++j, ++pos)
          nlr[pos] = (NR_TYPE)ntohl(*((unsigned long *)&lrp[j * sp->ADSoffset]));
        }
      else
        {
        for (j = 0; j < sp->SampleRate; ++j, ++pos)
          nlr[pos] = (NR_TYPE)ntohs(lrp[j * sp->ADSoffset]);
        }
      }
    else
      {
      if (strcmp(sp->type, "D20") == 0)
        {
        for (j = 0; j < sp->SampleRate; ++j, ++pos)
          nlr[pos] = (short)ntohl(*((long *)&lrp[j * sp->ADSoffset]));
        }
      else
        {
        for (j = 0; j < sp->SampleRate; ++j, ++pos)
          nlr[pos] = (short)ntohs((ushort)lrp[j * sp->ADSoffset]);
        }
      }
    }


  /* Extract block variables into new record
   */
  for (i = 0; (rp = raw[i]); ++i)
    (*rp->xlate)(rp, &lr[rp->ADSstart], &nlr[rp->SRstart]);

}	/* END DECODEADSRECORD */

/* END REC_DECODE.C */
