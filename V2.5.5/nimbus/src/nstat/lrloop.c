/*
-------------------------------------------------------------------------
OBJECT NAME:	lrloop.c

FULL NAME:		Main loop for low rate processing

ENTRY POINTS:	LowRateLoop()
				FindFirstLogicalRecord()
				FindNextLogicalRecord()

STATIC FNS:		none

DESCRIPTION:	

INPUT:			long beginning and ending times

OUTPUT:		

REFERENCES:		average.c, compute.c, netcdf.c, timeseg.c

REFERENCED BY:	StartProcessing()

COPYRIGHT:		University Corporation for Atmospheric Research, 1993
-------------------------------------------------------------------------
*/

#include <math.h>
#include "define.h"

static long		timeVarID[3];
static int		currentTime[3];
static NR_TYPE	*data;

extern XtAppContext context;

static void ComputeStats(), PrintTitle(), PrintReport();


/* -------------------------------------------------------------------- */
LowRateLoop(startTime, endTime)
long	startTime;
long	endTime;
{
	int		rc, i;
	int		nSecs, nPasses;

	data = (NR_TYPE *)GetMemory(DefaultSeconds * sizeof(NR_TYPE) * 1000);

	if (startTime == BEG_OF_TAPE)
		nSecs = nRecords;
	else
		nSecs = endTime - startTime;

	nPasses = nSecs / DefaultSeconds;

	if (nSecs % DefaultSeconds)
		++nPasses;


	if ((rc = FindFirstRecordNumber(startTime)) == ERR)
		nPasses = 0;

	/* Print header
	 */
	sprintf(buffer, "\t\tProject: %s, Flight #: %s\n",ProjectName,FlightNumber);
	Output(buffer);

	for (i = 0; i < nPasses; ++i, nSecs -= DefaultSeconds)
		{
		UpdateTime(currentTime);

		PrintTitle(nSecs > DefaultSeconds ? DefaultSeconds : nSecs);
		PrintReport(nSecs > DefaultSeconds ? DefaultSeconds : nSecs);

		CurrentInputRecordNumber += DefaultSeconds - 1;
		if (FindNextRecordNumber(endTime) != OK)
			break;

		while (PauseFlag == TRUE)
			XtAppProcessEvent(context, XtIMAll);

		if (PauseWhatToDo == P_QUIT)
			{
			rc = ERR;
			break;
			}
		}

exit:
	free((char *)data);
	return(rc);

}	/* END LOWRATELOOP */

/* -------------------------------------------------------------------- */
static void PrintReport(sex)
int		sex;
{
	int		i, j, nPoints, missCnt;
	double	mean, sigma, variance, min, max;
	VARTBL	*vp;

	static long start[] = {0, 0, 0};
	static long count[] = {1, 1, 1};

	start[0] = CurrentInputRecordNumber;
	count[0] = sex;

	for (i = 0; i < nVariables; ++i)
		if ((vp = Variable[i])->Output)
			{
			count[1] = vp->OutputRate;
			nPoints = sex * vp->OutputRate;

			ncvarget(InputFile, vp->inVarID, start, count, (void *)data);

			mean = sigma = 0.0;
			max = -999999999.0, min = 999999999.0;

			for (missCnt = j = 0; j < nPoints; ++j)
				{
				if ((int)data[j] == MISSING_VALUE)
					{
					++missCnt;
					continue;
					}

				mean += data[j];
				min = MIN(min, data[j]);
				max = MAX(max, data[j]);
				}

			nPoints -= missCnt;

			mean /= nPoints;

			for (j = 0; j < nPoints; ++j)
				sigma += pow(data[j] - mean, 2.0);

			variance = sigma / (nPoints - 1);
			sigma = sqrt(sigma / (nPoints - 1));

			sprintf(buffer, "%-12s% -12s%15.4lf%15.5lf%15.5lf%15.4lf%15.4lf\n",
						vp->name, vp->Units, mean, sigma, variance, min, max);

			Output(buffer);
			}

}	/* END PRINTVARIABLES */
 
/* -------------------------------------------------------------------- */
static void PrintTitle(sex)
int		sex;
{
	int		endTime[3];
	int		hours, mins, secs;

	memcpy(endTime, currentTime, sizeof(currentTime));

	hours = sex / 3600; sex -= hours * 3600;
	mins = sex / 60; sex -= mins * 60;
	secs = sex;

	if ((endTime[2] += secs) > 60)
		{
		endTime[1] += endTime[2] / 60;
		endTime[2] %= 60;
		}

	if ((endTime[1] += mins) > 60)
		{
		endTime[0] += endTime[1] / 60;
		endTime[1] %= 60;
		}

	if ((endTime[0] += hours) > 24)
		endTime[0] %= 24;

	sprintf(buffer, "\n\t%02d:%02d:%02d - %02d:%02d:%02d\n",
		currentTime[0], currentTime[1], currentTime[2],
		endTime[0], endTime[1], endTime[2]);
	Output(buffer);

	Output("\nName        Units                  Mean          Sigma       Variance           Min            Max\n");
	Output("---------------------------------------------------------------------------------------------------\n");

}	/* END PRINTTTITLE */

/* -------------------------------------------------------------------- */
FindFirstRecordNumber(starttime)
long	starttime;
{
	timeVarID[0] =
			Variable[SearchTable(Variable, nVariables, "HOUR")]->inVarID;
	timeVarID[1] =
			Variable[SearchTable(Variable, nVariables, "MINUTE")]->inVarID;
	timeVarID[2] =
			Variable[SearchTable(Variable, nVariables, "SECOND")]->inVarID;

	CurrentInputRecordNumber = -1;

	if (starttime != BEG_OF_TAPE)
		while (FindNextRecordNumber(starttime) == OK)
			;
	else
		FindNextRecordNumber(starttime);

	return(OK);

}	/* END FINDFIRSTRECORDNUMBER */

/* -------------------------------------------------------------------- */
FindNextRecordNumber(endtime)
long	endtime;
{
	int		current_time;
	long	mindex[1];
	NR_TYPE	f;

	static bool	rollOver = FALSE;

	if (++CurrentInputRecordNumber >= nRecords)	/* End of tape	*/
		return(ERR);

	mindex[0] = CurrentInputRecordNumber;

	ncvarget1(InputFile, timeVarID[0], mindex, (void *)&f);
	current_time = (int)f * 3600;
	currentTime[0] = (int)f;

	ncvarget1(InputFile, timeVarID[1], mindex, (void *)&f);
	current_time += (int)f * 60;
	currentTime[1] = (int)f;

	ncvarget1(InputFile, timeVarID[2], mindex, (void *)&f);
	current_time += (int)f;
	currentTime[2] = (int)f;

	if (endtime == END_OF_TAPE)
		return(OK);


	if (current_time == 0 && CurrentInputRecordNumber > 0)
		rollOver = TRUE;

	if (rollOver)
		current_time += 86400;

	if (current_time < endtime)
		return(OK);
	else
		return(ERR);

}	/* END FINDNEXTRECORDNUMBER */

/* END LRLOOP.C */
