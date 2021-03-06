/*
-------------------------------------------------------------------------
OBJECT NAME:	globals.c

FULL NAME:		Global Variable Definitions

ENTRY POINTS:	none

DESCRIPTION:	

INPUT:			none

OUTPUT:			none

REFERENCES:		none

REFERENCED BY:	none

COPYRIGHT:		University Corporation for Atmospheric Research, 1993
-------------------------------------------------------------------------
*/

#include "define.h"

char	buffer[4096];			/* Generic, volatile string space		*/
char	*ProjectNumber, *ProjectName, *DefaultFormat;
char	*Aircraft, *FlightNumber, *FlightDate, *Defaults;

VARTBL  *Variable[MAX_VARIABLES];

int		nVariables;
bool	PauseFlag, Interactive;
int		InputFile, PauseWhatToDo, XaxisType;
FILE	*OutputFile;
long	nRecords, CurrentInputRecordNumber;

/* END GLOBALS.C */
