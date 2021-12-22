/*
-------------------------------------------------------------------------
OBJECT NAME:	global.c

FULL NAME:		Global Variable Definitions

DESCRIPTION:	extern.h should look just like this.
-------------------------------------------------------------------------
*/

#include "define.h"

bool	Grid,				/* Overlay grid on graph?	*/
		Frozen,
		FlashMode;

char	buffer[BUFFSIZE], DataPath[1024], *ProjectDirectory,
		*ProjectNumber, *ProjectName;

SDITBL	*sdi[MAX_SDI];			/* SDI (aka raw) variable list	*/
RAWTBL	*raw[MAX_RAW];			/* Alphabeticly sorted pointers	*/


int		nsdi, nraw, nVariables;
VARTBL	Variable[MAXDATASETS];


/* Vars related to data sets	*/
int		NumberDataSets, NumberSeconds, NumberElements[MAXDATASETS],
		CurrentDataSet, Aircraft, FlightNumber, Units;

long	nFloats;

/* Data record pointers
 */
char	*ADSrecord, *AVAPSrecord[4];
ushort	*bits;
NR_TYPE	*volts;
NR_TYPE	*SampledData, *AveragedData;
NR_TYPE	*plotData[MAXDATASETS];


/* Parameter File Variables	*/
char	title[TITLESIZE], subtitle[TITLESIZE], xlabel[TITLESIZE],
		ylabel[TITLESIZE];

char    *kw_title = "title", *kw_subtitle = "subtitle", *kw_xlabel = "xlabel",
		*kw_ylabel = "ylabel", *kw_xmin = "xmin", *kw_xmax = "xmax",
		*kw_ymin = "ymin", *kw_ymax = "ymax", *kw_legend = "legend";

/* Scaling & tic mark stuff	*/
NR_TYPE	ymin, ymax, smallest_y, biggest_y;
int		numtics;

/* Error messages	*/
char	*o_mem = "Out of Memory.";

/* X vars	*/
Widget	canvas, varList;

instanceRec	iv;

/* END GLOBAL.C */
