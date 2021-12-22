/*
-------------------------------------------------------------------------
OBJECT NAME:	rec_decode.c

FULL NAME:		Decode ADS data Record

ENTRY POINTS:	DecodeADSrecord()
				DecodeADSrecordForRealTime()

DESCRIPTION:	Translates ADS data records into the internal format.
				The internal format is all one data type for ease of
				processing.  The 2 functions are essentially identical,
				the difference is that the one ForRealTime also puts the
				data into the arrays bits[] and volts[], for display
				purposes.

INPUT:			ADS logical record

OUTPUT:			New logical record

REFERENCES:		libxlate.a

REFERENCED BY:	lrloop.c hrloop.c winputops.c

NOTE:			If you chnage one, make sure the other does/doesn't need
				the same change.

COPYRIGHT:		University Corporation for Atmospheric Research, 1992-94
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
	short	*lrp;		/* ADS Logical Record Pointer	*/


	/* Cast SDI variables into new record
	 */
	for (i = 0; (sp = sdi[i]); ++i)
		{
		lrp	= &lr[sp->ADSstart];
		pos	= sp->SRstart;

		if (sp->type == 'C')
			for (j = 0; j < sp->SampleRate; ++j, ++pos)
				nlr[pos] = (ushort)lrp[j * sp->ADSoffset];
		else
			for (j = 0; j < sp->SampleRate; ++j, ++pos)
				nlr[pos] = lrp[j * sp->ADSoffset];
		}


	/* Extract block variables into new record
	 */
	for (i = 0; (rp = raw[i]); ++i)
		(*rp->xlate)(rp, &lr[rp->ADSstart], &nlr[rp->SRstart]);

}	/* END DECODERECORD */

/* -------------------------------------------------------------------- */
void DecodeADSrecordForRealtime(
	short	lr[],	/* ADS Logical Record	*/
	NR_TYPE	nlr[])	/* New Logical Record	*/
{
	int		i, j, pos;
	SDITBL	*sp;		/* Current SDI Variable Pointer	*/
	RAWTBL	*rp;		/* Current Raw Variable Pointer	*/
	short	*lrp;		/* ADS Logical Record Pointer	*/
	long	sum;

	extern NR_TYPE	*AveragedData;

	/* Cast SDI variables into new record.  Averaging in real-time is done
	 * slightly differently, so we are going to do it here, instead of calling
	 * AverageSampledData().
	 */
	for (i = 0; (sp = sdi[i]); ++i)
		{
		lrp	= &lr[sp->ADSstart];
		pos	= sp->SRstart;

		/* Move these two lines into the next 'for loop' if you want all the
		 * samples.  Since currently we only use one point pick off, no use
		 * decoding the rest.  Will also need to malloc more space in memalloc.c
		 * and change LRstart to SRstart.
		 */
		bits[sp->LRstart] = lrp[0];
		volts[sp->LRstart] = (NR_TYPE) (lrp[0] - sp->convertOffset) * sp->convertFactor;

		sum = 0;

		if (sp->type == 'C')
			for (j = 0; j < sp->SampleRate; ++j, ++pos)
				{
				nlr[pos] = (ushort)lrp[j * sp->ADSoffset];
				sum += (ushort)lrp[j * sp->ADSoffset];
				}
		else
			for (j = 0; j < sp->SampleRate; ++j, ++pos)
				{
				nlr[pos] = lrp[j * sp->ADSoffset];
				sum += lrp[j * sp->ADSoffset];
				}

		if (sp->type != 'C')
			sum /= sp->SampleRate;

		AveragedData[sp->LRstart] = sum;
		}


	/* Extract block variables into new record, again perform averaging locally.
	 */
	for (i = 0; (rp = raw[i]); ++i)
		{
		(*rp->xlate)(rp, &lr[rp->ADSstart], &nlr[rp->SRstart]);

        if (rp->SampleRate == 1)
            memcpy( (char *)&AveragedData[rp->LRstart],
                    (char *)&nlr[rp->SRstart],
                    NR_SIZE * rp->Length);
        else
            (*rp->Average)( &nlr[rp->SRstart],
                        &AveragedData[rp->LRstart],
                        rp->SampleRate,
                        rp->Length,
                        rp->Modulo);
		}

}	/* END DECODERECORDFORREALTIME */

/* END REC_DECODE.C */
