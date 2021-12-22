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

#include "define.h"

static long	timeVarID[3];
static int	currentTime[3];

extern XtAppContext context;

static int	highestRate;

static void PrintVariables();


/* -------------------------------------------------------------------- */
LowRateLoop(starttime, endtime)
long	starttime;
long	endtime;
{
	int		rc, i;

	if ((rc = FindFirstRecordNumber(starttime)) == ERR)
		goto exit;

	/* Print Title
	 */
	{
	int		siz;
	VARTBL	*vp;

	fprintf(OutputFile, "        ");

	highestRate = 1;

	for (i = 0; i < nVariables; ++i)
		if ((vp = Variable[i])->Output) 
			{
			if ((siz = atoi(&vp->Format[1])) < 5)
				siz = 10;

			fprintf(OutputFile, "%*s", siz, Variable[i]->name);

			if (Variable[i]->VectorLength > 1)
				{
				int	j;

				for (j = 2; j < Variable[i]->VectorLength; ++j)
					fprintf(OutputFile, "%*d", siz, j);
				}

			highestRate = MAX(highestRate, vp->OutputRate);
			}

	fprintf(OutputFile, "\n");
	}


	/* This is the main control loop.
	 */
	do
		{
		PrintVariables();
		UpdateTime(currentTime);

		while (PauseFlag == TRUE)
			XtAppProcessEvent(context, XtIMAll);

		if (PauseWhatToDo == P_QUIT)
			{
			rc = ERR;
			goto exit;
			}
		}
	while ((rc = FindNextRecordNumber(endtime)) == OK);

	rc = OK;

exit:
	return(rc);

}	/* END LOWRATELOOP */

/* -------------------------------------------------------------------- */
static void PrintVariables()
{
  int		i, j, msec;
  NR_TYPE	data[640];
  VARTBL	*vp;

  static long	start[] = {0, 0, 0};
  static long	count[] = {1, 1, 1};

  start[0] = CurrentInputRecordNumber;

  for (msec = 0; msec < 1000; msec += (1000 / highestRate))
    {
    /* Print timestamp / rolling counter.
     */
    if (XaxisType == TIME)
      {
      fprintf(OutputFile, "%02d:%02d:%02d",
              currentTime[0], currentTime[1], currentTime[2]);

      if (highestRate > 1)
        fprintf(OutputFile, ".%03d", msec);
      }
    else
      fprintf(OutputFile, "%8d", CurrentInputRecordNumber);


    for (i = 0; i < nVariables; ++i)
      if ((vp = Variable[i])->Output)
        {

        if (vp->VectorLength > 1)
          {
          count[1] = vp->OutputRate;
          count[2] = vp->VectorLength;

          ncvarget(InputFile, vp->inVarID, start, count, (void *)data);

          if (strchr(vp->Format, 'd'))
            for (j = 1; j < vp->VectorLength; ++j)
              fprintf(OutputFile, vp->Format, (int)data[j]);
          else
            for (j = 1; j < vp->VectorLength; ++j)
              fprintf(OutputFile, vp->Format, data[j]);
          }
        else
          {
          ncvarget1(InputFile, vp->inVarID, start, (void *)data);

          if (strchr(vp->Format, 'd'))
            fprintf(OutputFile, vp->Format, (int)data[0]);
          else
            fprintf(OutputFile, vp->Format, data[0]);
          }
        }
    }

  fprintf(OutputFile, "\n");

}	/* END PRINTVARIABLES */
 
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
