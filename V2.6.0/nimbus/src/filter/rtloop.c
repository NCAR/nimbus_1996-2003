/*
-------------------------------------------------------------------------
OBJECT NAME:    rtloop.c

FULL NAME:      RealTime Loop

ENTRY POINTS:   RealTimeLoop()

STATIC FNS:     WaitForNextLogicalRecord()
                OpenBroadcastSocket()

DESCRIPTION:

REFERENCES:     none

REFERENCED BY:  rtplot.c

COPYRIGHT:      University Corporation for Atmospheric Research, 1997
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "decode.h"
#include "ctape.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "/usr/local/winds/include/socket.h"

extern NR_TYPE	*SampledData, *AveragedData;
extern char		*ADSrecord;
extern bool		Interactive;

static int WaitForNextLogicalRecord(char dest[], int len);
static void OpenBroadcastSocket(int sockNum);


/* -------------------------------------------------------------------- */
void RealTimeLoop()
{
  int	nbytes;
  long	dummy[2], lrlen;

printf("\nnimbus in RealTime mode, output file is /home/tmp/RealTime.cdf\n");

  get_lrlen(&lrlen);

  /* wait for record and write base_time to netCDF file.
   */
  OpenBroadcastSocket((FIRST_AVAILABLE_PORT+CALIB_NETDATA));
  nbytes = WaitForNextLogicalRecord(ADSrecord, lrlen);
  SetBaseTime(ADSrecord);

  GetUserTimeIntervals();
  NextTimeInterval(&dummy[0], &dummy[1]);

  while ((nbytes = WaitForNextLogicalRecord(ADSrecord, lrlen)) > 0)
    {
/*
    if (CheckForTimeGap(ADSrecord, FALSE) == GAP_FOUND) {
      fprintf(stderr, "No data received in over 90 seconds, exiting.\n");
      break;
      }
*/
    /* We could fork a thread to do the processing.....
     */
    DecodeADSrecord((short *)ADSrecord, SampledData);
    ApplyCalCoes(SampledData);
 
    AverageSampledData();
    ComputeLowRateDerived();
 
    WriteNetCDF();
    UpdateTime(SampledData);
    SyncNetCDF();
    }

}	/* END REALTIMELOOP */

/* -------------------------------------------------------------------- */

static int	udpSocket;

/* -------------------------------------------------------------------- */
static int WaitForNextLogicalRecord(char dest[], int len)
{
  int rlen;

Hdr_blk *hdr = (Hdr_blk *)dest;

  do
    {
    if ((rlen = recvfrom(udpSocket, dest, len, 0, NULL, NULL)) == ERROR)
      perror("RealTimeLoop: recvfrom");
    }
  while (*((ushort *)dest) != 0x8681);

printf("%02d:%02d:%02d - %d\r", hdr->hour, hdr->minute, hdr->second, rlen);

  return(rlen);

}	/* END WAITFORNEXTLOGICALRECORD */

/* -------------------------------------------------------------------- */
static void OpenBroadcastSocket(int sockNum)	/* Only reading required	*/
{
  int	opt = 1;
  struct sockaddr_in sock_name;

// Open the socket.
  if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("UdpSocket: socket");
    exit(ERROR);
    }

// Allow previously used addresses to be reused.
  if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt,
                  sizeof(opt)) == ERROR) {
    perror("UdpSocket: setsockopt SO_REUSEADDR");
    exit(1);
    }

  sock_name.sin_family = AF_INET;
  sock_name.sin_port = htons(sockNum);
  sock_name.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(udpSocket, (struct sockaddr*)&sock_name, sizeof(sock_name))< 0) {
    perror("UdpSocket: bind");
    exit(1);
    }

  if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char*)&opt,
                 sizeof(opt)) == ERROR) {
    perror("UdpSocket: setsockopt SO_BROADCAST");
    exit(1);
    }

}	/* END OPENBROADCASTSOCKET */

/* END NIMBUS_RT */
