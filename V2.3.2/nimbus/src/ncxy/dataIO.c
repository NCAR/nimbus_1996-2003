/*
-------------------------------------------------------------------------
OBJECT NAME:	dataIO.c

FULL NAME:		Data File IO

ENTRY POINTS:	NewDataFile()
				ReadData()
				AddVariable()
				DeleteVariable()
				FindMinMax()

STATIC FNS:		freeData()

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

static NR_TYPE	*loadData();
static void		freeData();

/* -------------------------------------------------------------------- */
void NewDataFile(w, clientData, callData)
Widget		w;
XtPointer	clientData;
XtPointer	callData;
{
	VARTBL		*vp;
	XmString	item[MAX_VARIABLES];
	int			i, indx, len, nVars, nDims, dimIDs[3];
	char		name[NAMLEN*2];


	if (nVariables > 0)
		{
		ncclose(InputFile);
		freeData(XI | YI | UI | VI);

		free(ProjectName);
		free(ProjectNumber);
		free(FlightNumber);

		for (i = 0; i < nVariables; ++i)
			free((char *)Variable[i]);

		nVariables = 0;
		XmListDeleteAllItems(varListX);
		XmListDeleteAllItems(varListY);

		vpX = vpY = vpUI = vpVI = NULL;
		title[0] = subtitle[0] = xlabel[0] = ylabel[0] = '\0';
		}
	else
		ncopts = 0;


	/* If this was called from the menu then get the file name
	 * already selected.
	 */
	if (callData)
		{
		FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

		ExtractFileName(((XmFileSelectionBoxCallbackStruct *)
			callData)->value, &data_file);
		}


	/* Open Input File
	 */
	if ((InputFile = ncopen(data_file, NC_NOWRITE)) == ERR)
		{
		sprintf(buffer, "Can't open %s.\n", data_file);
		HandleError(buffer);
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
	NumberSeconds = 3600;
	memcpy((char *)UserStartTime, (char *)FileStartTime, 3 * sizeof(int));
	memcpy((char *)UserEndTime, (char *)FileStartTime, 3 * sizeof(int));

	++UserEndTime[0];

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


	sprintf(subtitle, "%s, %02d:%02d:%02d-%02d:%02d:%02d", FlightDate,
						UserStartTime[0], UserStartTime[1], UserStartTime[2],
						UserEndTime[0], UserEndTime[1], UserEndTime[2]);

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
			ncattget(InputFile, i, "OutputRate", (void *)&vp->OutputRate);

		vp->inVarID	= i;
		}

	SortTable(Variable, 0, nVariables - 1);

	if ((indx = SearchTable(Variable, nVariables - 1, "XUIC")) == ERR)
		if ((indx = SearchTable(Variable, nVariables - 1, "UIC")) == ERR)
			if ((indx = SearchTable(Variable, nVariables - 1, "UI")) == ERR)
				fprintf(stderr, "Missing wind component, barbs not available.\n");

	if (indx != ERR)
		vpUI = Variable[indx];

	if ((indx = SearchTable(Variable, nVariables - 1, "XVIC")) == ERR)
		if ((indx = SearchTable(Variable, nVariables - 1, "VIC")) == ERR)
			if ((indx = SearchTable(Variable, nVariables - 1, "VI")) == ERR)
				fprintf(stderr, "Missing wind component, barbs not available.\n");

	if (indx != ERR)
		vpVI = Variable[indx];


	if (!Interactive)
		return;

	for (i = 0; i < nVariables; ++i)
		item[i] = XmStringCreateLocalized(Variable[i]->name);

	XmListAddItems(varListX, item, nVariables, 1);
	XmListAddItems(varListY, item, nVariables, 1);

	for (i = 0; i < nVariables; ++i)
		XtFree(item[i]);

}	/* END NEWDATAFILE */

/* -------------------------------------------------------------------- */
void ReadData(axies)
int	axies;
{
	long	startTime, endTime;
	long	fileStartTime, fileEndTime;

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
		UserStartTime[2] = startTime-UserStartTime[0]*3600-UserStartTime[1]*60;
		}

	if (endTime > fileEndTime)
		{
		endTime = fileEndTime;

		UserEndTime[0] = endTime / 3600;
		UserEndTime[1] = (endTime - (UserEndTime[0]*3600)) / 60;
		UserEndTime[2] = endTime - UserEndTime[0]*3600 - UserEndTime[1]*60;
		}

	NumberSeconds = endTime - startTime;


	/* Read in data	*/
	if (vpX && axies & XI)
		{
		printf("Reading vpX\n");
		freeData(XI);
		data[0] = loadData(vpX, fileStartTime, startTime);
		NumberElements[0] = vpX->nPoints;
		}

	if (vpY && axies & YI)
		{
		printf("Reading vpY\n");
		freeData(YI);
		data[1] = loadData(vpY, fileStartTime, startTime);
		NumberElements[1] = vpY->nPoints;
		}

	if (WindBarbs)
		{
		if (vpUI && axies & UI)
			{
			printf("Reading vpUI\n");
			freeData(UI);
			uic = loadData(vpUI, fileStartTime, startTime);
			}

		if (vpVI && axies & VI)
			{
			printf("Reading vpVI\n");
			freeData(VI);
			vic = loadData(vpVI, fileStartTime, startTime);
			}
		}

printf("vpX=%d, vpY=%d, vpUI=%d, vpVI=%d, X=%d, Y=%d, uic=%d, vic=%d\n",
	vpX, vpY, vpUI, vpVI, data[X], data[Y], uic, vic);

	if (data[X] && data[Y])
		FindMinMax();

	sprintf(subtitle, "%s, %02d:%02d:%02d-%02d:%02d:%02d", FlightDate,
						UserStartTime[0], UserStartTime[1], UserStartTime[2],
						UserEndTime[0], UserEndTime[1], UserEndTime[2]);

}	/* END READDATA */

/* -------------------------------------------------------------------- */
static NR_TYPE *loadData(vp, fileStartTime, startTime)
VARTBL	*vp;
long	fileStartTime;
long	startTime;
{
	NR_TYPE	*dp;
	long	start[2], count[2];

	vp->nPoints = NumberSeconds * vp->OutputRate;

	dp = (NR_TYPE *)GetMemory(vp->nPoints * sizeof(NR_TYPE));

	start[0] = startTime - fileStartTime;
	start[1] = 0;

	count[0] = NumberSeconds;
	count[1] = vp->OutputRate;

	ncvarget(InputFile, vp->inVarID, start, count, (void *)dp);

	return(dp);

}	/* END LOADDATA */

/* -------------------------------------------------------------------- */
void FindMinMax()
{
	int		CurrentDataSet, i, j, nSecs, missCnt;
	NR_TYPE	y, sum, mean, sigma;
	char	temp[24];

	if (vpX == NULL || vpY == NULL)
		return;

	ncattget(InputFile, vpX->inVarID, "units", (void *)temp);
	sprintf(xlabel, "%s (%s)", vpX->name, temp);
	ncattget(InputFile, vpY->inVarID, "units", (void *)temp);
	sprintf(ylabel, "%s (%s)", vpY->name, temp);


	for (CurrentDataSet = 0; CurrentDataSet < 2; ++CurrentDataSet)
		{
		nSecs = NumberElements[CurrentDataSet];

		missCnt = 0;
		sum = sigma = 0.0;

		VarStats[CurrentDataSet].min = FLT_MAX;
		VarStats[CurrentDataSet].max = -FLT_MAX;

		for (i = 0; i < nSecs; ++i)
			{
			if ((int)(y = data[CurrentDataSet][i]) == -32767)
				{
				++missCnt;
				continue;
				}

			y = data[CurrentDataSet][i];

			VarStats[CurrentDataSet].min = MIN(VarStats[CurrentDataSet].min, y);
			VarStats[CurrentDataSet].max = MAX(VarStats[CurrentDataSet].max, y);
			sum += y;
			}

		mean = sum / (nSecs - missCnt);

		for (j = 0; j < nSecs; ++j)
			if ((int)data[CurrentDataSet][j] != -32767)
				sigma += pow(data[CurrentDataSet][j] - mean, 2.0);

		VarStats[CurrentDataSet].mean = mean;
		VarStats[CurrentDataSet].variance = sigma / (nSecs - 1);
		VarStats[CurrentDataSet].sigma = (float)sqrt(sigma / (nSecs - 1));
		}

}	/* END FINDMINMAX */

/* -------------------------------------------------------------------- */
static void freeData(axies)
int		axies;
{
	if (axies & XI && data[X])
		{
		free((char *)data[X]);
		data[X] = NULL;
		}

	if (axies & YI && data[Y])
		{
		free((char *)data[Y]);
		data[Y] = NULL;
		}

	if (axies & UI && uic)
		{
		free((char *)uic);
		uic = NULL;
		}

	if (axies & VI && vic)
		{
		free((char *)vic);
		vic = NULL;
		}

}	/* END FREE_DATA */

/* END DATAIO.C */
