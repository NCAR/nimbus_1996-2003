/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*\

	FILTERS.H
	Data structures and prototypes for FILTERS.C.

	The keyword 'FILTER_WORDS'
	must be defined for all files that use these routines
	(including FILTER.C) prior to the inclusion of FILTER.H,
	e.g. as a compiler option.

	CQRobinson 7Jun91

\*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef FILTERS_H
#define FILTERS_H

#include <sys/types.h>

#define MAX_COEF	200
#define MAX_FILTERS	10
#define MAX_STAGES	2

typedef NR_TYPE	filterType;

typedef struct
	{
	short	size;
	short	index;
	NR_TYPE	*value;
	} circBuff, *circBuffPtr;

typedef struct
	{
	char		*name;
	int		order;
	filterType	aCoef[MAX_COEF];
	} filterData, *filterPtr;

typedef struct
	{
	circBuffPtr	inBuff;	/* In sample buffer.	*/
	filterPtr	filter;	/* Filter coeffs	*/
	ushort	L, M;		/* Interpolation and Decimation Factors	*/
	ushort	inTime;		/* Input and		*/
	ushort	outTime;	/*  output sample counters.	*/
	ushort	coefPhase;	/* Index for first coef of 'polyphase' filter */
	ushort	task;		/* Current filter activity	*/
	ushort	currentHz;	/* Index into current second	*/
	MOD	*modulo;	/* Is this variable circular (i.e. Heading) */
	} mRFilterData, *mRFilterPtr;


#define MAX_BUFF_SIZE	MAX_COEF

/*  iterateMRFilter() return values.  */
#define OUT_AVAIL   69
#define GET_INPUT   55


extern filterData	FiveToOneDown, TwoToOneDown, FiveToOneUp, TwoToOneUp,
			TenToOneDown;

#endif
