/*
-------------------------------------------------------------------------
OBJECT NAME:	filter.c

FULL NAME:	

ENTRY POINTS:	InitMRFilters()
				ClearMRFilters()
				Filter()

STATIC FNS:		See below.

DESCRIPTION:	The mrf filter has it's own circular buffer routines
				which are included in this module and are not associated
				with circbuff.[ch].  filters.c contains the coefficients.

INPUT:		

OUTPUT:		

REFERENCES:		none

REFERENCED BY:	interact.c mrf.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "decode.h"
#include "filters.h"
#include "circbuff.h"


static mRFilterPtr	sdiFilters[MAX_SDI][MAX_STAGES+1],
					rawFilters[MAX_RAW][MAX_STAGES+1];

static mRFilterPtr	createMRFilter();
static circBuffPtr	newCircBuff();
static void			initCircBuff(), disposCircBuff(), putBuff();
static NR_TYPE		getBuff();
static void			initMultiRateFilter();
static int			disposMultiRateFilter();
static int			iterateMRFilter();

static bool	PRINT = FALSE;

/* -------------------------------------------------------------------- */
void InitMRFilters()
{
	int			i;
	MOD			*mv_p;


	for (i = 0; i < FiveToOneUp.order; ++i)
		FiveToOneUp.aCoef[i] *= 5.0;

	for (i = 0; i < TwoToOneUp.order; ++i)
		TwoToOneUp.aCoef[i] *= 2.0;


	/* Create filter Data for each variable
	 */
	for (i = 0; i < nsdi; ++i)
		{
		if (sdi[i]->DependedUpon == FALSE && sdi[i]->OutputRate != HIGH_RATE)
			{
			sdiFilters[i][0] = NULL;
			continue;
			}

		mv_p = sdi[i]->Modulo;

		switch (sdi[i]->SampleRate)
			{
			case 1:			/* Interpolate	*/
				sdiFilters[i][0] = createMRFilter(5, 1, &FiveToOneUp, mv_p);
				sdiFilters[i][1] = createMRFilter(5, 1, &FiveToOneUp, mv_p);
				sdiFilters[i][2] = NULL;
				break;

			case 5:
				sdiFilters[i][0] = createMRFilter(5, 1, &FiveToOneUp, mv_p);
				sdiFilters[i][1] = NULL;
				break;

			case 10:
				sdiFilters[i][0] = createMRFilter(5, 2, &FiveToOneUp, mv_p);
				sdiFilters[i][1] = NULL;
				break;

			case 25:
				sdiFilters[i][0] = NULL;
				break;

			case 50:		/* Decimate	*/
				sdiFilters[i][0] = createMRFilter(1, 2, &TwoToOneDown, mv_p);
				sdiFilters[i][1] = NULL;
				break;

			case 125:
				sdiFilters[i][0] = createMRFilter(1, 5, &FiveToOneDown, mv_p);
				sdiFilters[i][1] = NULL;
				break;

			case 250:
				sdiFilters[i][0] = createMRFilter(1, 5, &FiveToOneDown, mv_p);
				sdiFilters[i][1] = createMRFilter(1, 2, &TwoToOneDown, mv_p);
				sdiFilters[i][2] = NULL;

				break;

			default:
				sdiFilters[i][0] = NULL;
				fprintf(stderr, "mrfFilter: non-supported input rate, ");
				fprintf(stderr, "var = %s, rate = %d\n",
							sdi[i]->name, sdi[i]->SampleRate);
			}
		}


	for (i = 0; i < nraw; ++i)
		{
		if (raw[i]->DependedUpon == FALSE && raw[i]->OutputRate != HIGH_RATE)
			{
			rawFilters[i][0] = NULL;
			continue;
			}

		/* Can't filter Vectors.  And we don't want to filter PMS1D stuff.
		 */
		if (raw[i]->Length > 1 || raw[i]->ProbeType & PROBE_PMS1D)
			{
			rawFilters[i][0] = NULL;
			continue;
			}

		mv_p = raw[i]->Modulo;

		switch (raw[i]->SampleRate)
			{
			case 1:			/* Interpolate	*/
				rawFilters[i][0] = createMRFilter(5, 1, &FiveToOneUp, mv_p);
				rawFilters[i][1] = createMRFilter(5, 1, &FiveToOneUp, mv_p);
				rawFilters[i][2] = NULL;
				break;

			case 5:
				rawFilters[i][0] = createMRFilter(5, 1, &FiveToOneUp, mv_p);
				rawFilters[i][1] = NULL;
				break;

			case 10:
				rawFilters[i][0] = createMRFilter(5, 2, &FiveToOneUp, mv_p);
				rawFilters[i][1] = NULL;
				break;

			case 25:
				rawFilters[i][0] = NULL;
				break;

			case 50:		/* Decimate	*/
				rawFilters[i][0] = createMRFilter(1, 2, &TwoToOneDown, mv_p);
				rawFilters[i][1] = NULL;
				break;

			case 125:
				rawFilters[i][0] = createMRFilter(1, 5, &FiveToOneDown, mv_p);
				rawFilters[i][1] = NULL;
				break;

			case 250:
				rawFilters[i][0] = createMRFilter(1, 5, &FiveToOneDown, mv_p);
				rawFilters[i][1] = createMRFilter(1, 2, &TwoToOneDown, mv_p);
				rawFilters[i][2] = NULL;
				break;

			default:
				rawFilters[i][0] = NULL;
				fprintf(stderr, "mrfFilter: non-supported input rate, ");
				fprintf(stderr, "var = %s, rate = %d\n",
							raw[i]->name, raw[i]->SampleRate);
			}
		}

}	/* END INITMRFILTER */

/* -------------------------------------------------------------------- */
void Filter(CircularBuffer *PSCB)
{
	int			i, OUTindex;
	int			task, currentHz, HzDelay, PSCBindex;
	SDITBL		*sp;
	RAWTBL		*rp;
	NR_TYPE		*input_record;
	NR_TYPE		output;
	mRFilterPtr	thisMRF;

	extern NR_TYPE	*SampledData, *HighRateData;

	SampledData = (NR_TYPE *)GetBuffer(PSCB, -(PSCB->nbuffers - 1));

	for (i = 0; (sp = sdi[i]); ++i)
		{
		thisMRF		= sdiFilters[i][0];
		PSCBindex	= -(PSCB->nbuffers - 1);
		HzDelay		= 0;


		/* SampleRate == ProcessingRate, no filtering
		 */
		if (thisMRF == NULL)
			{
			memcpy(	(char *)&HighRateData[sp->HRstart],
					(char *)&SampledData[sp->SRstart],
					NR_SIZE * HIGH_RATE);

			continue;
			}


		/* Counters don't need to be filtered.
		 */
		if (sp->type == 'C')
			{
			NR_TYPE	*sdp = &SampledData[sp->SRstart];

			switch (sp->SampleRate)
				{
				case 50:
					for (OUTindex = 0; OUTindex < ProcessingRate; ++OUTindex)
						HighRateData[sp->HRstart + OUTindex] =
								sdp[OUTindex * 2] + sdp[OUTindex * 2 + 1];

					break;

				case 5:
					for (OUTindex = 0; OUTindex < ProcessingRate; ++OUTindex)
						HighRateData[sp->HRstart+OUTindex] = sdp[OUTindex / 5] / 5;

					break;

				case 1:
					for (OUTindex = 0; OUTindex < ProcessingRate; ++OUTindex)
						HighRateData[sp->HRstart+OUTindex] = sdp[0] / 25;

					break;

				case 250:
					for (OUTindex = 0; OUTindex < ProcessingRate; ++OUTindex)
						HighRateData[sp->HRstart + OUTindex] =
								sdp[OUTindex * 2] + sdp[OUTindex * 2 + 1];
								sdp[OUTindex * 2 + 2] + sdp[OUTindex * 2 + 3];
								sdp[OUTindex * 2 + 4];

					break;

				default:
					fprintf(stderr, "Rate [%d] not supported for Counters, exiting.\n", sp->SampleRate);
					exit(1);
				}

			continue;
			}


		switch (sp->SampleRate)
			{
			case 1:
				PSCBindex += 12;
				break;

			case 5:
				PSCBindex += 2;
				break;

			case 10:
				PSCBindex += 1;
				break;

			case 50:
				PSCBindex += 1;
				HzDelay = 48;
				break;

			case 250:
				PSCBindex += 2;
				HzDelay = 90;
				break;
			}

		input_record = (NR_TYPE *)GetBuffer(PSCB, PSCBindex);

		if (thisMRF->task == GET_INPUT)
			{
			currentHz = HzDelay;
			++PSCBindex;
			}
		else
			if ((currentHz = sp->SampleRate - 1 + HzDelay) >= sp->SampleRate)
				{
				++PSCBindex;
				currentHz -= sp->SampleRate;
				}


		/* One stage filtering
		 */
		if (sdiFilters[i][0] && !sdiFilters[i][1])
			{
			for (OUTindex = 0; OUTindex < ProcessingRate; ++OUTindex)
				{
				do
					{
					task = iterateMRFilter(thisMRF,
						input_record[sp->SRstart + currentHz], &output);

					if (task == GET_INPUT && ++currentHz == sp->SampleRate)
						{
						currentHz = 0;
						input_record = (NR_TYPE *)GetBuffer(PSCB, PSCBindex++);
						}
					}
				while (task == GET_INPUT);

				HighRateData[sp->HRstart + OUTindex] = output;
				}

			continue;
			}


		/* Two stage filtering
		 */
		if (sdiFilters[i][0] && sdiFilters[i][1])
			{
			for (OUTindex = 0; OUTindex < ProcessingRate; ++OUTindex)
				{
				do
					{
					if (sdiFilters[i][1]->task == GET_INPUT)
						{
						do
							{
							task = iterateMRFilter(sdiFilters[i][0],
								input_record[sp->SRstart + currentHz], &output);

							if (task == GET_INPUT &&
								++currentHz == sp->SampleRate)
								{
								currentHz = 0;
								input_record = (NR_TYPE *)GetBuffer(PSCB, PSCBindex++);
								}
							}
						while (task == GET_INPUT);
						}

					task = iterateMRFilter(sdiFilters[i][1], output, &output);
					}
				while (task == GET_INPUT);

				HighRateData[sp->HRstart + OUTindex] = output;
				}

			continue;
			}
		}


	/* Do raw variables
	 */
	for (i = 0; (rp = raw[i]); ++i)
		{
		thisMRF		= rawFilters[i][0];
		PSCBindex	= -(PSCB->nbuffers - 1);
		HzDelay		= 0;


		/* SampleRate == ProcessingRate, no filtering
		 */
		if (thisMRF == NULL)
			{
			memcpy(	(char *)&HighRateData[rp->HRstart],
					(char *)&SampledData[rp->SRstart],
					NR_SIZE * rp->SampleRate * rp->Length);

			continue;
			}


		/* We don't support filtering vector data.
		 */
		if (rp->Length > 1)
			continue;


		switch (rp->SampleRate)
			{
			case 1:
				PSCBindex += 12;
				break;

			case 5:
				PSCBindex += 2;
				break;

			case 10:
				PSCBindex += 1;
				break;

			case 50:
				PSCBindex += 1;
				HzDelay = 48;
				break;

			case 250:
				break;
			}

		input_record = (NR_TYPE *)GetBuffer(PSCB, PSCBindex);

		if (thisMRF->task == GET_INPUT)
			{
			currentHz = HzDelay;
			++PSCBindex;
			}
		else
			if ((currentHz = rp->SampleRate - 1 + HzDelay) >= rp->SampleRate)
				{
				++PSCBindex;
				currentHz -= rp->SampleRate;
				}


		/* One stage filtering
		 */
		if (rawFilters[i][0] && !rawFilters[i][1])
			{
			for (OUTindex = 0; OUTindex < ProcessingRate; ++OUTindex)
				{
				do
					{
					task = iterateMRFilter(thisMRF,
						input_record[rp->SRstart + currentHz], &output);

					if (task == GET_INPUT && ++currentHz == rp->SampleRate)
						{
						currentHz = 0;
						input_record = (NR_TYPE *)GetBuffer(PSCB, PSCBindex++);
						}
					}
				while (task == GET_INPUT);

				HighRateData[rp->HRstart + OUTindex] = output;
				}

			continue;
			}


		/* Two stage filtering
		 */
		if (rawFilters[i][0] && rawFilters[i][1])
			{
			for (OUTindex = 0; OUTindex < ProcessingRate; ++OUTindex)
				{
				do
					{
					if (rawFilters[i][1]->task == GET_INPUT)
						{
						do
							{
							task = iterateMRFilter(rawFilters[i][0],
								input_record[rp->SRstart + currentHz], &output);

							if (task == GET_INPUT &&
								++currentHz == rp->SampleRate)
								{
								currentHz = 0;
								input_record = (NR_TYPE *)GetBuffer(PSCB, PSCBindex++);
								}
							}
						while (task == GET_INPUT);
						}

					task = iterateMRFilter(rawFilters[i][1], output, &output);
					}
				while (task == GET_INPUT);

				HighRateData[rp->HRstart + OUTindex] = output;
				}

			continue;
			}
		}

}	/* END FILTER */

/* -------------------------------------------------------------------- */
void ClearMRFilters()
{
	int		i, j;

	for (i = 0; i < nsdi; ++i)
		for (j = 0; sdiFilters[i][j]; ++j)
			if (sdiFilters[i][j])
				initMultiRateFilter(sdiFilters[i][j]);

	for (i = 0; i < nraw; ++i)
		for (j = 0; rawFilters[i][j]; ++j)
			if (rawFilters[i][j])
				initMultiRateFilter(rawFilters[i][j]);

}	/* END CLEARMRFILTER */

/* -------------------------------------------------------------------- */
static mRFilterPtr createMRFilter(L, M, filter, modvar)
int			L;
int			M;
filterPtr	filter;
MOD			*modvar;
{
	mRFilterPtr	aMRFPtr;

	aMRFPtr			= (mRFilterPtr)GetMemory(sizeof(mRFilterData));
	aMRFPtr->inBuff	= newCircBuff(filter->order / L);
	aMRFPtr->filter	= filter;
	aMRFPtr->L		= L;
	aMRFPtr->M		= M;
	aMRFPtr->modulo = modvar;
		
	return(aMRFPtr);

}	/* END CREATEMRFILTER */



/* -------------------------------------------------------------------- * 
 *
 * Everything below here was written by Charlie Robinson, and adapted
 * to our needs by me (Chris Webster).
 *
 * -------------------------------------------------------------------- */


/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*\

	FILTERS.C                CQRobinson 7Jun91

	Source library for circular buffer and filter functions.

    Data structures are defined in FILTERS.H.

\*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*=============== Circular Buffer Routines ==================================
-   The following routines provide easy interfaces for accesing
-		LIFO circular buffers.  Circular buffers are useful for a
-		variety of DSP applications, particularly FIR filters.
-   The following routines are included:
-       newCircBuff: allocates and inits memory for circular buffer struct.
-       disposCircBuff: deallocates structure memory.
-       putBuff: add an item to a circular buffer.
-       getBuff: retrieve an item from a circular buffer.
*/

/*---------------------------------------------------------------------------
-	circBuffPtr newCircBuff(int size)
-		Allocate new circular buffer of size and type specified.
*/
static circBuffPtr newCircBuff(size)
int	size;
{
	circBuffPtr 	aCBPtr;

	/* Check size  */
	if (size < 0)
		{
		puts("newCircBuff: parameter 'size' < 0");
		exit(0);
		}
	else if (size > MAX_BUFF_SIZE)
		{
		puts("newCircBuff: parameter 'size' too large.");
		exit(0);
		}

	/* Allocate memory for structure.		*/
	aCBPtr = (circBuffPtr)GetMemory(sizeof(circBuff));

	/* Allocate memory for data buffer.	*/
	aCBPtr->value = (NR_TYPE *)GetMemory(sizeof(NR_TYPE) * size);
	aCBPtr->size  = size;

	return(aCBPtr);
}

/*---------------------------------------------------------------------------
-	circBuffPtr initCircBuff(circBuffPtr aCBPtr, int size)
-		Allocate new circular buffer of size and type specified.
*/
static void initCircBuff(aCBPtr)
circBuffPtr	aCBPtr;
{
	NR_TYPE		*buffer;
	int			i;

	buffer = aCBPtr->value;

	for (i = 0; i < aCBPtr->size; i++)
		buffer[i] = 0.0;

	aCBPtr->index = 0;

}

/*--------------------------------------------------------------
-	void disposCircBuff(circBuffPtr aCBPtr)
-		Free memory of circular buffer specified.
*/
static void disposCircBuff(aCBPtr)
circBuffPtr	aCBPtr;
{
	free((char *)aCBPtr->value);
	free((char *)aCBPtr);
}

/*------------------------------------------------------------
-	void putBuff(datum, aCBPtr)
-		Put datum into aCBPtr buffer.
*/
static void putBuff(datum, aCBPtr)
NR_TYPE		datum;
circBuffPtr	aCBPtr;
{
	/* Update index, wrap if necessary.	*/
	aCBPtr->index++;
	if ((aCBPtr->index >= aCBPtr->size) || (aCBPtr->index < 0))
		aCBPtr->index = 0;

	/*  Store value.                        */
	aCBPtr->value[aCBPtr->index] = datum;
if (PRINT) printf("%f\n", datum);
}

/*------------------------------------------------------------------*/
static NR_TYPE getBuff(offset, aCBPtr)
int			offset;
circBuffPtr	aCBPtr;
/*
-	Return datum at offset 'offset' from aCBPtr buffer.
-	Note: getBuff does not remove items from buffer.
*/
{
	/* Calculate index from offset and index.	  */
	offset = (aCBPtr->index - offset) % aCBPtr->size;

	while (offset < 0)
		offset += aCBPtr->size;

	return(aCBPtr->value[offset]);
}

/*======================= Multi Rate Filter Routines =====================
-   The following routines implement a multi rate filter for
-       changing the sample rate of an input sample stream.
-   The process accepts an input stream, interpolates by L, filters,
-       then decimates by M.
-   The routines are implemented such that the interpolation and
-       decimation are incorperated into the filter so that multiplies
-       by zero (from interpolation) are avoided, and samples to be
-       thrown out (during decimation) are not computed.
-   The following routines are included:
-       initMultiRateFilter - To be called before any samples are computed.
-       iterateMultiRateFilter - To be called as samples are available for
-           input or output.
-       disposMultiRateFilter - Used to free up allocated memory.
-
-   For a copmlete discussion of multirate filtering techniques see
-       Crochiere, Ronald E. and Lawrence R. Rabiner. "Multirate
-       Digital Signal Processing", Prentice-Hall 1983.
*/

/*--------------------------------------------------------------------------
-   initMultiRateFilter
-       initialize MultiFilt Struct with filter data and instance vars.
*/
static void initMultiRateFilter(aMRFPtr)
mRFilterPtr	aMRFPtr;
{
	initCircBuff(aMRFPtr->inBuff);
	aMRFPtr->inTime		= 0;	/* Do not return for input on first iteration */
	aMRFPtr->outTime	= aMRFPtr->M; /* Return with output on first iteration*/
	aMRFPtr->coefPhase	= 0;		/* Start with zero phase offset.	*/
	aMRFPtr->task	= GET_INPUT;	/* Start by collecting input.		*/

}

/*--------------------------------------------------------------------------
-   disposMultiRateFilter
-       delete MultiFilt Struct with filter data and instance vars.
*/
static int disposMultiRateFilter(aMRFPtr)
mRFilterPtr	aMRFPtr;
{
	if (aMRFPtr == NULL)
		return(FALSE);

	disposCircBuff(aMRFPtr->inBuff);
	aMRFPtr->inBuff = NULL;
	free((char *)aMRFPtr);

	return(TRUE);
}

/*--------------------------------------------------------------------------
-   iterateMultiRateFilter
-       The multi rate filter collects input and generates output
-       at different rates.  Each call exacutes the filter algorithm
-       untill an input is required or an output is available.  The
-       return value specifies which condition is in effect.
*/
static int iterateMRFilter(thisMRF, input, output)
mRFilterPtr	thisMRF;	/* The filter object to be iterated.	*/
NR_TYPE		input;		/* A new input sample, as requested.	*/
NR_TYPE		*output;	/* Pointer to new output, if available.	*/
{
	int			tap, i;	/* Indecies for coef and input data arrays.	*/
	filterType	result;	/* Temporary accumulator for output calc.	*/

	/* Input was just requested; here it is.	*/
	if (thisMRF->task == GET_INPUT)
		{
		putBuff(input, thisMRF->inBuff);
		thisMRF->coefPhase = 0;
		}


	/* Execute this loop until an input sample is needed,
	 * or an output sample is available.
	 */
	while (TRUE)
		{
		/* Time to get new input sample?   */
		if (thisMRF->inTime >= thisMRF->L)
			{
			thisMRF->inTime = 0;
			return(thisMRF->task = GET_INPUT);
			}

		/* Time to calc output sample? 	*/
		if (thisMRF->outTime >= thisMRF->M)
			{
			thisMRF->outTime = 0;
			tap = thisMRF->coefPhase;

			/* Filter. */
			result = 0.0;

			if (thisMRF->modulo)
				{
				NR_TYPE	value;
				bool	low_value = FALSE,
						high_value = FALSE;

				for(i = 0; tap < thisMRF->filter->order; tap += thisMRF->L, i++)
					{
					value = getBuff(i, thisMRF->inBuff);

					if (value > thisMRF->modulo->bound[1])
						high_value = TRUE;

					if (value < thisMRF->modulo->bound[0])
						low_value = TRUE;
					}

				if (low_value && high_value)
					{
					for (i = 0; tap < thisMRF->filter->order;
														tap += thisMRF->L, i++)
						{
						if ((value = getBuff(i, thisMRF->inBuff))
											< thisMRF->modulo->bound[0])
							value +=	thisMRF->modulo->diff;

						result += thisMRF->filter->aCoef[tap] * value;
						}
					}
				}
			else
				for(i = 0; tap < thisMRF->filter->order; tap += thisMRF->L, i++)
					result += thisMRF->filter->aCoef[tap] *
										getBuff(i, thisMRF->inBuff);

			*output = result;

			/*  Advance time.   */
			thisMRF->inTime++;
			thisMRF->outTime++;
			thisMRF->coefPhase++;

			return(thisMRF->task = OUT_AVAIL);
			}

		/*  Advance time.   */
		thisMRF->inTime++;
		thisMRF->outTime++;
		thisMRF->coefPhase++;
		}

}

/* END FILTER.C */
