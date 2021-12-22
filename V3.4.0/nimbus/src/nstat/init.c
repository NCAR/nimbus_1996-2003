/*
-------------------------------------------------------------------------
OBJECT NAME:	init.c

FULL NAME:		Initialize Global Variables

ENTRY POINTS:	Initialize()
				ProcessArgv()
				ReadFormatFile()

STATIC FNS:		

DESCRIPTION:	

INPUT:			none

OUTPUT:			none

REFERENCES:		none

REFERENCED BY:	nimbus.c, cb_main.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1993
-------------------------------------------------------------------------
*/

#include <Xm/TextF.h>

#include "define.h"

char	*getenv();


/* -------------------------------------------------------------------- */
void Initialize()
{
	int		pos;

	PauseFlag		= FALSE;
	PauseWhatToDo	= P_CONTINUE;
	DefaultSeconds	= 900;

	pos = XmTextFieldGetLastPosition(inputFileText);
	XmTextFieldSetInsertionPosition(inputFileText, pos);

	XtSetSensitive(goButton, TRUE);

}	/* END INITIALIZE */

/* -------------------------------------------------------------------- */
void ProcessArgv(argc, argv)
int		argc;
char	**argv;
{
	int		i;

	Interactive = TRUE;

	for (i = 1; i < argc; ++i)
		{
		if (argv[i][0] != '-')
			{
			fprintf(stderr, "Invalid option %s, ignoring.\n", argv[i]);
			continue;
			}

		switch (argv[i][1])
			{
			case 's':
				DefaultSeconds = atoi(argv[++i]);
				break;

			default:
				fprintf(stderr, "Invalid option %s, ignoring.\n", argv[i]);
			}

		}

}	/* END PROCESSARGV */

/* END INIT.C */
