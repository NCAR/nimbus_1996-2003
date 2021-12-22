/*
-------------------------------------------------------------------------
OBJECT NAME:	init.c

FULL NAME:		Initialize  Procedures

TYPE:			X11/R4 application, motif

DESCRIPTION:	Inializiation routines run once at the beginning of main
				Initialize is used to set all global variables.
				Process_args does just that.

AUTHOR:			websterc@ncar.ucar.edu
-------------------------------------------------------------------------
*/

#include "define.h"

/* --------------------------------------------------------------------- */
void Initialize()
{
	ProjectDirectory = (char *)getenv("PROJ_DIR");

	title[0] = subtitle[0] = xlabel[0] = ylabel[0] = '\0';
	strcpy(xlabel, "Time");

	numtics = NUMTICMARKS;
	FlashMode = Grid = Frozen = FALSE;
	NumberSeconds = 10;

}	/* END INITIALIZE */

/* --------------------------------------------------------------------- */
NR_TYPE GetDefaultsValue()
{
	return(0.0);
}

/* --------------------------------------------------------------------- */
void process_args(argv)
char	**argv;
{

	while (*++argv)
		if ((*argv)[0] == '-')
			switch ((*argv)[1])
				{
				case 'p':   /* Project Number		*/
					if (*++argv == NULL)
						return;

					ProjectNumber = GetMemory((unsigned)strlen(*argv)+1);

					strcpy(ProjectNumber, *argv);
					break;

				case 's':   /* Number of seconds	*/
					if (*++argv == NULL)
						return;

					NumberSeconds = atoi(*argv);
					break;

				default:
					fprintf(stderr, "Invalid option, Usage:\n");
					fprintf(stderr, "ncrt -p PrjNum [-s Seconds]\n");
				}

}	/* END PROCESS_ARGS */

/* END INIT.C */
