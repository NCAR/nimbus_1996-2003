/*
-------------------------------------------------------------------------
OBJECT NAME:	global.c

FULL NAME:	Global Variable Definitions

DESCRIPTION:	extern.h should look just like this.
-------------------------------------------------------------------------
*/

#include "define.h"

bool	Interactive;		/* Interactive or batch mode*/
bool	Grid;				/* Overlay grid on graph?	*/
bool	AutoScaling;		/* Perform Auto Scaling?	*/
bool	DataChanged;		/* New data was read from file.	*/
bool	YaxisRescaled;
bool	RecomputeSpectra;
bool	AsciiWinOpen;
bool	DiffWinOpen;
bool	SpecWinOpen;

char	buffer[BUFFSIZE], *parms_file, *data_file, *out_file, DataPath[1024],
		*timeSeg, *ProjectNumber, *ProjectName, *FlightNumber, UserPath[1024],
		*FlightDate;

int		nVariables;
VARTBL	*Variable[MAX_VARIABLES];
STAT	VarStats[MAX_VARIABLES];


/* Vars related to data sets	*/
int		NumberDataSets, NumberSeconds, NumberElements[MAXDATASETS],
		CurrentDataSet, SelectedVarIndex[MAXDATASETS];

NR_TYPE	*data[MAXDATASETS];

/* Time stuff */
int		FileStartTime[3], FileEndTime[3];
int		UserStartTime[3], UserEndTime[3];

/* Parameter File Variables	*/
char	title[TITLESIZE], subtitle[TITLESIZE], xlabel[TITLESIZE],
		ylabel[TITLESIZE], diffYlabel[TITLESIZE], asciiFormat[10],
		lpCommand[TITLESIZE];

int		nASCIIpoints;
char    *kw_title = "title", *kw_subtitle = "subtitle", *kw_xlabel = "xlabel",
		*kw_ylabel = "ylabel", *kw_xmin = "xmin", *kw_xmax = "xmax",
		*kw_ymin = "ymin", *kw_ymax = "ymax", *kw_legend = "legend";

/* Scaling & tic mark stuff	*/
NR_TYPE	ymin, ymax, smallest_y, biggest_y;
int		numtics;

/* X vars	*/
Widget	canvas, varList;

instanceRec	iv;

/* END GLOBAL.C */
