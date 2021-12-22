/*
-------------------------------------------------------------------------
OBJECT NAME:	lrloop.c

FULL NAME:	Main loop for low rate processing

ENTRY POINTS:	LowRateLoop()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		long beginning and ending times

OUTPUT:		

REFERENCES:	circbuff.c, adsIO.c, rec_decode.c, phase_shift.c average.c
		compute.c netcdf.c, timeseg.c

REFERENCED BY:	StartProcessing()

NOTE:		Changes here may also be required in hrloop.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "decode.h"
#include "gui.h"
#include "circbuff.h"

#define NBUFFERS	5
#define INDEX		-(NBUFFERS-2)

extern char		*ADSrecord;
extern NR_TYPE		*SampledData, *AveragedData;
extern XtAppContext	context;


/* -------------------------------------------------------------------- */
LowRateLoop(long starttime, long endtime)
{
  int			i;
  long			nBytes;
  NR_TYPE		*BuffPtr;
  CircularBuffer	*LRCB;	/* Logical Record Circular Buffers	*/


  /* Account for Circular Buffer slop	*/
  if (starttime != BEG_OF_TAPE)
    starttime -= 2;

  if (endtime != END_OF_TAPE)
    endtime += -(INDEX+1);

  nBytes = nFloats * NR_SIZE;
  if ((LRCB = CreateCircularBuffer(NBUFFERS, nBytes)) == NULL)
    {
    nBytes = ERR;
    goto exit;
    }


  /* Perform initialization before entering main loop.
   */
  if ((nBytes = FindFirstLogicalRecord(ADSrecord, starttime)) <= 0)
    goto exit;

nBytes = FindNextLogicalRecord(ADSrecord, endtime);
nBytes = FindNextLogicalRecord(ADSrecord, endtime);

  SetBaseTime(ADSrecord);		/* See netcdf.c	*/


  /* Fill circular Buffers
   */
  BuffPtr = (NR_TYPE *)AddToCircularBuffer(LRCB);
  DecodeADSrecord((short *)ADSrecord, BuffPtr);
  ApplyCalCoes(BuffPtr);

  for (i = 1; i < NBUFFERS-1; ++i)
    {
    if ((nBytes = FindNextLogicalRecord(ADSrecord, endtime)) <= 0)
      goto exit;

    if (CheckForTimeGap(ADSrecord, False) == GAP_FOUND)
      goto exit;

    BuffPtr = (NR_TYPE *)AddToCircularBuffer(LRCB);
    DecodeADSrecord((short *)ADSrecord, BuffPtr);
    ApplyCalCoes(BuffPtr);
    }


  /* This is the main loop.
   */
  while ((nBytes = FindNextLogicalRecord(ADSrecord, endtime)) > 0)
    {
    if (CheckForTimeGap(ADSrecord, False) == GAP_FOUND)
      break;

    BuffPtr = (NR_TYPE *)AddToCircularBuffer(LRCB);
    DecodeADSrecord((short *)ADSrecord, BuffPtr);
    ApplyCalCoes(BuffPtr);

    DespikeData(LRCB, INDEX+1);
    PhaseShift(LRCB, INDEX, SampledData);

    AverageSampledData();
    ComputeLowRateDerived();

    WriteNetCDF();
    UpdateTime(SampledData);

    while (PauseFlag == True)
      XtAppProcessEvent(context, XtIMAll);

    if (PauseWhatToDo == P_QUIT) {
      nBytes = ERR;
      break;
      }
    }


exit:
  ReleaseCircularBuffer(LRCB);

  return(nBytes);

}	/* END LOWRATELOOP */

/* END LRLOOP.C */
