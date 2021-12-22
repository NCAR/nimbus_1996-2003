/*
-------------------------------------------------------------------------
OBJECT NAME:	init.c

FULL NAME:		Initialize  Procedures

TYPE:			X11/R4 application, motif

DESCRIPTION:	Inializiation routines run once at the beginning of main
				Initialize is used to set all global variables.
				Process_args does just that.

AUTHOR:			websterc@ncar
-------------------------------------------------------------------------
*/

#include "define.h"


/* --------------------------------------------------------------------- */
Initialize()
{
	register	i;
	char		*p;

	parms_file = data_file = out_file = timeSeg = NULL;

	Interactive = TRUE;
	ScatterPlot = FALSE;
	WindBarbs	= FALSE;

	p = (char *)getenv("DATA_DIR");
	strcpy(DataPath, p);
	strcat(DataPath, "/*.cdf");

	p = (char *)getenv("HOME");
	strcpy(UserPath, p);
	strcat(UserPath, "/nimbus/*");

	title[0] = subtitle[0] = xlabel[0] = ylabel[0] = '\0';

	vpX = vpY = vpUI = vpVI = NULL;
	data[0] = data[1] = uic = vic = NULL;

	numtics = NUMTICMARKS;
	Grid = FALSE;

}	/* END INITIALIZE */

/* --------------------------------------------------------------------- */
process_args(argv)
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

				case 't':	/* TimeSeg		*/
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

}	/* END PROCESS_ARGS */

/* END INIT.C */
