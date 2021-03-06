/*
-------------------------------------------------------------------------
OBJECT NAME:	pms2_data.c

FULL NAME:	Translate PMS2D records to look like 1D histograms

ENTRY POINTS:	xlTwoD(), xlTwoDG()

STATIC FNS:	none

DESCRIPTION:	This sums up the data bins from SampleRate to 1hz.

INPUT:		

OUTPUT:		

NOTES:		Order of raw variables is determined alphabetically.  There
		are some not so raw (more like derived) variables being
		created here.  Watch out changing variable names (A1DC, etc).

REFERENCES:	none

REFERENCED BY:	rec_decode.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1999-2000
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "amlib.h"
#include "queue.h"

#define OVERLOAD	(-1)

static Queue	*probes[MAX_PMS2];

/* Data from the file is only marked as 'C1', 'C2', 'P1', 'P2', we'll
 * use this array to determine which records go with which Queue above.
 */
static short	probeIDorder[MAX_PMS2] = { 0, 0, 0, 0 };
static long	startTime[MAX_PMS2];
static short	startMilliSec[MAX_PMS2];
static float	twoD[MAX_PMS2][BINS_64];


/* Exported to amlib/std/pms2d.c */

NR_TYPE	deadTime[MAX_PMS2][2];	/* Probe down time (subtract from 1 second) */
int	overFlowCnt[MAX_PMS2];	/* Number of particles greater than 64 bins */


struct particle
  {
  long	time;		/* Seconds since mid-night	*/
  long	msec;

  int		w, h;
  bool		edge;		/* particle touched edge	*/
  ulong		timeWord;
  NR_TYPE	deltaTime; /* Amount of time between prev & this particle */
  NR_TYPE	liveTime; /* Amount of time consumed by particle */
/*  ulong	*p; */
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

printf("  %x startTime = %d.%d\n", rec.id, startTime[probeCnt], startMilliSec[probeCnt]);
    }

  sTwodInit(varp);

}	/* END INITTWOD */

/* -------------------------------------------------------------------- */
void xl2dDeadTime1(RAWTBL *varp, void *in, NR_TYPE *np)
{
  /* Return Milliseconds.
   */
  *np = deadTime[varp->ProbeCount][0] / 1000;
//printf("dt[%d][0] = %f\n", varp->ProbeCount, deadTime[varp->ProbeCount][0]);
}	/* END XLTWOD */

/* -------------------------------------------------------------------- */
void xl2dDeadTime2(RAWTBL *varp, void *in, NR_TYPE *np)
{
  /* Return Milliseconds.
   */
  *np = deadTime[varp->ProbeCount][1] / 1000;
//printf("dt[%d][1] = %f\n", varp->ProbeCount, deadTime[varp->ProbeCount][1]);
}	/* END XLTWOD */

/* -------------------------------------------------------------------- */
void xlTwoD(RAWTBL *varp, void *in, NR_TYPE *np)
{
  int		i;
  int		probeCnt = varp->ProbeCount;

  for (i = 0; i < varp->Length; ++i)
    np[i] = twoD[probeCnt][i];

}	/* END XLTWOD */

/* -------------------------------------------------------------------- */
void xlOneD(RAWTBL *varp, void *in, NR_TYPE *np)
{
  int		i, thisTime, n;
  Queue		*probe;
  Particle	*p;

  overFlowCnt[varp->ProbeCount] = 0;
  deadTime[varp->ProbeCount][0] = 0.0;
  deadTime[varp->ProbeCount][1] = 0.0;
  memset((void *)np, 0, NR_SIZE * varp->Length);
  memset((void *)twoD[varp->ProbeCount], 0, NR_SIZE * BINS_64);
/*
np[16] = 13.25 * 49;
deadTime[varp->ProbeCount][0] = 13.25* 0.036;
deadTime[varp->ProbeCount][1] = 13.25 * 0.036;
twoD[varp->ProbeCount][17] = 13.25 * 49;
return;
*/
//printf("------ %02d:%02d:%02d -----------------------------------\n",
//	((short *)in)[1], ((short *)in)[2], ((short *)in)[3]);

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
//printf("   %d.%d w=%d, h=%d, e=%d - %d\n", ((Particle *)p)->time,
//  ((Particle *)p)->msec, ((Particle *)p)->w, ((Particle *)p)->h,
//  ((Particle *)p)->edge, ((Particle *)FrontQueue(probe))->time);

    if (p->timeWord == OVERLOAD)
      {
//printf("OVERLOAD, incr deadTime by %d\n", p->deltaTime);
      deadTime[varp->ProbeCount][0] += p->deltaTime;
      deadTime[varp->ProbeCount][1] += p->deltaTime;
      }
    else
      {
      /* A2DC */
      if (1 /* rejection criteria */)
        {
        n = MAX(p->w, p->h);

        if (n < BINS_64)
          {
          twoD[varp->ProbeCount][n] += 1.0;
          }
        else
          ++overFlowCnt[varp->ProbeCount];
        }
      else
        deadTime[varp->ProbeCount][1] += p->liveTime;

      /* A1DC */
      if (p->h > 0 && !p->edge)
        np[p->h] += 1.0;
      else
        deadTime[varp->ProbeCount][0] += p->liveTime;
      }

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
{//printf("AddMore2dData: returning empty handed, no more data\n");
      return;
}
    else
      Process(probe, &rec, probeCnt);
    }


  /* Keep adding records until we have all for this second.  We'll need to
   * get at the tail to make sure last particle is greater than current time.
   */
  while (((Particle *)probe->tail->datum)->time <= thisTime &&
		Next2dRecord(&rec, probeCnt, probeIDorder[probeCnt]))
{//printf("  %x thisTime=%d, newRec=%d tail=%d, ol=%d\n", rec.id, thisTime, rec.hour*3600+rec.minute*60+rec.second, ((Particle *)probe->tail->datum)->time, rec.overld);
    Process(probe, &rec, probeCnt);
//  printf("    %02d:%02d:%02d  tail=%d\n", rec.hour, rec.minute, rec.second, ((Particle *)probe->tail->datum)->time);
}

}

/* -------------------------------------------------------------------- */
void Process(Queue *probe, P2d_rec *rec, int probeCnt)
{
  int		i, j, h, partCnt;
  long		endTime, oload;
  bool		firstParticleAfter512;
  ulong		*p, slice, pSlice, ppSlice, tBarElapsedtime, DASelapsedTime;
  float		tas, frequency, overLap;
  Particle	*part[512], *cp; /* cp stands for "currentParticle" */

  static int	overLoad = 0;


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

  /* Overload should never exceed 60 milliseconds, the new 2d interface card
   * starting in 1996 (Sheba) has some bugs regarding this.
   */
  overLoad = rec->overld;
  if (overLoad == 1000)
    overLoad = 0;
  else
  if (overLoad > 100)
    overLoad = 50;

  /* Compute frequency, which is used to convert timing words from TAS clock
   * pulses to microseconds.
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
  tBarElapsedtime = 1024;
  firstParticleAfter512 = TRUE;

  /* Locate each particle and characteristics (e.g. h & w, touched edge).
   */
  for (i = 0; i < 1024; ++i, ++p)
    {
    if ((slice = *p) == 0xffffffff)
      --tBarElapsedtime;

    /* Ok, found start of particle.
     */
//    if (slice == 0xff000000 && ppSlice == 0xffffffff && pSlice != 0xffffffff)
    if (slice == 0x55000000 && ppSlice == 0xffffffff && pSlice != 0xffffffff)
      {
      if (i > 512 && firstParticleAfter512 && overLoad > 0)
        {
        /* Overload is treated as a particle.
         */
        cp = part[partCnt++] = GetMemory(sizeof(Particle));

        cp->time = startTime[probeCnt];
        cp->msec = startMilliSec[probeCnt];
        cp->deltaTime = (NR_TYPE)overLoad * 1000; /* microseconds */
        cp->timeWord = OVERLOAD;
        cp->w = 0;
        cp->h = 0;

        firstParticleAfter512 = FALSE;
        }

      cp = part[partCnt++] = GetMemory(sizeof(Particle));
      cp->time = startTime[probeCnt];
      cp->msec = startMilliSec[probeCnt];
      cp->timeWord = pSlice & 0x00ffffff;
      cp->deltaTime = (NR_TYPE)cp->timeWord * frequency;
      cp->w = 1;	/* first slice of particle is in sync word */
      cp->h = 1;
      cp->edge = FALSE;

      if ((ulong)cp->deltaTime < DASelapsedTime)
        tBarElapsedtime += cp->timeWord;

      /* Determine height of particle.
       */
      ++p; ++i;
      for (; i < 1024 && *p != 0xffffffff; ++p, ++i)
        {
        ++cp->w;

        slice = ~(*p);
        if ((slice & 0x80000000) || (slice & 0x00000001)) /* touched edge */
          cp->edge = TRUE;

        h = 32;
        for (	j = 0, slice = *p;
		j < 32 && (slice & 0x80000000); slice <<= 1, ++j)
          --h;
        for (	j = 0, slice = *p;
		j < 32 && (slice & 0x00000001); slice >>= 1, ++j)
          --h;

        cp->h = MAX(cp->h, h);
        }

      /* If the particle becomes rejected later, we need to now much time the
       * particle consumed, so we can add it to the deadTime, so sampleVolume
       * can be reduced accordingly.
       */
      cp->liveTime = (NR_TYPE)((cp->w + 1) * frequency);
      }

    ppSlice = p[-1];
    pSlice = p[0];
    }


  /* Determine number milliseconds to add/subtract to each particle.
   */
  tBarElapsedtime *= frequency;	/* Convert to microseconds */
  tBarElapsedtime += overLoad * 1000;
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

  if (DASelapsedTime > 8000000)	/* 8 seconds */
    {
    printf("Rejecting 2D record: %x %02d:%02d:%02d.%d, too much elapsed time, %ld seconds.\n", rec->id, rec->hour, rec->minute, rec->second, rec->msec, DASelapsedTime / 1000000);
    goto cleanup;
    }

  if (overLap > 2 && tas < 70.0)
    {
    printf("Rejecting 2D record: %x %02d:%02d:%02d.%d, oLap=%f, tas=%5.1f\n",
	rec->id, rec->hour, rec->minute, rec->second, rec->msec, overLap, tas);
    goto cleanup;
    }



  /* Go back through particles and add in the timing word to get accurate
   * time for each particle.
   */
  tBarElapsedtime = 0;
  oload = 0;
  overLap = 1.0 / overLap;

  for (i = 0; i < partCnt; ++i)
    {
    cp = part[i];

    if (cp->deltaTime < DASelapsedTime)
      {
      if (cp->timeWord == OVERLOAD)
        oload = cp->deltaTime / 1000;
      else
        tBarElapsedtime += cp->timeWord;
      }

    cp->msec = startMilliSec[probeCnt] +
		(((tBarElapsedtime * frequency / 1000) + oload) * overLap);

    while (cp->msec < 0)
      {
      cp->time--;
      cp->msec += 1000;
      }

    while (cp->msec > 999)
      {
      cp->time++;
      cp->msec -= 1000;
      }

    /* Add in number of slices in particle, since timing word is in reset
     * state while diode array is shadowed.
     */
    tBarElapsedtime += cp->w + 1;

    EnQueue(probe, cp);
    }

cleanup:
  startTime[probeCnt] = endTime;
  startMilliSec[probeCnt] = rec->msec;


}	/* END PROCESS */

/* END PMS2_DATA.C */
