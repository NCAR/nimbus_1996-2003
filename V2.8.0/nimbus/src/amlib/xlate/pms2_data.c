/*
-------------------------------------------------------------------------
OBJECT NAME:	pms2_data.c

FULL NAME:	Translate PMS2D records to look like 1D histograms

ENTRY POINTS:	xlTwoD(), xlTwoDG()

STATIC FNS:	none

DESCRIPTION:	This sums up the data bins from SampleRate to 1hz.

INPUT:		

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	rec_decode.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1999-2000
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"
#include "queue.h"

static Queue	*probes[MAX_PMS2];

/* Data from the file is only marked as 'C1', 'C2', 'P1', 'P2', we'll
 * use this array to determine which records go with which Queue above.
 */
static short	probeIDorder[MAX_PMS2] = { 0, 0, 0, 0 };
static long	startTime[MAX_PMS2];
static short	startMilliSec[MAX_PMS2];
static float	overLoad[MAX_PMS2];
static float	twoD[MAX_PMS2][BINS_64];


struct particle
	{
	long	time;	/* Seconds since mid-night	*/
        long	msec;

	int	w, h;
	bool	edge;	/* particle touched edge	*/
	ulong	timeWord;
/*	ulong	*p; */
	};

typedef struct particle Particle;


static void AddMore2dData(Queue *, long, int);
void Process(Queue *, P2d_rec *, int probeCnt);

bool Next2dRecord(P2d_rec *, int, short);


/* -------------------------------------------------------------------- */
void Add2DtoList(RAWTBL *varp)	/* Called by hdr_decode.c */
{
  int	i, cCnt, pCnt, hCnt;
  static int piCnt = 0;

  probes[piCnt] = CreateQueue();
  
  probeIDorder[varp->ProbeCount] |= varp->name[3] << 8;

  ++piCnt;

printf("name=%s, cnt=%d\n", varp->name, varp->ProbeCount);

  /* Set up ordering and ID's so we now which 2D records go with which
   * probe index
   */
  cCnt = pCnt = hCnt = '1';
  for (i = 0; i < piCnt; ++i)
    {
    if ((probeIDorder[i] & 0x4300) == 0x4300)   /* 2D-C */
      probeIDorder[i] |= cCnt++;

    if ((probeIDorder[i] & 0x5000) == 0x5000)   /* 2D-P */
      probeIDorder[i] |= pCnt++;

    if ((probeIDorder[i] & 0x4800) == 0x4800)   /* HVPS */
      probeIDorder[i] |= hCnt++;

printf("%x\n", probeIDorder[i]);
    }

}	/* END ADD2DTOLIST */

/* -------------------------------------------------------------------- */
void xlTwodInit(RAWTBL *varp)
{
  int		probeCnt = varp->ProbeCount;
  P2d_rec	rec;


  /* Acquire time stamps from first record, and throw data away.
   */
  if (Next2dRecord(&rec, probeCnt, probeIDorder[probeCnt]))
    {
    startTime[probeCnt]	= rec.hour*3600+rec.minute*60+rec.second;
    startMilliSec[probeCnt] = rec.msec;
    overLoad[probeCnt]	= rec.overld;

    if (overLoad[probeCnt] > 40) /* MAP was screwed up.  37ms is max. */
      overLoad[probeCnt] = 37;

printf("  %x startTime = %d.%d\n", rec.id, startTime[probeCnt], startMilliSec[probeCnt]);
    }

  sTwodInit(varp);

}	/* END INITTWOD */

/* -------------------------------------------------------------------- */
void xlTwoD(RAWTBL *varp, void *in, NR_TYPE *np)
{
  int		i;
  int		probeCnt = varp->ProbeCount;

  /* Unfortunatly the ProbeCount for the 1D vs. 2D arrays/variables is not
   * the same.  xlOneD will be placing it into twoD of it's ProbeCount.
   * Compensate by subtracting 1 below
   */
  for (i = 0; i < varp->Length; ++i)
    np[i] = twoD[probeCnt][i];

}	/* END XLTWOD */

/* -------------------------------------------------------------------- */
void xlOneD(RAWTBL *varp, void *in, NR_TYPE *np)
{
  int		i, thisTime, n;
  Queue		*probe;
  Particle	*p;

  for (i = 0; i < varp->Length; ++i)
    np[i] = 0.0;

  for (i = 0; i < BINS_64; ++i)
    twoD[varp->ProbeCount][i] = 0.0;


  thisTime = ((short *)in)[1]*3600 + ((short *)in)[2]*60 + ((short *)in)[3];
  probe = probes[varp->ProbeCount];
//printf("xlTwoD entered %x, thisTime = %d\n", probeIDorder[varp->ProbeCount], thisTime);

  /* Make sure we have enough processed data to carry us through this second
   */
  AddMore2dData(probe, thisTime, varp->ProbeCount);


  if (FrontQueue(probe) == NULL)	/* No data left in 2d file? */
{
//printf(" Queue is empty, leaving.\n");
    return;
}

  /* If first time available in Q is later, bail out
   */
  if (thisTime < ((Particle *)FrontQueue(probe))->time)
{
//printf(" Front Q has later time, leaving.  this=%d, Q=%d\n", thisTime, ((Particle *)FrontQueue(probe))->time);
    return;
}


  /* Skip prior particles.  In theory this shouldn't happen.
   */
  while (FrontQueue(probe) && ((Particle *)FrontQueue(probe))->time < thisTime)
    free(DeQueue(probe));


  while (FrontQueue(probe) && ((Particle *)FrontQueue(probe))->time == thisTime)
    {
    p = (Particle *)DeQueue(probe);
//printf("   %d.%d w=%d, h=%d, e=%d\n", ((Particle *)p)->time,
//((Particle *)p)->msec, ((Particle *)p)->w, ((Particle *)p)->h,
//((Particle *)p)->edge);

    n = MAX(p->w, p->h);
    if (n < BINS_64)
      {

      if (p->edge)	/* reconstruct particle if it touches edge */
        {


        }
      else
        twoD[varp->ProbeCount][n] += 1.0;
      }

    if (!p->edge)
      np[p->h] += 1.0;

    free(p);
    }
//printf(" leaving\n");
}	/* END XLONED */

/* -------------------------------------------------------------------- */
static void AddMore2dData(Queue *probe, long thisTime, int probeCnt)
{
  P2d_rec	rec;

  /* No more data? */
  while (probe->tail == NULL)
    {
    if (Next2dRecord(&rec, probeCnt, probeIDorder[probeCnt]) == FALSE)
      return;
    else
      Process(probe, &rec, probeCnt);
    }


  /* Keep adding records until we have all for this second.  We'll need to
   * get at the tail to make sure last particle is greater than current time.
   */
  while (((Particle *)probe->tail->datum)->time <= thisTime &&
		Next2dRecord(&rec, probeCnt, probeIDorder[probeCnt]))
{//printf("  %x thisTime=%d, newRec=%d tail=%d\n", rec.id, thisTime, rec.hour*3600+rec.minute*60+rec.second, ((Particle *)probe->tail->datum)->time);
    Process(probe, &rec, probeCnt);
}

}

/* -------------------------------------------------------------------- */
void Process(Queue *probe, P2d_rec *rec, int probeCnt)
{
  int		i, j, h, partCnt;
  long		endTime;
  ulong		*p, slice, pSlice, ppSlice, timeWord,
		tBarElapsedtime, DASelapsedTime;
  float		tas, frequency, overLap;
  bool		rejectParticle;
  Particle	*part[512];


  /* Perform byte swapping on whole [data] record if required.
   */
  if (1 != ntohs(1))
    {
    short       *sp = (short *)rec;

    for (i = 1; i < 10; ++i, ++sp)
      *sp = ntohs(*sp);


    p = (ulong *)rec->data;

    for (i = 0; i < 1024; ++i, ++p)
      *p = ntohl(*p);
    }

  endTime = rec->hour * 3600 + rec->minute * 60 + rec->second;

  tas = (float)rec->tas * 125 / 255;

  /* Compute frequency, which is used to convert timing words from TAS clock
   * pulses to milliseconds.
   */
  if (((char *)&rec->id)[0] == 'P')
    frequency = 200 / tas;
  else
    frequency = 25 / tas;


  DASelapsedTime = (((endTime * 1000) + rec->msec) -
	((startTime[probeCnt] * 1000) + startMilliSec[probeCnt])) * 1000;

  if (DASelapsedTime == 0)	/* Duplicate record */
    return;

  p = (ulong *)rec->data;
  partCnt = 0;
  tBarElapsedtime = 0;

  /* Locate each particle and characteristics (e.g. h & w, touched edge).
   */
  for (i = 0; i < 1024; ++i, ++p)
    {
    slice = *p;

    /* Ok, found start of particle.
     */
    if (slice == 0xff000000 && ppSlice == 0xffffffff && pSlice != 0xffffffff)
      {
      part[partCnt] = GetMemory(sizeof(Particle));
      part[partCnt]->time = startTime[probeCnt];
      part[partCnt]->msec = startMilliSec[probeCnt];
      part[partCnt]->w = 1;	// first slice of particle is in sync word
      part[partCnt]->h = 1;
      part[partCnt]->edge = FALSE;

      timeWord = pSlice & 0x00ffffff;
      part[partCnt]->timeWord = (ulong)((float)timeWord * frequency);

      if (part[partCnt]->timeWord < DASelapsedTime)
        tBarElapsedtime += part[partCnt]->timeWord;

      /* Determine height of particle.
       */
      ++p; ++i;
      for (; i < 1024 && *p != 0xffffffff; ++p, ++i)
        {
        ++part[partCnt]->w;

        slice = ~(*p);
        if ((slice & 0x80000000) || (slice & 0x00000001)) /* touched edge */
          part[partCnt]->edge = TRUE;

        h = 32;
        for (	j = 0, slice = *p;
		j < 32 && (slice & 0x80000000); slice <<= 1, ++j)
          --h;
        for (	j = 0, slice = *p;
		j < 32 && (slice & 0x00000001); slice >>= 1, ++j)
          --h;

        part[partCnt]->h = MAX(part[partCnt]->h, h);
        }

      ++partCnt;
      }

    ppSlice = p[-1];
    pSlice = p[0];
    }



  /* Add in number of slices in particle, since timing word is in reset
   * state while diode array is shadowed.
   */
  tBarElapsedtime += 1024;
  tBarElapsedtime += overLoad[probeCnt] * 1000;
  p = (ulong *)rec->data;
  for (i = 0; i < 1024; ++i, ++p)
    if (p[i] == 0xffffffff)
      --tBarElapsedtime;


  /* Determine number milliseconds to add/subtract to each particle.
   */
  overLap = (float)tBarElapsedtime / DASelapsedTime;


//printf("DASeTime = %d, tBarEtime=%d, %f\n", DASelapsedTime, tBarElapsedtime, overLap);


  /* Determine whether to reject this record.
   *  - too few particles, indicates something wrong with probe/datasystem.
   *  - too much DAS elaplsed time, means timing words are rolling over and
   *    we can't accuratly compute time for those particles.
   *  - if timeBar total is way out of line with DAS elapsed time.
   */
  if (partCnt < 20)
    {
    printf("Rejecting 2D record: %x %02d:%02d:%02d.%d, too few particles, nPart=%d\n", rec->id, rec->hour, rec->minute, rec->second, rec->msec, partCnt);
    goto cleanup;
    }

  if (DASelapsedTime / 1000000 > partCnt)
    {
    printf("Rejecting 2D record: %x %02d:%02d:%02d.%d, too much elapsed time, %ld seconds.\n", rec->id, rec->hour, rec->minute, rec->second, rec->msec, DASelapsedTime / 1000000);
    goto cleanup;
    }

  if (overLap > 2 && tas < 70)
    {
    printf("Rejecting 2D record: %x %02d:%02d:%02d.%d, oLap=%f, tas=%5.1f\n",
	rec->id, rec->hour, rec->minute, rec->second, rec->msec, overLap, tas);
    goto cleanup;
    }



  /* Go back through particles and add in the timing word to get accurate
   * time for each particle.
   */
  tBarElapsedtime = 0;
  for (i = 0; i < partCnt; ++i)
    {
    if (partCnt < 20 || overLap > 2 && tas < 70 || DASelapsedTime / 1000000 > partCnt)
      rejectParticle = TRUE;
    else
      rejectParticle = FALSE;

    if (part[i]->timeWord < DASelapsedTime)
      tBarElapsedtime += part[i]->timeWord;

    part[i]->msec = startMilliSec[probeCnt] + ((tBarElapsedtime / 1000) * overLap);

//if (!rejectParticle)
//  printf("  new part, time = %d.%d - %d %d %5.1f, ", part[i]->time, part[i]->msec, startMilliSec[probeCnt], tBarElapsedtime, overLap);

    while (part[i]->msec < 0)
      {
      part[i]->time--;
      part[i]->msec += 1000;
      }

    while (part[i]->msec > 999)
      {
      part[i]->time++;
      part[i]->msec -= 1000;
      }

    /* Add in number of slices in particle, since timing word is in reset
     * state while diode array is shadowed.
     */
    tBarElapsedtime += part[i]->w;

//if (!rejectParticle)
//  printf(" %d.%d, w=%d, h=%d\n", part[i]->time, part[i]->msec, part[i]->w, part[i]->h);

    if (rejectParticle)
      free(part[i]);
    else
      EnQueue(probe, part[i]);
    }

cleanup:
  startTime[probeCnt] = endTime;
  startMilliSec[probeCnt] = rec->msec;
  overLoad[probeCnt] = rec->overld;

  if (overLoad[probeCnt] > 40)
    overLoad[probeCnt] = 37;

}

/* END PMS2_DATA.C */
