/*
-------------------------------------------------------------------------
OBJECT NAME:	dataIO.c

FULL NAME:		Data IO

ENTRY POINTS:	ReadData()
				GetData()
				AddVariable()
				DeleteVariable()

STATIC FNS:		free_data()

DESCRIPTION:	Read data from shared memory and process it.

INPUT:			none

OUTPUT:			none
-------------------------------------------------------------------------
*/

#include "define.h"

#include <float.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include <Xm/List.h>

#include "eth_shm.h"

extern XtAppContext    app_con;
extern struct SUN_ETH_SHM *eth_shm;     /* ethernet shared mem adr */
void	CatchAlarm(), AddDataToBuffer();

/* -------------------------------------------------------------------- */
void CatchAlarm()
{
	static NR_TYPE		curYmax, curYmin;
	static int	seq_num = 0;

	if (eth_shm->sync.rseq - seq_num)
		{
		seq_num = eth_shm->sync.rseq;

		DecodeADSrecord((short *)ADSrecord, SampledData);

		if (!Counts)
			ApplyCalCoes(SampledData, SAMPLE_RATE);

		AddDataToBuffer(SampledData);

		AutoScale(SCALE_YAXIS);

		if (!Frozen)
			if (!FlashMode || Variable[0].buffIndex == 0)
				{
				if (curYmin == ymin && curYmax == ymax)
					PlotLines();
				else
					PlotData(NULL, NULL, NULL);

				curYmin = ymin, curYmax = ymax;
				}
		}

	XtAppAddTimeOut(app_con, 100, CatchAlarm, NULL);

}	/* END CATCHALARM */

/* -------------------------------------------------------------------- */
void AddDataToBuffer(newData)
NR_TYPE	*newData;
{
	int		i, j;

	biggest_y = -FLT_MAX;
	smallest_y = FLT_MAX;

	for (i = 0; i < nVariables; ++i)
		{
		memcpy( (char *)&plotData[i][Variable[i].buffIndex],
				(char *)&newData[Variable[i].SRstart],
				NR_SIZE * Variable[i].SampleRate);

		Variable[i].buffIndex += Variable[i].SampleRate;

		if (Variable[i].buffIndex >= Variable[i].nPoints)
			Variable[i].buffIndex -= Variable[i].nPoints;

		for (j = 0; j < Variable[i].nPoints; ++j)
			{
			biggest_y = MAX(biggest_y, plotData[i][j]);
			smallest_y = MIN(smallest_y, plotData[i][j]);
			}
		}

}	/* END ADDDATATOBUFFER */

/* -------------------------------------------------------------------- */
NR_TYPE GetData(set, indx)
int		set;
int		indx;
{
	indx += Variable[set].buffIndex;

	if (indx >= Variable[set].nPoints)
		indx -= Variable[set].nPoints;

	return(plotData[set][indx]);

}	/* END GETDATA */

/* -------------------------------------------------------------------- */
void AddVariable(indx)
int		indx;
{
	if (indx > nsdi)
		{
		indx -= nsdi;

		strcpy(Variable[nVariables].name, raw[indx]->name);
		Variable[nVariables].SampleRate = raw[indx]->SampleRate;
		Variable[nVariables].SRstart = raw[indx]->SRstart;
		Variable[nVariables].nPoints = raw[indx]->SampleRate * NumberSeconds;
		Variable[nVariables].buffIndex = 0;
		}
	else
		{
		strcpy(Variable[nVariables].name, sdi[indx]->name);
		Variable[nVariables].SampleRate = sdi[indx]->SampleRate;
		Variable[nVariables].SRstart = sdi[indx]->SRstart;
		Variable[nVariables].nPoints = sdi[indx]->SampleRate * NumberSeconds;
		Variable[nVariables].buffIndex = 0;
		}

	plotData[nVariables] = (NR_TYPE *)GetMemory(Variable[nVariables].nPoints * NR_SIZE);
	Variable[nVariables].pts =
			(XPoint *)GetMemory(Variable[nVariables].nPoints * sizeof(XPoint));
	memset(plotData[nVariables], 0, Variable[nVariables].nPoints * NR_SIZE);

	++NumberDataSets;
	++nVariables;

	PlotData(NULL, NULL, NULL);

}	/* END ADDVARAIBLE */

/* -------------------------------------------------------------------- */
void DeleteVariable(indx)
int		indx;
{
	int		i;

	free(plotData[indx]);
	free(Variable[indx].pts);

	for (i = indx+1; i < NumberDataSets; ++i)
		{
		memcpy(&Variable[i-1], &Variable[i], sizeof(VARTBL));
		plotData[i-1] = plotData[i];
		}

	--NumberDataSets;
	--nVariables;

	PlotData(NULL, NULL, NULL);

}	/* END DELETEVARIABLE */

NR_TYPE angcr(angle2, angle1, syncro_range, amin)
NR_TYPE angle2;
NR_TYPE angle1;
float   syncro_range;
float   amin;
{
	return(angle2);
}

/* END DATAIO.C */
