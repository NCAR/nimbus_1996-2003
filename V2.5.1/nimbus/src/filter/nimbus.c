/*
-------------------------------------------------------------------------
OBJECT NAME:	nimbus.c

FULL NAME:	

ENTRY POINTS:	main()

STATIC FNS:		none

DESCRIPTION:	

INPUT:		

OUTPUT:		

REFERENCES:		Xwin.c, init.c then enters XtAppMainLoop()

REFERENCED BY:	user

COPYRIGHT:		University Corporation for Atmospheric Research, 1993
-------------------------------------------------------------------------
*/

#include "nimbus.h"
#include "gui.h"

#define APP_NAME	"nimbus"
#define APP_CLASS	"XmNimbus"


Widget	AppShell;				/* The Main Application Shell */
Widget	Shell000, MainWindow;
Widget	Shell001, SetupWindow;
Widget	Shell002, EditWindow;
Widget	Shell003, TimeSliceWindow;

XtAppContext context;


Widget CreateMainWindow(Widget parent);
Widget CreateSetupWindow(Widget parent);
Widget CreateEditWindow(Widget parent);
Widget CreateTimeSliceWindow(Widget parent);


/* -------------------------------------------------------------------- */
main(int argc, char **argv)
{
	Arg			args[8];
	Cardinal	n;

	n = 0;
	AppShell = XtAppInitialize(&context, APP_CLASS, NULL, 0, &argc, argv,
															NULL, NULL, 0);

	n = 0;
	Shell000 = XtCreatePopupShell("topLevelShell", 
							topLevelShellWidgetClass, AppShell, args, n);

	MainWindow = CreateMainWindow(Shell000);

	n = 0;
	XtSetArg(args[n], XmNiconName, "Nimbus"); n++;
	Shell001 = XtCreatePopupShell("setupShell", 
							topLevelShellWidgetClass, AppShell, args, n);

	SetupWindow = CreateSetupWindow(Shell001);

	n = 0;
	XtSetArg(args[n], XmNtitle, "Edit Variable"); n++;
	XtSetArg(args[n], XmNiconName, "VarEdit"); n++;
	XtSetArg(args[n], XmNtransientFor, Shell001); n++;
	Shell002 = XtCreatePopupShell("editVariableShell", 
							transientShellWidgetClass, AppShell, args, n);

	EditWindow = CreateEditWindow(Shell002);

	n = 0;
	XtSetArg(args[n], XmNtitle, "Edit Time Slices"); n++;
	XtSetArg(args[n], XmNtransientFor, Shell001); n++;
	Shell003 = XtCreatePopupShell("timeSliceShell",
		transientShellWidgetClass, AppShell, args, n);

	TimeSliceWindow = CreateTimeSliceWindow(Shell003);


	CreatePauseWindows(AppShell);
	CreateErrorBox(AppShell);
	CreateQueryBox(AppShell);
	CreateWarningBox(AppShell);
	CreateFileSelectionBox(AppShell);

	Initialize();
	ProcessArgv(argc, argv);

	if (Interactive)
		{
		XtManageChild(MainWindow);
		XtPopup(XtParent(MainWindow), XtGrabNone);

		XtAppMainLoop(context);
		}
	else
		{
		Proceed(NULL, NULL, NULL);
		StartProcessing(NULL, NULL, NULL);
		}

	return(0);

}	/* END MAIN */

/* END NIMBUS.C */
