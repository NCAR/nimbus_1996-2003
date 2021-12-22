/*
-------------------------------------------------------------------------
OBJECT NAME:	amlib.h

DESCRIPTION:	Header File declaring amlib functions.
-------------------------------------------------------------------------
*/

#ifndef AMLIB_H
#define AMLIB_H

#include "header.h"
#include <math.h>
#include <netinet/in.h>

/* Values for 'FeedBack' variable	*/
#define	nFeedBackTypes		2

#define LOW_RATE_FEEDBACK	0
#define HIGH_RATE_FEEDBACK	1

#define AMBIENT(t, rf, xm2)	((t + 273.16) / (1.0 + 0.2 * rf * xm2) - 273.16)

#define GetSample(dp, di)	\
	(FeedBack == LOW_RATE_FEEDBACK \
		? AveragedData[dp->depend_LRindex[di]] \
		: HighRateData[dp->depend_HRindex[di] + SampleOffset])

#define GetVector(dp, di, vlen)	\
	(FeedBack == LOW_RATE_FEEDBACK \
		? &AveragedData[dp->depend_LRindex[di]] \
		: &HighRateData[dp->depend_HRindex[di] + (SampleOffset * vlen)])

#define GetSampleFor1D(dp, di)	\
	(FeedBack == LOW_RATE_FEEDBACK \
		? AveragedData[dp->depend_LRindex[di]] \
		: HighRateData[dp->depend_HRindex[di] + (SampleOffset << 1)])


#define PutSample(dp, y)	\
	if (FeedBack == LOW_RATE_FEEDBACK) \
		AveragedData[dp->LRstart] = y; \
	else \
		HighRateData[dp->HRstart + SampleOffset] = y

#define PutStaticSample(dp, y)	\
	if (FeedBack == LOW_RATE_FEEDBACK) \
		AveragedData[dp->LRstart] = y[HIGH_RATE]; \
	else \
		HighRateData[dp->HRstart + SampleOffset] = y[SampleOffset]

struct _dnfn
	{
	char		*name;
	void		(*constructor)(void *);
	void		(*xlate)(RAWTBL *, void *, float *);
	void		(*compute)(DERTBL *);
	} ;

inline NR_TYPE	FirstPoly(NR_TYPE x, NR_TYPE c[]);
inline NR_TYPE	SecondPoly(NR_TYPE x, NR_TYPE c[]);


int	SearchDERIVFTNS();
void	RunAMLIBinitializers(), SetLookupSuffix();

extern NR_TYPE		*AveragedData, *HighRateData;
extern int		FeedBack, SampleOffset;
extern struct _dnfn	deriveftns[];

extern float	HDRversion;

extern const NR_TYPE	FTMTR, MPS2, KTS2MS, FTMIN, RESOLV14BIT, RESOLV16BIT;
extern const double	RAD_DEG, DEG_RAD;

NR_TYPE *GetDefaultsValue(char target[], char var[]);

#if defined(__LITTLE_ENDIAN) || defined(_LITTLE_ENDIAN) || defined(LITTLE_ENDIAN)
float	ntohf(float);
#else
#define ntohf(x)        (x)
#endif

#include "amlibProto.h"

#endif

/* END AMLIB.H */
