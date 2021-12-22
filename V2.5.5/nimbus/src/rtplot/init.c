/*
-------------------------------------------------------------------------
OBJECT NAME:	init.c

FULL NAME:	Initialize  Procedures

TYPE:		X11/R5 application, motif

DESCRIPTION:	Inializiation routines run once at the beginning of main
		Initialize is used to set all global variables.
		Process_args does just that.

AUTHOR:		websterc@ncar.ucar.edu
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
  bool	haveHost = False, haveProj = False;

  while (*++argv)
    if ((*argv)[0] == '-')
      switch ((*argv)[1])
        {
        case 'h':   /* Host to attach to	*/
          if (*++argv == NULL)
            return;

          strcpy(HostName, *argv);
          haveHost = True;
          break;

        case 'p':   /* Project Number		*/
          if (*++argv == NULL)
            return;

          ProjectNumber = GetMemory((unsigned)strlen(*argv)+1);

          strcpy(ProjectNumber, *argv);
          haveProj = True;
          break;

        case 's':   /* Number of seconds	*/
          if (*++argv == NULL)
            return;

          NumberSeconds = atoi(*argv);
          break;

        default:
          fprintf(stderr, "Invalid option, Usage:\n");
          fprintf(stderr, "  rtplot -h host -p PrjNum [-s Seconds]\n");
        }

  if (haveHost == False)
    {
    fprintf(stderr, "rtplot: host to attach to required.\n");
    fprintf(stderr, "Usage:  rtplot -h host -p PrjNum [-s Seconds]\n");
    exit(1);
    }

  if (haveProj == False)
    {
    fprintf(stderr, "rtplot: project number required.\n");
    fprintf(stderr, "Usage:  rtplot -h host -p PrjNum [-s Seconds]\n");
    exit(1);
    }

}	/* END PROCESS_ARGS */

/* END INIT.C */
