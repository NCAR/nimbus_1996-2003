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
#include "shmem.h"
#include "eth_shm.h"

#include <sys/utsname.h>

extern struct SUN_ETH_SHM *eth_shm;     /* ethernet shared mem adr */
extern int read_static_ids(char *ComputeHost);
extern int getaddr_static();
extern int getaddr_dynamic();
extern void read_dynamic_ids(char *prog_name, char *ComputeHost);


/* --------------------------------------------------------------------- */
void Initialize()
{
	register	i;
	char		*p;

	ProjectDirectory = (char *)getenv("PROJ_DIR");
	winds_path = (char *)getenv("WINDS");

	title[0] = subtitle[0] = xlabel[0] = ylabel[0] = '\0';
	strcpy(xlabel, "Time");
	strcpy(ylabel, "Engineering");

	numtics = NUMTICMARKS;
	Counts = FlashMode = Grid = Frozen = FALSE;
	NumberSeconds = 10;

}	/* END INITIALIZE */

/* --------------------------------------------------------------------- */
AttachSharedMemory()
{
	struct utsname	name;

	uname(&name);

	if (!read_static_ids(name.nodename))
		{
		(void)fprintf(stderr, "ncrt: read_static_ids returned ERROR.\n");
		exit(1);
		}

	if (getaddr_static() == ERROR)
		{
		(void)fprintf(stderr, "ncrt: getaddr_static returned ERROR.\n");
		exit(1);
		}

	read_dynamic_ids("NCRT", name.nodename);

	if (getaddr_dynamic() == ERROR)
		{
		(void)fprintf(stderr, "ncrt: getaddr_dynamic returned ERROR.\n");
		exit(1);
		}

	ADSrecord = (char *)eth_shm->syncBuf;

}	/* END ATTACHSHAREDMEMORY */

/* --------------------------------------------------------------------- */
NR_TYPE GetDefaultsValue()
{
	return(0.0);
}

/* --------------------------------------------------------------------- */
process_args(argv)
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
