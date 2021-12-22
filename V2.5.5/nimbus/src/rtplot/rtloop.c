/*
-------------------------------------------------------------------------
OBJECT NAME:	rtloop.c

FULL NAME:	RealTime Loop

ENTRY POINTS:	RealTimeLoop()

STATIC FNS:	WaitForNextLogicalRecord()
		OpenSocket()

DESCRIPTION:	

REFERENCES:	none

REFERENCED BY:	rtplot.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1997
-------------------------------------------------------------------------
*/

#include "define.h"
#include "ctape.h"

#include <Xm/Xm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#include "/home/local/winds/include/socket.h"

int	Mode	= REALTIME;

extern XtAppContext    app_con;

static int WaitForNextLogicalRecord(char dest[], int len);
static void OpenSocket(int sockNum);


/* -------------------------------------------------------------------- */
void RealTimeLoop()
{
  int	nbytes, cnt = 0;
  long	lrlen;

  get_lrlen(&lrlen);

  OpenSocket(1106);

  while ((nbytes = WaitForNextLogicalRecord(ADSrecord, lrlen)) > 0)
    {
    DecodeRecord();
    ApplyCalCoes(SampledData);

    switch (Units)
      {
      case COUNTS:
        AddDataToBuffer(bits);
        break;

      case VOLTS:
        AddDataToBuffer(volts);
        break;

      case ENGINEERING:
        AddDataToBuffer(SampledData);
        break;

      }

    AutoScale(SCALE_YAXIS);

    if (!Frozen)
      if (!FlashMode || Variable[0].buffIndex == 0)
        PlotData(NULL, NULL, NULL);

    while (XtAppPending(app_con))
      XtAppProcessEvent(app_con, XtIMAll);
    }

}	/* END REALTIMELOOP */

/* -------------------------------------------------------------------- */

static int	sock;

/* -------------------------------------------------------------------- */
static int WaitForNextLogicalRecord(char dest[], int len)
{
  int	rlen;
  Hdr_blk *hdr = (Hdr_blk *)dest;

  do
    {
    if ((rlen = recv(sock, dest, 2, 0)) == ERROR)
      perror("WaitForNextLogicalRecord: read");

    while (XtAppPending(app_con))
      XtAppProcessEvent(app_con, XtIMAll);
    }
  while (*((ushort *)dest) != 0x8681);


  while (rlen < len)
    {
    if ((rlen += recv(sock, &dest[rlen], len-rlen, 0)) == ERROR)
      perror("WaitForNextLogicalRecord: read");

    while (XtAppPending(app_con))
      XtAppProcessEvent(app_con, XtIMAll);
    }

  sprintf(xlabel, "UTC (%02d:%02d:%02d)", hdr->hour, hdr->minute, hdr->second);

  return(rlen);

}	/* END WAITFORNEXTLOGICALRECORD */

/* -------------------------------------------------------------------- */
static void OpenSocket(int sockNum)	/* Only reading required	*/
{
  int			n, opt = 1, len;
  struct hostent	*hp;
  struct sockaddr_in	sock_name;


// Open the socket.
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("OpenSocket: socket");
    exit(ERROR);
    }

  sock_name.sin_family = AF_INET;
  sock_name.sin_port = htons(sockNum);


  if ((hp = gethostbyname(HostName)) == 0) {
    perror("OpenSocket: gethostbyname");
    exit(ERROR);
    }

  memcpy((char*)&sock_name.sin_addr.s_addr, (char*)hp->h_addr_list[0],
         hp->h_length);


  if (connect(sock, (struct sockaddr*)&sock_name, sizeof(sock_name)) < 0) {
    perror("OpenSocket: connect");
    exit(1);
    }

  opt = 65536;
  setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&opt, sizeof(int));
  getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&opt, &len);
  printf("sizeof RCVBUF = %d\n", opt);

}	/* END OPENSOCKET */

/* END RTLOOP.C */
