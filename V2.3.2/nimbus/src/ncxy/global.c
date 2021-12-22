/*
-------------------------------------------------------------------------
OBJECT NAME:	global.c

FULL NAME:		Global Variable Definitions

DESCRIPTION:	extern.h should look just like this.
-------------------------------------------------------------------------
*/

#include "define.h"

bool	Interactive;		/* Interactive or batch mode*/
bool	Grid;				/* Overlay grid on graph?	*/
bool	ScatterPlot;
bool	WindBarbs;

char	buffer[BUFFSIZE], *parms_file, *data_file, *out_file, DataPath[1024];
char	*ProjectNumber, *ProjectName, *FlightNumber, UserPath[1024],*FlightDate,
		*timeSeg;

int		nVariables;
VARTBL	*Variable[MAX_VARIABLES], *vpX, *vpY, *vpUI, *vpVI;
STAT	VarStats[MAX_VARIABLES];


/* Vars related to data sets	*/
int		NumberSeconds, NumberElements[MAXDATASETS];

NR_TYPE	*data[MAXDATASETS], *uic, *vic;

/* Time stuff */
int		FileStartTime[3], FileEndTime[3];
int		UserStartTime[3], UserEndTime[3];

/* Parameter File Variables	*/
char	title[TITLESIZE], subtitle[TITLESIZE], xlabel[TITLESIZE],
		ylabel[TITLESIZE];

char    *kw_title = "title", *kw_subtitle = "subtitle", *kw_xlabel = "xlabel",
		*kw_ylabel = "ylabel", *kw_xmin = "xmin", *kw_xmax = "xmax",
		*kw_ymin = "ymin", *kw_ymax = "ymax", *kw_legend = "legend";

/* Scaling & tic mark stuff	*/
NR_TYPE	xmin, xmax, ymin, ymax;
int		numtics;

/* Error messages	*/
char	*o_mem = "Out of Memory.";

/* X vars	*/
Widget	canvas, varListX, varListY;

instanceRec	iv;

/* END GLOBAL.C */
