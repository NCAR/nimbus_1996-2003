/*
-------------------------------------------------------------------------
OBJECT NAME:	cb_main.c

FULL NAME:		Command CallBacks

ENTRY POINTS:	
				CancelSetup()
				Proceed()
				ReadHeader()
				StartProcessing()
				StopProcessing()
				CreateListLineItem()
				ToggleOutput()
				PrintText()
				Output()
				Quit()

STATIC FNS:		ValidateFileNames()
				FillListWidget()
				CondenseVariableList()

DESCRIPTION:	Contains callbacks for the nimbus GUI main window & setup
				window.

INPUT:			

OUTPUT:		

REFERENCES:		Everything.

REFERENCED BY:	XtAppMainLoop()

COPYRIGHT:		University Corporation for Atmospheric Research, 1993
-------------------------------------------------------------------------
*/

#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>

#include "define.h"


static char	InputFileName[MAXPATHLEN];


void		StopProcessing();
static void	FillListWidget(), CondenseVariableList();
static int	ValidateFileNames();


/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
CancelSetup(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int		i;

	close(InputFile);

	for (i = 0; i < nVariables; ++i)
		free((char *)Variable[i]);

	free(ProjectName);


	DismissTimeSliceWindow(NULL, NULL, NULL);

	XtPopdown(XtParent(SetupWindow));
	XtUnmanageChild(SetupWindow);

	XmListDeleteAllItems(list1);

	XtSetSensitive(readHeaderButton, TRUE);
	XtSetSensitive(inputFileText, TRUE);

	Initialize();

}	/* END CANCELSETUP */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
Proceed(w, client, call)
Widget		w;
XtPointer	client;
XtPointer	call;
{
	strcpy(InputFileName, XmTextFieldGetString(inputFileText));

	if (ValidateFileNames() == OK)
		ReadHeader(NULL, NULL, NULL);

}	/* END PROCEED */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
ReadHeader(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int		hdrfd;

	XtSetSensitive(readHeaderButton, FALSE);
	XtSetSensitive(inputFileText, FALSE);

	XmUpdateDisplay(Shell001);

	if (ReadInputFile(InputFileName) == ERR)
		{
		CancelSetup(NULL, NULL, NULL);
		return;
		}


	if (Interactive)
		{
		Arg		args[1];

		FillListWidget();

		sprintf(buffer, "%s - %s, Flight #%d",
				ProjectName,
				ProjectNumber,
				atoi(FlightNumber));

		XtSetArg(args[0], XmNtitle, buffer);
		XtSetValues(Shell001, args, 1);

		XtManageChild(SetupWindow);
		XtPopup(XtParent(SetupWindow), XtGrabNone);
		}

}	/* END READHEADER */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
StartProcessing(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmString	label;
	Arg			args[1];
	int			rc;
	long		btim, etim;

	DismissTimeSliceWindow(NULL, NULL, NULL);
	XtSetSensitive(list1, FALSE);
	XtSetSensitive(menuBar, FALSE);

	GetUserTimeIntervals();
	CondenseVariableList();

	FlushXEvents();


	/* Turn "Go" button into "Pause" button.
	 */
	XtRemoveAllCallbacks(goButton, XmNactivateCallback);
	label = XmStringCreate("Pause", XmFONTLIST_DEFAULT_TAG);
	XtSetArg(args[0], XmNlabelString, label);
	XtSetValues(goButton, args, 1);
	XmStringFree(label);
	XtAddCallback(goButton, XmNactivateCallback, PauseProcessing, NULL);

	FlushXEvents();


	while (NextTimeInterval(&btim, &etim))
		{
		rc = LowRateLoop(btim, etim);

		if (PauseWhatToDo == P_QUIT || rc == ERR)
			break;
		}

	StopProcessing();

}	/* END STARTPROCESSING */

/* -------------------------------------------------------------------- */
void StopProcessing()
{
	XmString	label;
	Arg			args[1];
	float		x;

	CloseNetCDF();


	/* Turn "Pause" button back into "Go" button.
	 */
	label = XmStringCreate("Go", XmFONTLIST_DEFAULT_TAG);
	XtSetArg(args[0], XmNlabelString, label);
	XtSetValues(goButton, args, 1);
	XmStringFree(label);
	XtRemoveAllCallbacks(goButton, XmNactivateCallback);
	XtAddCallback(goButton, XmNactivateCallback, StartProcessing, NULL);

	XtSetSensitive(menuBar, TRUE);
	XtSetSensitive(list1, TRUE);

	CancelSetup(NULL, NULL, NULL);

	XtPopdown(XtParent(MainWindow));
	XtUnmanageChild(MainWindow);

	XtManageChild(EditWindow);
	XtPopup(XtParent(EditWindow), XtGrabNone);

}	/* END STOPPROCESSING */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
Quit(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	exit(0);
}

/* -------------------------------------------------------------------- */
static ValidateFileNames()
{
	char	scratch[16];	/* character scratch area */
	int		proj_num;

	if (strcmp(&InputFileName[strlen(InputFileName)-4], ".cdf") != 0)
		{
		strcat(InputFileName, ".cdf");
		XmTextFieldSetString(inputFileText, InputFileName);
		}

	if (access(InputFileName, R_OK) == ERR || strlen(InputFileName) == 0)
		{
		HandleError("Non-existent input file.");
		return(ERR);
		}

	return(OK);

}	/* END VALIDATEFILENAMES */

/* -------------------------------------------------------------------- */
XmString CreateListLineItem(vp)
VARTBL	*vp;
{
	int			i;
	char		tmp[16];
	static char	*list1lineFrmt = "%-13s  %c   %4d    %4d";

	sprintf(buffer, list1lineFrmt,
				vp->name,
				vp->Output ? 'Y' : 'N',
				vp->SampleRate,
				vp->OutputRate);

	return(XmStringCreateLocalized(buffer));

}	/* END CREATELISTLINEITEM */

/* -------------------------------------------------------------------- */
static void FillListWidget()
{
	int		i, cnt;
	XmString	items[MAX_VARIABLES];

	XmListDeleteAllItems(list1);

	cnt = 0;

	for (i = 0; i < nVariables; ++i)
		items[cnt++] = CreateListLineItem(Variable[i]);


	XmListAddItems(list1, items, cnt, 1);

	for (i = 0; i < cnt; ++i)
		XmStringFree(items[i]);

}	/* END FILLLISTWIDGET */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
ToggleOutput(w, client, call)
Widget		w;
XtPointer	client;
XmListCallbackStruct *call;
{
	int			*pos_list, pos_cnt = 0;
	int			i, indx;
	XmString	new;

	XmListGetSelectedPos(list1, &pos_list, &pos_cnt);

	for (i = 0; i < pos_cnt; ++i)
		{
		indx = pos_list[i] - 1;

		Variable[indx]->Output = 1 - Variable[indx]->Output;

		new = CreateListLineItem(Variable[indx]);

		XmListReplaceItemsPos(list1, &new, 1, pos_list[i]);
/*		XmListSelectPos(list1, pos_list[i], FALSE); */
		XmStringFree(new);
		}

}	/* END TOGGLEOUTPUT */

/* -------------------------------------------------------------------- */
void PrintText(w, client, call)
Widget		w;
XtPointer	client;
XtPointer	call;
{
	FILE	*fp;
	char	*s;

	if ((fp = popen("lpr", "w")) == NULL)
		{
		HandleError("PrintText: can't open pipe to 'lpr'");
		return;
		}

	fprintf(fp, "%s", XmTextGetString(outputText));

	pclose(fp);

}	/* END PRINTTEXT */

/* -------------------------------------------------------------------- */
void Output(msg)
char	msg[];
{
	XmTextPosition	position;
	extern Widget	outputText;

	if (Interactive)
		{
		position = XmTextGetInsertionPosition(outputText);
		XmTextInsert(outputText, position, msg);

		position += strlen(msg);
		XmTextShowPosition(outputText, position);
		XmTextSetInsertionPosition(outputText, position);
		}

}	/* END OUTPUT */

/* -------------------------------------------------------------------- */
static void CondenseVariableList()
{
	int		i, newCnt;

	newCnt = 0;

	for (i = 0; i < nVariables; ++i)
		if (Variable[i]->Output || strcmp(Variable[i]->name, "HOUR") == 0
								|| strcmp(Variable[i]->name, "MINUTE") == 0
								|| strcmp(Variable[i]->name, "SECOND") == 0)
			{
			if (i == newCnt)
				{
				++newCnt;
				continue;
				}

			Variable[newCnt++] = Variable[i];
			}
		else
			free((char *)Variable[i]);

	nVariables = newCnt;

}	/* END CONDENSEVARIABLELIST */

/* END CB_MAIN.C */
