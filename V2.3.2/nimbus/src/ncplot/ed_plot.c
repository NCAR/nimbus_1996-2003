/*
-------------------------------------------------------------------------
OBJECT NAME:	ed_plot.c

FULL NAME:		Callbacks for Parms Window

ENTRY POINTS:	CreateParmsWindow()
				EditPlotParms()
				SetYdialog()
				SetYlabel()
				SetSubtitle()

STATIC FNS:		SetReadData()
				ApplyParms()
				ValidateTime()
				ValidateFloat()

DESCRIPTION:	This set of procedures Creates & pops up/down the widget
				for "Edit Parameters" menu item.

				CreateParmsWindow - Creates the dialog widget.
				EditPlotParms	- Sets text widgets to current values
								  and pops up dialog window.
				ApplyParms		- Retrieves new values and puts in global
								  variables.

INPUT:			Global vars, title, subtitle, xlabel, ylabel, legend,
				xmin, xmax, ymin, ymax.

OUTPUT:			Same as above

AUTHOR:			websterc@ncar.ucar.edu
-------------------------------------------------------------------------
*/

#include "define.h"
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>

#define TOTAL_PLOT_PARMS	8
#define TOTAL_ASCII_PARMS	2
#define TOTAL_DIFF_PARMS	1
#define TOTAL_PRINT_PARMS	1

#define TOTAL_PARMS		(TOTAL_PLOT_PARMS+TOTAL_ASCII_PARMS+TOTAL_DIFF_PARMS+TOTAL_PRINT_PARMS)

static Widget	ParmsWindow, parmsText[TOTAL_PARMS];
static bool		TimeModified = FALSE;

static void SetReadData(), ApplyParms(), ValidateTime(), ValidateFloat();


/* -------------------------------------------------------------------- */
void EditPlotParms(Widget w, XtPointer clientData, XtPointer callData)
{
	XmTextFieldSetString(parmsText[0], title);
	XmTextFieldSetString(parmsText[1], subtitle);
	XmTextFieldSetString(parmsText[2], xlabel);
	XmTextFieldSetString(parmsText[3], ylabel);

	XtRemoveAllCallbacks(parmsText[4], XmNvalueChangedCallback);
	XtRemoveAllCallbacks(parmsText[5], XmNvalueChangedCallback);

	sprintf(buffer, "%02d:%02d:%02d",
					UserStartTime[0], UserStartTime[1], UserStartTime[2]);
	XmTextFieldSetString(parmsText[4], buffer);
	sprintf(buffer, "%02d:%02d:%02d",
					UserEndTime[0], UserEndTime[1], UserEndTime[2]);
	XmTextFieldSetString(parmsText[5], buffer);

	XtAddCallback(parmsText[4], XmNvalueChangedCallback, SetReadData, NULL);
	XtAddCallback(parmsText[5], XmNvalueChangedCallback, SetReadData, NULL);

	SetYdialog();

	sprintf(buffer, "%d", nASCIIpoints);
	XmTextFieldSetString(parmsText[8], buffer);
	XmTextFieldSetString(parmsText[9], asciiFormat);
	XmTextFieldSetString(parmsText[10], diffYlabel);
	XmTextFieldSetString(parmsText[11], lpCommand);

	XtManageChild(ParmsWindow);
	XtPopup(XtParent(ParmsWindow), XtGrabNone);

}	/* END EDITPARMS */

/* -------------------------------------------------------------------- */
void SetYlabel(char *s)
{
	strcpy(ylabel, s);
	XmTextFieldSetString(parmsText[3], ylabel);

}	/* END SETYLABEL */

/* -------------------------------------------------------------------- */
void SetYdialog()
{
	sprintf(buffer, "%e", ymin);
	XmTextFieldSetString(parmsText[6], buffer);
	sprintf(buffer, "%e", ymax);
	XmTextFieldSetString(parmsText[7], buffer);

}	/* END SETYDIALOG */

/* -------------------------------------------------------------------- */
void SetSubtitle()
{
	sprintf(subtitle, "%s, %02d:%02d:%02d-%02d:%02d:%02d", FlightDate,
				UserStartTime[0], UserStartTime[1], UserStartTime[2],
				UserEndTime[0], UserEndTime[1], UserEndTime[2]);

}	/* END SETSUBTITLE */

/* -------------------------------------------------------------------- */
static void SetReadData(Widget w, XtPointer clientData, XtPointer callData)
{
	TimeModified = TRUE;

}	/* END SETREADDATA */

/* -------------------------------------------------------------------- */
static void ApplyParms(Widget w, XtPointer clientData, XtPointer callData)
{
	char	*p;

	p = XmTextFieldGetString(parmsText[0]);
	strcpy(title, p);
	XtFree(p);

	p = XmTextFieldGetString(parmsText[1]);
	strcpy(subtitle, p);
	XtFree(p);

	p = XmTextFieldGetString(parmsText[2]);
	strcpy(xlabel, p);
	XtFree(p);

	p = XmTextFieldGetString(parmsText[3]);
	strcpy(ylabel, p);
	XtFree(p);

	p = XmTextFieldGetString(parmsText[4]);
	sscanf(p, "%2d:%2d:%2d",
				&UserStartTime[0], &UserStartTime[1], &UserStartTime[2]);
	XtFree(p);

	p = XmTextFieldGetString(parmsText[5]);
	sscanf(p, "%2d:%2d:%2d",
				&UserEndTime[0], &UserEndTime[1], &UserEndTime[2]);
	XtFree(p);

	p = XmTextFieldGetString(parmsText[6]);
	if (ymin != atof(p))
		YaxisRescaled = TRUE;

	ymin = atof(p);
	XtFree(p);

	p = XmTextFieldGetString(parmsText[7]);
	if (ymax != atof(p))
		YaxisRescaled = TRUE;

	ymax = atof(p);
	XtFree(p);

	p = XmTextFieldGetString(parmsText[8]);
	nASCIIpoints = atoi(p);
	XtFree(p);

	p = XmTextFieldGetString(parmsText[9]);
	strcpy(asciiFormat, p);
	XtFree(p);

	p = XmTextFieldGetString(parmsText[10]);
	strcpy(diffYlabel, p);
	XtFree(p);

	p = XmTextFieldGetString(parmsText[11]);
	strcpy(lpCommand, p);
	XtFree(p);

	if (TimeModified)
		{
		ReadData(NULL, NULL, NULL);
		AutoScale(SCALE_YAXIS);
		TimeModified = FALSE;
		}

	PlotData(NULL, NULL, NULL);

}	/* END APPLYPARMS */

/* -------------------------------------------------------------------- */
void CreateParmsWindow(Widget parent)
{
	Widget		plRC[TOTAL_PARMS], label[TOTAL_PARMS], b[3];
	Widget		frame[5], title[5], RC[5];
	Arg			args[10];
	int			i, n, cnt;

	n = 0;
	ParmsWindow = XmCreateRowColumn(parent, "parmsRC", args, n);

	n = 0;
	frame[0] = XmCreateFrame(ParmsWindow, "plotParmsFrame", args, 0);
	frame[1] = XmCreateFrame(ParmsWindow, "asciiParmsFrame", args, 0);
	frame[2] = XmCreateFrame(ParmsWindow, "diffParmsFrame", args, 0);
	frame[3] = XmCreateFrame(ParmsWindow, "printParmsFrame", args, 0);
	frame[4] = XmCreateFrame(ParmsWindow, "buttonFrame", args, 0);
	XtManageChildren(frame, 5);

	n = 0;
	title[0] = XmCreateLabel(frame[0], "plotParmsTitle", args, 0);
	title[1] = XmCreateLabel(frame[1], "asciiParmsTitle", args, 0);
	title[2] = XmCreateLabel(frame[2], "diffParmsTitle", args, 0);
	title[3] = XmCreateLabel(frame[3], "printParmsTitle", args, 0);
	XtManageChild(title[0]);
	XtManageChild(title[1]);
	XtManageChild(title[2]);
	XtManageChild(title[3]);

	n = 0;
	RC[0] = XmCreateRowColumn(frame[0], "plotParmsRC", args, 0);
	RC[1] = XmCreateRowColumn(frame[1], "asciiParmsRC", args, 0);
	RC[2] = XmCreateRowColumn(frame[2], "diffParmsRC", args, 0);
	RC[3] = XmCreateRowColumn(frame[3], "printParmsRC", args, 0);
	RC[4] = XmCreateRowColumn(frame[4], "buttonRC", args, 0);
	XtManageChild(RC[0]);
	XtManageChild(RC[1]);
	XtManageChild(RC[2]);
	XtManageChild(RC[3]);
	XtManageChild(RC[4]);


	/* Plot Parameters.
	 */
	for (i = cnt = 0; i < TOTAL_PLOT_PARMS; ++i, ++cnt)
		{
		n = 0;
		plRC[i] = XmCreateRowColumn(RC[0], "plRC", args, n);

		n = 0;
		sprintf(buffer, "lbl%d", cnt);
		label[cnt] = XmCreateLabel(plRC[i], buffer, args, n);

		n = 0;
		sprintf(buffer, "txt%d", cnt);
		parmsText[cnt] = XmCreateTextField(plRC[i], buffer, args, n);

		if (i == 4 || i == 5)
			{
			XtAddCallback(	parmsText[cnt], XmNvalueChangedCallback,
							SetReadData, NULL);
			XtAddCallback(	parmsText[cnt], XmNlosingFocusCallback,
							ValidateTime, NULL);
			}

		if (i == 6 || i == 7)
			XtAddCallback(	parmsText[cnt], XmNlosingFocusCallback,
							ValidateFloat, NULL);

		XtManageChild(label[cnt]);
		XtManageChild(parmsText[cnt]);
		}
		
	XtManageChildren(plRC, TOTAL_PLOT_PARMS);


	/* ASCII parameters.
	 */
	n = 0;
	plRC[0] = XmCreateRowColumn(RC[1], "plRC", args, n);
	XtManageChild(plRC[0]);

	for (i = 0; i < TOTAL_ASCII_PARMS; ++i, ++cnt)
		{
		n = 0;
		sprintf(buffer, "lbl%d", cnt);
		label[cnt] = XmCreateLabel(plRC[0], buffer, args, n);

		n = 0;
		sprintf(buffer, "txt%d", cnt);
		parmsText[cnt] = XmCreateTextField(plRC[0], buffer, args, n);

		XtManageChild(label[cnt]);
		XtManageChild(parmsText[cnt]);
		}



	/* Difference parameters.
	 */
	n = 0;
	plRC[0] = XmCreateRowColumn(RC[2], "plRC", args, n);
	XtManageChild(plRC[0]);

	for (i = 0; i < TOTAL_DIFF_PARMS; ++i, ++cnt)
		{
		n = 0;
		sprintf(buffer, "lbl%d", cnt);
		label[cnt] = XmCreateLabel(plRC[0], buffer, args, n);

		n = 0;
		sprintf(buffer, "txt%d", cnt);
		parmsText[cnt] = XmCreateTextField(plRC[0], buffer, args, n);

		XtManageChild(label[cnt]);
		XtManageChild(parmsText[cnt]);
		}



	/* Difference parameters.
	 */
	n = 0;
	plRC[0] = XmCreateRowColumn(RC[3], "plRC", args, n);
	XtManageChild(plRC[0]);

	for (i = 0; i < TOTAL_PRINT_PARMS; ++i, ++cnt)
		{
		n = 0;
		sprintf(buffer, "lbl%d", cnt);
		label[cnt] = XmCreateLabel(plRC[0], buffer, args, n);

		n = 0;
		sprintf(buffer, "txt%d", cnt);
		parmsText[cnt] = XmCreateTextField(plRC[0], buffer, args, n);

		XtManageChild(label[cnt]);
		XtManageChild(parmsText[cnt]);
		}




	/* Command buttons.
	 */
	n = 0;
	b[0] = XmCreatePushButton(RC[4], "applyButton", args, n);
	b[1] = XmCreatePushButton(RC[4], "resetButton", args, n);
	b[2] = XmCreatePushButton(RC[4], "dismissButton", args, n);
	XtAddCallback(b[0], XmNactivateCallback, ApplyParms, NULL);
	XtAddCallback(b[1], XmNactivateCallback, EditPlotParms, NULL);
	XtAddCallback(b[2], XmNactivateCallback, DismissWindow, ParmsWindow);
	XtManageChildren(b, 3);

}	/* END CREATEPARMSWINDOW */

/* -------------------------------------------------------------------- */
static void ValidateTime(Widget w, XtPointer client, XtPointer call)
{
	unsigned		hour, min, sec;
	char	*p = XmTextFieldGetString(w);

	if (strlen(p) == 0)
		return;

	hour = min = sec = 0;

	if (strchr(p, ':') != NULL)
		sscanf(p, "%u:%u:%u", &hour, &min, &sec);
	else
		sscanf(p, "%02u%02u%02u", &hour, &min, &sec);

	if (hour > 48)	hour = 48;
	if (min > 59)	min = 59;
	if (sec > 59)	sec = 59;

	sprintf(buffer, "%02d:%02d:%02d", hour, min, sec);
	XmTextFieldSetString(w, buffer);
	XtFree(p);

}	/* END VALIDATETIME */

/* -------------------------------------------------------------------- */
static void ValidateFloat(Widget w, XtPointer client, XtPointer call)
{
	char	*p;

	p = XmTextFieldGetString(w);

	sprintf(buffer, "%e", atof(p));
	XmTextFieldSetString(w, buffer);

	XtFree(p);

}	/* END VALIDATEFLOAT */

/* END ED_PLOT.C */
