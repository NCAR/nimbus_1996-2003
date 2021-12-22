/*
-------------------------------------------------------------------------
OBJECT NAME:	rtloop.c

FULL NAME:		RealTime Loop

ENTRY POINTS:	RealTimeLoop()

STATIC FNS:		WaitForNextLogicalRecord()
				OpenBroadcastSocket()

DESCRIPTION:	

REFERENCES:		none

REFERENCED BY:	rtplot.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1997
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ctape.h"

#include <Xm/Xm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "/home/local/winds/include/socket.h"

extern NR_TYPE	*SampledData, *AveragedData;
extern char		*ADSrecord;

extern XtAppContext    app_con;

static int WaitForNextLogicalRecord(char dest[], int len);
static void OpenBroadcastSocket(int sockNum);
static void ApplyVolts(NR_TYPE *SampledData);


/* -------------------------------------------------------------------- */
void RealTimeLoop()
{
  int	nbytes;
  long	lrlen;

  get_lrlen(&lrlen);

  OpenBroadcastSocket((FIRST_AVAILABLE_PORT+CALIB_NETDATA));

  while ((nbytes = WaitForNextLogicalRecord(ADSrecord, lrlen)) > 0)
    {
    DecodeADSrecord((short *)ADSrecord, SampledData);

    if (Units == ENGINEERING)
      ApplyCalCoes(SampledData, SAMPLE_RATE);
    else
    if (Units == VOLTS)
      ApplyVolts(SampledData);

 
    AddDataToBuffer(SampledData);

    AutoScale(SCALE_YAXIS);

    if (!Frozen)
      if (!FlashMode || Variable[0].buffIndex == 0)
        PlotData(NULL, NULL, NULL);

    while (XtAppPending(app_con))
      XtAppProcessEvent(app_con, XtIMAll);
    }

}	/* END REALTIMELOOP */

/* -------------------------------------------------------------------- */

static int	udpSocket;

/* -------------------------------------------------------------------- */
static int WaitForNextLogicalRecord(char dest[], int len)
{
  int	rlen;
  Hdr_blk *hdr = (Hdr_blk *)dest;

  do
    {
    if ((rlen = recvfrom(udpSocket, dest, len, 0, NULL, NULL)) == ERROR)
      perror("UdpSocket: read");

    while (XtAppPending(app_con))
      XtAppProcessEvent(app_con, XtIMAll);
    }
  while (*((ushort *)dest) != 0x8681);

  sprintf(xlabel, "UTC (%02d:%02d:%02d)", hdr->hour, hdr->minute, hdr->sec);

  return(rlen);

}	/* END WAITFORNEXTLOGICALRECORD */

/* -------------------------------------------------------------------- */
static void OpenBroadcastSocket(int sockNum)	/* Only reading required	*/
{
  int	n, opt = 1;
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

/* -------------------------------------------------------------------- */
static void ApplyVolts(NR_TYPE *record)
{
  int     i, j, pos;
  SDITBL  *sp;

  for (i = 0; (sp = sdi[i]); ++i)
    {
    pos = sp->SRstart;

    for (j = 0; j < sp->SampleRate; ++j, ++pos)
      record[pos] = (record[pos] - sp->convertOffset) * sp->convertFactor;
    }

}	/* END APPLYVOLTS */

/* END RTLOOP.C */
