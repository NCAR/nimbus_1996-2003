/*
-------------------------------------------------------------------------
OBJECT NAME:	lrloop.c

FULL NAME:		Main loop for low rate processing

ENTRY POINTS:	LowRateLoop()

STATIC FNS:		none

DESCRIPTION:	

INPUT:			long beginning and ending times

OUTPUT:		

REFERENCES:		circbuff.c, adsIO.c, rec_decode.c, phase_shift.c average.c
				compute.c netcdf.c, timeseg.c

REFERENCED BY:	StartProcessing()

NOTE:			Changes here may also be required in hrloop.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "decode.h"
#include "gui.h"
#include "circbuff.h"

#define NBUFFERS	5
#define INDEX		-(NBUFFERS-2)

extern char		*ADSrecord;
extern NR_TYPE	*SampledData, *AveragedData;
extern XtAppContext context;


/* -------------------------------------------------------------------- */
LowRateLoop(long starttime, long endtime)
{
	int				i;
	long			nbytes;
	NR_TYPE			*BuffPtr;
	CircularBuffer	*LRCB;	/* Logical Record Circular Buffers	*/


	/* Account for Circular Buffer slop	*/
	if (starttime != BEG_OF_TAPE)
		starttime -= 2;

	if (endtime != END_OF_TAPE)
		endtime += -(INDEX+1);

	nbytes = nFloats * NR_SIZE;
	if ((LRCB = CreateCircularBuffer(NBUFFERS, nbytes)) == NULL)
		{
		nbytes = ERR;
		goto exit;
		}


	/* Perform initialization before entering main loop.
	 */
	if ((nbytes = FindFirstLogicalRecord(ADSrecord, starttime)) <= 0)
		goto exit;

	SetBaseTime(ADSrecord);		/* See netcdf.c	*/


	/* Fill circular Buffers
	 */
	BuffPtr = (NR_TYPE *)AddToCircularBuffer(LRCB);
	DecodeADSrecord((short *)ADSrecord, BuffPtr);
	ApplyCalCoes(BuffPtr, SAMPLE_RATE);

	for (i = 1; i < NBUFFERS-1; ++i)
		{
		if ((nbytes = FindNextLogicalRecord(ADSrecord, endtime)) <= 0)
			goto exit;

		if (CheckForTimeGap(ADSrecord, TRUE) == GAP_FOUND)
			goto exit;

		BuffPtr = (NR_TYPE *)AddToCircularBuffer(LRCB);
		DecodeADSrecord((short *)ADSrecord, BuffPtr);
		ApplyCalCoes(BuffPtr, SAMPLE_RATE);
		}

	/* This is the main control loop.
	 */
	while ((nbytes = FindNextLogicalRecord(ADSrecord, endtime)) > 0)
		{
		if (CheckForTimeGap(ADSrecord, FALSE) == GAP_FOUND)
			break;

		BuffPtr = (NR_TYPE *)AddToCircularBuffer(LRCB);
		DecodeADSrecord((short *)ADSrecord, BuffPtr);
		ApplyCalCoes(BuffPtr, SAMPLE_RATE);
		DespikeData(LRCB, INDEX+1);
		PhaseShift(LRCB, INDEX, SampledData);

		AverageSampledData();
		ComputeLowRateDerived();

		WriteNetCDF();
		UpdateTime(SampledData);

		while (PauseFlag == TRUE)
			XtAppProcessEvent(context, XtIMAll);

		if (PauseWhatToDo == P_QUIT)
			{
			nbytes = ERR;
			break;
			}
		}


exit:
	ReleaseCircularBuffer(LRCB);

	return(nbytes);

}	/* END LOWRATELOOP */

/* END LRLOOP.C */
