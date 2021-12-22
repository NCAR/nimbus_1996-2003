/*
-------------------------------------------------------------------------
OBJECT NAME:	calco.c

FULL NAME:		Apply Calibration Coefficients

ENTRY POINTS:	ApplyCalCoes()

STATIC FNS:		none

DESCRIPTION:	Perform application of calibration coeffiecients to the
				AveragedData or SampledData records.

INPUT:			data, int LOW_RATE/SAMPLE_RATE

OUTPUT:			modified input stream

REFERENCES:		none

REFERENCED BY:	LowRateLoop(), HighRateLoop(), winputops.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1994
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "decode.h"


/* -------------------------------------------------------------------- */
void ApplyCalCoes(NR_TYPE *record, int which_record)
{
	int		i, j, k, corder, pos;
	SDITBL	*sp;
	NR_TYPE	out;


	switch (which_record)
		{
		case LOW_RATE:
			for (i = 0; (sp = sdi[i]); ++i)
				{
/*				if (sp->type == 'C' || sp->type == 'O')
					continue;
*/
				corder  = sp->order - 1;
				out     = sp->cof[corder];

				record[sp->LRstart] =
				(record[sp->LRstart] - sp->convertOffset) * sp->convertFactor;

				for (k = 1; k < sp->order; k++)
					out = sp->cof[corder-k] + record[sp->LRstart] * out;

				record[sp->LRstart] = out;
				}

			break;

		case SAMPLE_RATE:
			for (i = 0; (sp = sdi[i]); ++i)
				{
/*				if (sp->type == 'C' || sp->type == 'O')
					continue;
*/
				pos = sp->SRstart;

				for (j = 0; j < sp->SampleRate; ++j, ++pos)
					{
					corder  = sp->order - 1;
					out     = sp->cof[corder];

					record[pos] = 
					(record[pos] - sp->convertOffset) * sp->convertFactor;

					for (k = 1; k < sp->order; k++)
						out = sp->cof[corder-k] + record[pos] * out;

					record[pos] = out;
					}
				}

			break;

		default:
			fprintf(stderr, "calco.c: coding fuck up.\n");
			exit(1);
		}

}	/* END APPLYCALCOES */

/* END CALCO.C */
