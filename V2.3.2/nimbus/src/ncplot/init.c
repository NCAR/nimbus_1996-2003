/*
-------------------------------------------------------------------------
OBJECT NAME:	init.c

FULL NAME:		Initialize Procedures

TYPE:			X11/R4 application, motif

DESCRIPTION:	Inializiation routines run once at the beginning of main
				Initialize is used to set all global variables.
				Process_args does just that.

AUTHOR:			websterc@ncar
-------------------------------------------------------------------------
*/

#include "define.h"


/* --------------------------------------------------------------------- */
void Initialize()
{
	char		*p;

	parms_file = data_file = out_file = timeSeg = NULL;
	data[0] = NULL;

	Interactive = TRUE;
	AutoScaling = TRUE;
	DataChanged = TRUE;
	AsciiWinOpen = FALSE;
	DiffWinOpen = FALSE;
	SpecWinOpen = FALSE;
	YaxisRescaled = FALSE;

	p = (char *)getenv("DATA_DIR");

	if (p)
		{
		strcpy(DataPath, p);
		strcat(DataPath, "/*.cdf");
		}
	else
		strcpy(DataPath, "/*");

	p = (char *)getenv("HOME");
	strcpy(UserPath, p);
	strcat(UserPath, "/nimbus/*");

	title[0] = subtitle[0] = xlabel[0] = ylabel[0] = diffYlabel[0] = '\0';
	strcpy(xlabel, "UTC");
	strcpy(lpCommand, "lp -o nobanner");
	strcpy(asciiFormat, "%14e");
	nASCIIpoints = 300;

	numtics = NUMTICMARKS;
	Grid = FALSE;

}	/* END INITIALIZE */

/* --------------------------------------------------------------------- */
void processArgs(argv)
char	**argv;
{
	while (*++argv)
	    if ((*argv)[0] == '-')
			switch ((*argv)[1])
				{
				case 'p':	/* Parms File		*/
					if (*++argv == NULL)
						return;

					parms_file = GetMemory((unsigned)strlen(*argv)+1);

					strcpy(parms_file, *argv);
					break;

				case 'f':	/* Output File(for PS)	*/
					if (*++argv == NULL)
						return;

					out_file = GetMemory((unsigned)strlen(*argv)+1);

					strcpy(out_file, *argv);
					break;

				case 't':	/* Time Segment	*/
					if (*++argv == NULL)
						return;

					timeSeg = GetMemory((unsigned)strlen(*argv)+1);

					strcpy(timeSeg, *argv);
					break;

				case 'h':	/* Hardcopy		*/
					Interactive = FALSE;
					break;
				}
	    else
			{
			data_file = GetMemory((unsigned)strlen(*argv)+1);

			strcpy(data_file, *argv);
			}

}	/* END PROCESSARGS */

/* END INIT.C */
