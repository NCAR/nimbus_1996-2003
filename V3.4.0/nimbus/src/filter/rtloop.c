/*
-------------------------------------------------------------------------
OBJECT NAME:    rtloop.c

FULL NAME:      RealTime Loop

ENTRY POINTS:   RealTimeLoop()

STATIC FNS:     none

DESCRIPTION:

REFERENCES:     none

REFERENCED BY:  rtplot.c

COPYRIGHT:      University Corporation for Atmospheric Research, 1997-2000
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "decode.h"

#include <errno.h>

extern NR_TYPE	*SampledData, *AveragedData;
extern char	*ADSrecord;


/* -------------------------------------------------------------------- */
void RealTimeLoop()
{
  int	nBytes;
  long	dummy[2];

  /* wait for record and write base_time to netCDF file.
   */
  nBytes = FindFirstLogicalRecord(ADSrecord, BEG_OF_TAPE);
  SetBaseTime(ADSrecord);

//  GetUserTimeIntervals();
//  NextTimeInterval(&dummy[0], &dummy[1]);

  while (1)
    {
    while ((nBytes = FindNextLogicalRecord(ADSrecord, END_OF_TAPE)) == 0)
      usleep(100000);

    if (nBytes < 0)
      {
      fprintf(stderr, "FindNextLogicalRecord: nBytes = %d, errno = %d\n",
		nBytes, errno);
      exit(1);
      }

    CheckForTimeGap(ADSrecord, FALSE);

    DecodeADSrecord((short *)ADSrecord, SampledData);
    ApplyCalCoes(SampledData);
 
    AverageSampledData();
    ComputeLowRateDerived();
 
    WriteNetCDF();
    UpdateTime(SampledData);
    SyncNetCDF();
    }

}	/* END REALTIMELOOP */

/* END RTLOOP.C */
