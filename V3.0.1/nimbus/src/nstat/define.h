/*
-------------------------------------------------------------------------
OBJECT NAME:	define.h

DESCRIPTION:	Header File for Nimbus Skeleton.
-------------------------------------------------------------------------
*/

#ifndef DEFINE_H
#define DEFINE_H

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <Xm/Xm.h>

#include "constants.h"

#define MISSING_VALUE	(-32767.0)

#define NAMELEN			NAMLEN
#define MAX_VARIABLES	2000
#define MAX_TIME_SLICES	5


/* Values for "VariableType"	*/
#define SDI				0
#define RAW				1
#define DERIVED			2


/* PauseWhatToDo values.		*/
#define P_QUIT			0
#define P_CONTINUE		1


#define BEG_OF_TAPE		(-1)
#define END_OF_TAPE		(-1)

/* OpenProjectFile action values	*/
#define RETURN			0
#define EXIT			1


#define LOW_RATE	1
#define HIGH_RATE	25


/* Nimbus Record Info		*/
#define NR_TYPE float
#define NR_SIZE sizeof(NR_TYPE)


typedef struct
	{
	char	name[NAMELEN];
	int		inVarID;		/* netCDF variable ID		*/
	char	Units[16];
	int		SampleRate;
	int		OutputRate;
	int		VectorLength;
	bool	Output;
	} VARTBL;


/*		Global Variables
 */
extern char		*ProjectNumber, *ProjectName;
extern char		*Aircraft, *FlightNumber, *FlightDate, *Defaults;
extern VARTBL	*Variable[];
extern bool		PauseFlag, Interactive;
extern int		nVariables, InputFile, PauseWhatToDo, DefaultSeconds;
extern char		buffer[];
extern long		nRecords, CurrentInputRecordNumber;


/* Setup window widgets */
extern Widget	Shell001;
extern Widget	MainWindow, SetupWindow, EditWindow, TimeSliceWindow;
extern Widget	inputFileText, outputFileText;
extern Widget	timeDisplayText, outputText;
extern Widget	list1, goButton, menuBar, readHeaderButton;
extern Widget	ts_text[];


double	atof();

/*		Local Functions
 */
char	*SearchList(), *GetMemory();

void	CloseNetCDF(), FlushXEvents(), GetUserTimeIntervals(), Initialize(),
		Output(), HandleError(), HandleWarning();

/* Callbacks	*/
void	CancelWarning(), CancelSetup(), EditTimeSlices(),
		DismissEditWindow(), EditVariable(), Quit(), ReadHeader(),
		StartProcessing(), ValidateTime(), Proceed(), ToggleOutput(),
		PauseProcessing(), PrintText(), ResetTimeSliceWindow(),
		PauseStop(), PauseContinue(), DismissTimeSliceWindow();


#endif

/* END DEFINE.H */
