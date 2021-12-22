/*
-------------------------------------------------------------------------
OBJECT NAME:	ncrt.c

FULL NAME:		Real Time High Rate

TYPE:			X11/R5 Motif 1.2

DESCRIPTION:	See man page

INPUT:			Command line options

OUTPUT:			Wow!
-------------------------------------------------------------------------
*/

#include "define.h"

#define APP_NAME	"rtplot"
#define APP_CLASS	"XmNCrt"


Widget	AppShell, Shell000, Shell001;
Widget	MainWindow, ParmsWindow;
XtAppContext	app_con;

Widget	CreateMainWindow();


static XtResource resources[] = {
	{XtNtitleFont, XtCTitleFont, XtRString, sizeof(char *),
	 XtOffsetOf(instanceRec, titleFont), XtRString, NULL},

	{XtNsubtitleFont, XtCSubtitleFont, XtRString, sizeof(char *),
	 XtOffsetOf(instanceRec, subtitleFont), XtRString, NULL},

	{XtNticFont, XtCTicFont, XtRString, sizeof(char *),
	 XtOffsetOf(instanceRec, ticFont), XtRString, NULL},
	};

void CatchAlarm();

/* --------------------------------------------------------------------- */
main(argc, argv)
int	argc;
char	*argv[];
{
	int				n;
	Arg				args[6];

	if (argc < 3)
		{
		fprintf(stderr, "Usage: %s -p PrjNum [-s NumSeconds]\n", argv[0]);
		exit(1);
		}


	Initialize();
	process_args(argv);

	sprintf(buffer, "%s/%s/header", ProjectDirectory, ProjectNumber);
	DecodeHeader(buffer);
	AllocateDataArrays(); printf("Header decoded\n"); fflush(stdout);

	AppShell = XtAppInitialize(&app_con, APP_CLASS, NULL, 0, &argc, argv,
															NULL, NULL, 0);

	XtGetApplicationResources(AppShell, (caddr_t) &iv, resources,
						XtNumber(resources), NULL, 0);

	n = 0;
	Shell000 = XtCreatePopupShell("topLevelShell",
								topLevelShellWidgetClass, AppShell, args, n);

	MainWindow = CreateMainWindow(Shell000);

	CreateErrorBox(AppShell);
	CreateWarningBox(AppShell);

	XtManageChild(MainWindow);
	XtPopup(XtParent(MainWindow), XtGrabNone);

	AttachSharedMemory(); printf("Attached shmem\n"); fflush(stdout);
	PlotData(NULL, NULL, NULL);

	XtAppAddTimeOut(app_con, 100, CatchAlarm, NULL);
	XtAppMainLoop(app_con);

	return(0);

}	/* END MAIN */

/* END NCRT.C */
