/*
-------------------------------------------------------------------------
OBJECT NAME:	despike.c

FULL NAME:		Despiking object routines

ENTRY POINTS:	AddVariableToSDIdespikeList()
				AddVariableToRAWdespikeList()
				DespikeData()

STATIC FNS:		despike()

DESCRIPTION:	This implementation will/can only remove single point
				spikes.  The LaGrange polynomial is used for interpolation.

INPUT:		

OUTPUT:		

REFERENCES:		none

REFERENCED BY:	hdr_decode.c, lrloop.c, hrloop.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1995
-------------------------------------------------------------------------
*/

#include <math.h>

#include "nimbus.h"
#include "decode.h"
#include "circbuff.h"

#define MAX_VARIABLES	128

static SDITBL	*sdi_spike[MAX_VARIABLES];
static RAWTBL	*raw_spike[MAX_VARIABLES];
static int		nsdi_spike = 0, nraw_spike = 0;
static int		nSpikesSDI[MAX_VARIABLES], nSpikesRAW[MAX_VARIABLES];

static void checkVariable(int SRstart, int rate, NR_TYPE slope, int *counter);
static NR_TYPE despike(NR_TYPE *points);


/* -------------------------------------------------------------------- */
void AddVariableToSDIdespikeList(SDITBL *varp)
{
	if (nsdi_spike >= MAX_VARIABLES)
		{
		LogMessage("despike.c: MAX_VARIABLES exceeded.\n");
		return;
		}

	sdi_spike[nsdi_spike] = varp;
	nSpikesSDI[nsdi_spike] = 0;
	++nsdi_spike;

}
 
/* -------------------------------------------------------------------- */
void AddVariableToRAWdespikeList(RAWTBL *varp)
{
	if (nraw_spike >= MAX_VARIABLES)
		{
		LogMessage("despike.c: MAX_VARIABLES exceeded.\n");
		return;
		}

	raw_spike[nraw_spike] = varp;
	nSpikesRAW[nraw_spike] = 0;
	++nraw_spike;

}

/* -------------------------------------------------------------------- */
void LogDespikeInfo()
{
	int	i;

	for (i = 0; i < nsdi_spike; ++i)
		{
		if (nSpikesSDI[i] > 0)
			{
			sprintf(buffer, "%s: %d spikes removed with slope exceeding %f\n",
				sdi_spike[i]->name, nSpikesSDI[i], sdi_spike[i]->SpikeSlope);

			LogMessage(buffer);
			}
		}

	for (i = 0; i < nraw_spike; ++i)
		{
		if (nSpikesRAW[i] > 0)
			{
			sprintf(buffer, "%s: %d spikes removed with slope exceeding %f\n",
				raw_spike[i]->name, nSpikesRAW[i], raw_spike[i]->SpikeSlope);

			LogMessage(buffer);
			}
		}

}	/* END LOGDESPIKEINFO */

/* -------------------------------------------------------------------- */
static NR_TYPE	*prev_rec, *this_rec, *next_rec;

void DespikeData(
	CircularBuffer	*LRCB,
	int				index)		/* Index into CircBuff			*/
{
	int		i;

	/* I don't expect to be doing despiking often, so let's check.
	 */
	if (nsdi_spike == 0 && nraw_spike == 0)
		return;

	prev_rec	= (NR_TYPE *)GetBuffer(LRCB, index-1);
	this_rec	= (NR_TYPE *)GetBuffer(LRCB, index);
	next_rec	= (NR_TYPE *)GetBuffer(LRCB, index+1);

	for (i = 0; i < nsdi_spike; ++i)
		checkVariable(	sdi_spike[i]->SRstart,
						sdi_spike[i]->SampleRate,
						sdi_spike[i]->SpikeSlope,
						&nSpikesSDI[i]);

	for (i = 0; i < nraw_spike; ++i)
		checkVariable(	raw_spike[i]->SRstart,
						raw_spike[i]->SampleRate,
						raw_spike[i]->SpikeSlope,
						&nSpikesRAW[i]);

}	/* END DESPIKEDATA */

/* -------------------------------------------------------------------- */
static void checkVariable(int SRstart, int SampleRate, NR_TYPE SpikeSlope, int *counter)
{
	int		i, ix, spike[1000], spikeCount = 0;
	NR_TYPE	points[5],
			dir1, dir2;	/* Direction of data	*/


	/* Do first point.
	 */
	points[0] = prev_rec[SRstart + SampleRate - 2];
	points[1] = prev_rec[SRstart + SampleRate - 1];
	memcpy((char *)&points[2], (char *)&this_rec[SRstart], NR_SIZE * 3);

	dir1 = points[1] - points[2];
	dir2 = points[2] - points[3];

	if (dir1 * dir2 < 0.0 &&
		fabs((double)dir1) > SpikeSlope && fabs((double)dir2) > SpikeSlope)
		{
		this_rec[SRstart] = despike(points);
		*counter++;
		}
	
	/* Do second point
	 */
	points[0] = points[1];
	memcpy((char *)&points[1], (char *)&this_rec[SRstart], NR_SIZE * 4);

	dir1 = points[1] - points[2];
	dir2 = points[2] - points[3];

	if (dir1 * dir2 < 0.0 &&
		fabs((double)dir1) > SpikeSlope && fabs((double)dir2) > SpikeSlope)
		{
		this_rec[SRstart + 1] = despike(points);
		*counter++;
		}


	for (i = 2; i < SampleRate - 1; ++i)
		{
		ix = SRstart + i;

		dir1 = this_rec[ix-1] - this_rec[ix];
		dir2 = this_rec[ix] - this_rec[ix+1];

		if (dir1 * dir2 < 0.0 &&
			fabs((double)dir1) > SpikeSlope && fabs((double)dir2) > SpikeSlope)
			{
			spike[spikeCount++] = ix;
			++i;
			}
		}


	for (i = 0; i < spikeCount; ++i)
		if (spike[i] + 2 != spike[i+1])
			this_rec[spike[i]] = despike(&this_rec[spike[i]-2]);
		else
			this_rec[spike[i]] = (this_rec[spike[i]-1] + this_rec[spike[i]+1])
								/ 2.0;

	*counter += spikeCount;


	/* Do last 2 points.
	 */
	ix = SRstart + SampleRate;

	memcpy((char *)points, (char *)&this_rec[ix - 4], NR_SIZE*4);
	points[4] = next_rec[SRstart];

	dir1 = points[1] - points[2];
	dir2 = points[2] - points[3];

	if (dir1 * dir2 < 0.0 &&
		fabs((double)dir1) > SpikeSlope && fabs((double)dir2) > SpikeSlope)
		{
		this_rec[ix - 2] = despike(points);
		*counter++;
		}
	
	memcpy((char *)points, (char *)&this_rec[ix - 3], NR_SIZE*3);
	points[3] = points[4];
	points[4] = next_rec[SRstart + 1];

	dir1 = points[1] - points[2];
	dir2 = points[2] - points[3];

	if (dir1 * dir2 < 0.0 &&
		fabs((double)dir1) > SpikeSlope && fabs((double)dir2) > SpikeSlope)
		{
		this_rec[ix - 1] = despike(points);
		*counter++;
		}

}	/* END CHECKVARIABLE */

/* -------------------------------------------------------------------- */
static NR_TYPE despike(NR_TYPE *points)
{
	int		i;
	NR_TYPE	y = 0.0;

	static NR_TYPE	c[] = { -0.166667, 0.666667, 0.0, 0.666667, -0.166667 };

	for (i = 0; i < 5; ++i)
		if (i != 2)
			y += c[i] * points[i];

	return(y);

}	/* END DESPIKE */

/* END DESPIKE.C */
