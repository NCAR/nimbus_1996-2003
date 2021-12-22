/*
-------------------------------------------------------------------------
OBJECT NAME:	ncplot.c

FULL NAME:		eXtra Simple Graphs

TYPE:			X11/R5 Motif 1.2

DESCRIPTION:	See man page

INPUT:			Command line options

OUTPUT:			Wow!
-------------------------------------------------------------------------
*/

#include "define.h"

#define APP_NAME	"ncplot"
#define APP_CLASS	"XmNCplot"


Widget	AppShell;
Widget	Shell000, MainWindow;
Widget	Shell001, Shell003;

Widget	CreateMainWindow();
void	CreateParmsWindow();
void	CreateSpecParmsWindow();


static XtResource resources[] = {
	{XtNtitleFont, XtCTitleFont, XtRString, sizeof(char *),
	 XtOffsetOf(instanceRec, titleFont), XtRString, NULL},

	{XtNsubtitleFont, XtCSubtitleFont, XtRString, sizeof(char *),
	 XtOffsetOf(instanceRec, subtitleFont), XtRString, NULL},

	{XtNticFont, XtCTicFont, XtRString, sizeof(char *),
	 XtOffsetOf(instanceRec, ticFont), XtRString, NULL},
	};


/* --------------------------------------------------------------------- */
main(argc, argv)
int	argc;
char	*argv[];
{
	int				n;
	Arg				args[6];
	XtAppContext	app_con;

	Initialize();
	processArgs(argv);

	if (parms_file)
		ReadParmsFile((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);


	AutoScale(SCALE_YAXIS);

	if (!Interactive)
		PrintPostScript((Widget)NULL,(XtPointer)NULL,(XtPointer)NULL);
	else
		{
		AppShell = XtAppInitialize(&app_con, APP_CLASS, NULL, 0, &argc, argv,
															NULL, NULL, 0);

		XtGetApplicationResources(AppShell, (caddr_t) &iv, resources,
						XtNumber(resources), NULL, 0);

		n = 0;
		Shell000 = XtCreatePopupShell("topLevelShell",
								topLevelShellWidgetClass, AppShell, args, n);

		MainWindow = CreateMainWindow(Shell000);

		n = 0;
		Shell001 = XtCreatePopupShell("editShell",
								topLevelShellWidgetClass, AppShell, args, n);

		CreateParmsWindow(Shell001);

		n = 0;
		Shell003 = XtCreatePopupShell("editSpecShell",
								topLevelShellWidgetClass, AppShell, args, n);

		CreateSpecParmsWindow(Shell003);


		CreateErrorBox(AppShell);
		CreateQueryBox(AppShell);
		CreateWarningBox(AppShell);
		CreateFileSelectionBox(AppShell);

		if (data_file)
			NewDataFile((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

		XtManageChild(MainWindow);
		XtPopup(XtParent(MainWindow), XtGrabNone);

		XtAppMainLoop(app_con);
		}

	return(0);

}	/* END MAIN */

/* END NCPLOT.C */
