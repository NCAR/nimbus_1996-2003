/************************ QCRC ***********************************  QCRC
 ** CORRECTED RADOME DYNAMIC PRESSURE (MB) -- KING AIR 312
 ** REQUIRES:1-QCR;2-ADIFR;3-BDIFR
 	Input:
 		qcr - raw radome dynamic pressure
 		adifr - raw differential pressure
 		bdifr - raw differential pressure
 	Output:
 		qcrc - corrected radome dynamic pressure
 	Include:
 		pcorr - radome static defect correction
*/


#include "nimbus.h"
#include "amlib.h"
#include "raf.h"

NR_TYPE	(*pcorQCR)();

extern int	Aircraft;

/* -------------------------------------------------------------------- */
void sqcrc(varp)
DERTBL	*varp;
{
	NR_TYPE	qcr, qcrc, adifr, bdifr;
	NR_TYPE	bqcrc, satk3, sbeta3;
	double	atk3, beta3;

	qcr	= GetSample(varp, 0);
	adifr	= GetSample(varp, 1);
	bdifr	= GetSample(varp, 2);

	if (qcr < 0.001)
		qcr = 0.1;

	switch (Aircraft)
		{
		case C130:
			qcrc = qcr - (*pcorQCR)(qcr);
			break;

		case ELECTRA:
			atk3 = fabs((double)((adifr / qcr) + 0.4095) / 0.0715);
			beta3= fabs((double)((bdifr / qcr) + 0.0375) / 0.06577);
			satk3	= sin(atk3 * DEG_RAD);
			sbeta3	= sin(beta3 * DEG_RAD);
			bqcrc	= (1.0 - 2.25 * satk3 * satk3) *
				  (1.0 - 2.25 * sbeta3 * sbeta3);

			if (bqcrc == 0.0)
				bqcrc = 0.0001;

			qcrc	= (qcr * (1.02633 - 0.00819 * atk3) -
						(*pcorQCR)(qcr)) / bqcrc;	
			break;

		case KINGAIR:
			atk3	= fabs((double)adifr / (qcr * 0.08207));
			beta3	= fabs((double)bdifr / (qcr * 0.07448));
			satk3	= sin(atk3 * DEG_RAD);
			sbeta3	= sin(beta3 * DEG_RAD);
			bqcrc	= (1.0 - 2.25 * satk3 * satk3) *
				  (1.0 - 2.25 * sbeta3 * sbeta3);

			if (bqcrc == 0.0)
				bqcrc = 0.0001;

			qcrc	= (qcr / bqcrc) - (*pcorQCR)(qcr);
			break;
		}

	if (qcrc < 11.0)
		qcrc = qcr;

	PutSample(varp, qcrc);

}	/* END SQCRC */

/* END QCRC.C */
