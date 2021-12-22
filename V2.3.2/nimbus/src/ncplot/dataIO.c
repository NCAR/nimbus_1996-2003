/*
-------------------------------------------------------------------------
OBJECT NAME:	dataIO.c

FULL NAME:		Data File IO

ENTRY POINTS:	NewDataFile()
				ReadData()
				AddVariable()
				DeleteVariable()

STATIC FNS:		free_data()
				FindMinMax()

DESCRIPTION:	

INPUT:			none

OUTPUT:			none
-------------------------------------------------------------------------
*/

#include "define.h"

#include <math.h>
#include <string.h>
#include <float.h>
#include <Xm/FileSB.h>
#include <Xm/List.h>

#include "netcdf.h"


static int	InputFile;

static void	FindMinMax(), free_data();

/* -------------------------------------------------------------------- */
void NewDataFile(Widget w, XtPointer client, XtPointer call)
{
	VARTBL		*vp;
	XmString	item[MAX_VARIABLES];
	int			i, len, nVars, nDims, dimIDs[3];
	char		name[NAMLEN*2];


	if (nVariables > 0)
		{
		ncclose(InputFile);
		free_data();

		free(ProjectName);
		free(ProjectNumber);
		free(FlightNumber);

		for (i = 0; i < nVariables; ++i)
			free((char *)Variable[i]);

		nVariables = 0;
		NumberDataSets = 0;
		DataChanged = TRUE;
		RecomputeSpectra = TRUE;
		XmListDeleteAllItems(varList);

		title[0] = subtitle[0] = ylabel[0] = '\0';
		}
	else
		ncopts = 0;


	/* If this was called from the menu then get the file name
	 * already selected.
	 */
	if (call)
		{
		FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

		ExtractFileName(((XmFileSelectionBoxCallbackStruct *)
			call)->value, &data_file);
		}


	/* Open Input File
	 */
	if ((InputFile = ncopen(data_file, NC_NOWRITE)) == ERR)
		{
		sprintf(buffer, "Can't open %s.\n", data_file);
		HandleError(buffer, Interactive, IRET);
		}

	ncinquire(InputFile, NULL, &nVars, NULL, NULL);


	if (ncattinq(InputFile, NC_GLOBAL, "ProjectName", (nc_type *)0, &len) != -1)
		{
		ProjectName = GetMemory(len);
		ncattget(InputFile, NC_GLOBAL, "ProjectName", ProjectName);
		}
	else
		{
		ProjectName = GetMemory(1);
		ProjectName[0] = '\0';
		}

	ncattinq(InputFile, NC_GLOBAL, "ProjectNumber", (nc_type *)0, &len);
	ProjectNumber = GetMemory(len);
	ncattget(InputFile, NC_GLOBAL, "ProjectNumber", ProjectNumber);

	ncattinq(InputFile, NC_GLOBAL, "FlightNumber", (nc_type *)0, &len);
	FlightNumber = GetMemory(len);
	ncattget(InputFile, NC_GLOBAL, "FlightNumber", FlightNumber);

	ncattinq(InputFile, NC_GLOBAL, "FlightDate", (nc_type *)0, &len);
	FlightDate = GetMemory(len);
	ncattget(InputFile, NC_GLOBAL, "FlightDate", FlightDate);


	sprintf(title, "%s - %s, Flight #%s",
						ProjectName, ProjectNumber, FlightNumber);

	ncattget(InputFile, NC_GLOBAL, "TimeInterval", buffer);
	sscanf(buffer, "%02d:%02d:%02d-%02d:%02d:%02d",
					&FileStartTime[0], &FileStartTime[1], &FileStartTime[2],
					&FileEndTime[0], &FileEndTime[1], &FileEndTime[2]);

	if (FileStartTime[0] > FileEndTime[0])
		FileEndTime[0] += 24;


	/* Set default TimeSlice as the first 15 minutes
	 */
	NumberSeconds = 900;
	memcpy((char *)UserStartTime, (char *)FileStartTime, 3 * sizeof(int));
	memcpy((char *)UserEndTime, (char *)FileStartTime, 3 * sizeof(int));

	if ((UserEndTime[1] += 15) > 59)
		{
		UserEndTime[0] += 1;
		UserEndTime[1] -= 60;
		}

	if (memcmp((char *)UserEndTime, (char *)FileEndTime, 3 * sizeof(int)) > 0)
		{
		memcpy((char *)UserEndTime, (char *)FileEndTime, 3 * sizeof(int));

		i = 60 - UserStartTime[2];
		i += (UserEndTime[1] - (UserStartTime[1] + 1)) * 60;
		i += UserEndTime[2];

		NumberSeconds = i + 1;
		}

	if (timeSeg)
		{
		sscanf(timeSeg, "%02d:%02d:%02d-%02d:%02d:%02d",
				&UserStartTime[0], &UserStartTime[1], &UserStartTime[2],
				&UserEndTime[0], &UserEndTime[1], &UserEndTime[2]);

		NumberSeconds =
			(UserEndTime[0]*3600 + UserEndTime[1]*60 + UserEndTime[2]) -
			(UserStartTime[0]*3600 + UserStartTime[1]*60 + UserStartTime[2]);

		free(timeSeg); timeSeg = NULL;
		}

	SetSubtitle();

	for (i = 2; i < nVars; ++i)
		{
		ncvarinq(InputFile, i, name, NULL, &nDims, dimIDs, NULL);

		if (nDims > 2)
			continue;

		vp = Variable[nVariables++] = (VARTBL *)GetMemory(sizeof(VARTBL));

		strcpy(vp->name, name);
		if (nDims == 1)
			vp->OutputRate = 1;
		else
			ncdiminq(InputFile, dimIDs[1],NULL, (void *)&vp->OutputRate);

		vp->inVarID	= i;
		vp->Output	= FALSE;
		}

	SortTable((char **)Variable, 0, nVariables - 1);

	if (!Interactive)
		return;

	for (i = 0; i < nVariables; ++i)
		item[i] = XmStringCreateLocalized(Variable[i]->name);

	XmListAddItems(varList, item, nVariables, 1);

	for (i = 0; i < nVariables; ++i)
		XtFree(item[i]);

}	/* END NEWDATAFILE */

/* -------------------------------------------------------------------- */
void ReadData(Widget w, XtPointer client, XtPointer call)
{
	int		i, n;
	long	startTime, endTime;
	long	fileStartTime, fileEndTime;
	long	start[2], count[2];
	VARTBL	*vp;

	/* Perform time computations.
	 */
	startTime = UserStartTime[0]*3600 + UserStartTime[1]*60 + UserStartTime[2];
	endTime = UserEndTime[0]*3600 + UserEndTime[1]*60 + UserEndTime[2];

	fileStartTime = FileStartTime[0]*3600+FileStartTime[1]*60+FileStartTime[2];
	fileEndTime = FileEndTime[0]*3600 + FileEndTime[1]*60 + FileEndTime[2];

	if (endTime < startTime)
		endTime += 86400;		/* 24 hours	*/

	if (fileEndTime < fileStartTime)
		fileEndTime += 86400;	/* 24 hours	*/

	if (startTime < fileStartTime)
		{
		startTime = fileStartTime;

        UserStartTime[0] = startTime / 3600;
        UserStartTime[1] = (startTime - (UserStartTime[0]*3600)) / 60;
        UserStartTime[2] = (startTime - (UserStartTime[0]*3600) - (UserStartTime[1]*60));
		}

	if (endTime > fileEndTime)
		{
		endTime = fileEndTime;

        UserEndTime[0] = endTime / 3600;
        UserEndTime[1] = (endTime - (UserEndTime[0]*3600)) / 60;
        UserEndTime[2] = (endTime - (UserEndTime[0]*3600) - (UserEndTime[1]*60));
		}

	NumberSeconds = endTime - startTime;


	/* Free any old data	*/
	if ((int)call > 0 && (int)call < NumberDataSets)
		{
		i = (int)call;
		n = i + 1;
		}
	else
		{
		i = 0;
		n = NumberDataSets;
		free_data();
		}

	for (; i < n; ++i)
		{
		vp = Variable[SelectedVarIndex[i]];

		NumberElements[i] = NumberSeconds * vp->OutputRate;

		data[i] = (NR_TYPE *)GetMemory(NumberElements[i]*sizeof(NR_TYPE));

		start[0] = startTime - fileStartTime;
		start[1] = 0;

		count[0] = NumberSeconds;
		count[1] = vp->OutputRate;

		ncvarget(InputFile, vp->inVarID, start, count, (void *)data[i]);
		}

	FindMinMax();

	sprintf(subtitle, "%s, %02d:%02d:%02d-%02d:%02d:%02d", FlightDate,
						UserStartTime[0], UserStartTime[1], UserStartTime[2],
						UserEndTime[0], UserEndTime[1], UserEndTime[2]);

	DataChanged = TRUE;
	RecomputeSpectra = TRUE;

}	/* END READDATA */

/* -------------------------------------------------------------------- */
static void FindMinMax()
{
	int		i, j, nSecs, missCnt;
	NR_TYPE	y, sum, mean, sigma;

	if (NumberDataSets == 0)
		buffer[0] = '\0';
	else
		ncattget(InputFile, Variable[SelectedVarIndex[0]]->inVarID, "units",
								(void *)buffer);

	SetYlabel(buffer);


	biggest_y = -FLT_MAX;
	smallest_y = FLT_MAX;

	for (CurrentDataSet = 0; CurrentDataSet < NumberDataSets; ++CurrentDataSet)
		{
		ncattget(InputFile, Variable[SelectedVarIndex[CurrentDataSet]]->inVarID,
				"units", (void *)VarStats[CurrentDataSet].units);

		nSecs = NumberElements[CurrentDataSet];

		missCnt = 0;
		sum = sigma = 0.0;

		VarStats[CurrentDataSet].min = FLT_MAX;
		VarStats[CurrentDataSet].max = -FLT_MAX;

		for (i = 0; i < nSecs; ++i)
			{
			if ((int)(y = data[CurrentDataSet][i]) == MISSING_VALUE)
				{
				++missCnt;
				continue;
				}

			smallest_y	= MIN(smallest_y, y);
			biggest_y	= MAX(biggest_y, y);

			VarStats[CurrentDataSet].min = MIN(VarStats[CurrentDataSet].min, y);
			VarStats[CurrentDataSet].max = MAX(VarStats[CurrentDataSet].max, y);
			sum += y;
			}

		if (nSecs == missCnt)
			{
			mean = 0.0;
			VarStats[CurrentDataSet].min = 0.0;
			VarStats[CurrentDataSet].max = 0.0;
			}
		else
			mean = sum / (nSecs - missCnt);

		for (j = 0; j < nSecs; ++j)
			if ((int)data[CurrentDataSet][j] != MISSING_VALUE)
				sigma += pow(data[CurrentDataSet][j] - mean, 2.0);

		nSecs -= missCnt;

		VarStats[CurrentDataSet].mean = mean;
		VarStats[CurrentDataSet].variance = sigma / (nSecs - 1);
		VarStats[CurrentDataSet].sigma = (float)sqrt(sigma / (nSecs - 1));
		}

	if (biggest_y == -FLT_MAX && smallest_y == FLT_MAX)
		{
		smallest_y = 0.0;
		biggest_y = 1.0;
		}

}	/* END FINDMINMAX */

/* -------------------------------------------------------------------- */
void AddVariable(int indx)
{
	SelectedVarIndex[NumberDataSets++] = indx;
	data[NumberDataSets] = NULL;
	ReadData(NULL, NULL, (XtPointer)NumberDataSets - 1);
	AutoScale(SCALE_YAXIS);

}	/* END ADDVARAIBLE */

/* -------------------------------------------------------------------- */
void ReduceData(int start, int newNumberSeconds)
{
	int		i, rate;
	int		hours, mins, sex;

	for (CurrentDataSet = 0; CurrentDataSet < NumberDataSets; ++CurrentDataSet)
		{
		rate = NumberElements[CurrentDataSet] / NumberSeconds;
		NumberElements[CurrentDataSet] = newNumberSeconds * rate;

		for (i = 0; i < newNumberSeconds * rate; ++i)
			data[CurrentDataSet][i] = data[CurrentDataSet][i + (start*rate)];

		data[CurrentDataSet] = realloc(data[CurrentDataSet],
								newNumberSeconds * rate * sizeof(NR_TYPE));
		}

	NumberSeconds = newNumberSeconds;


	/* Increment User Start Time accordingly.
	 */
	hours = start / 3600; start -= hours * 3600;
	mins = start / 60; start -= mins * 60;
	sex = start;

	if ((UserStartTime[2] += sex) > 59) {
		UserStartTime[2] -= 60;
		++UserStartTime[1];
		}

	if ((UserStartTime[1] += mins) > 59) {
		UserStartTime[1] -= 60;
		++UserStartTime[0];
		}

	UserStartTime[0] += hours;


	/* Set User End Time accordingly.
	 */
	hours = newNumberSeconds / 3600; newNumberSeconds -= hours * 3600;
	mins = newNumberSeconds / 60; newNumberSeconds -= mins * 60;
	sex = newNumberSeconds;

	UserEndTime[0] = UserStartTime[0] + hours;
	UserEndTime[1] = UserStartTime[1] + mins;
	UserEndTime[2] = UserStartTime[2] + sex;

	if (UserEndTime[2] > 59) {
		UserEndTime[2] -= 60;
		++UserEndTime[1];
		}

	if (UserEndTime[1] > 59) {
		UserEndTime[1] -= 60;
		++UserEndTime[0];
		}


	SetSubtitle();
	DataChanged = TRUE;
	RecomputeSpectra = TRUE;
	FindMinMax();

}	/* END REDUCEDATA */

/* -------------------------------------------------------------------- */
void DeleteVariable(int indx)
{
	int		i;

	for (i = 0; i < NumberDataSets; ++i)
		if (SelectedVarIndex[i] == indx)
			{
			free(data[i]);

			for (++i; i < NumberDataSets; ++i)
				{
				data[i-1] = data[i];
				SelectedVarIndex[i-1] = SelectedVarIndex[i];
				}

			--NumberDataSets;

			break;
			}

	data[NumberDataSets] = NULL;

	FindMinMax();
	DataChanged = TRUE;
	RecomputeSpectra = TRUE;

}	/* END DELETEVARIABLE */

/* -------------------------------------------------------------------- */
static void free_data()
{
	register int	set;

	for (set = 0; set < NumberDataSets; ++set)
		free((char *)data[set]);

}	/* END FREE_DATA */

/* END DATAIO.C */
