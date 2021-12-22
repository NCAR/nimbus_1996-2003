/*
-------------------------------------------------------------------------
OBJECT NAME:	Xdialog.c

FULL NAME:		Callbacks for Parms Window

ENTRY POINTS:	CreateParmsWindow()
				EditParms()
				ApplyParms()
				DismissParmsWindow()
				SetYdialog()

STATIC FNS:		SetReadData()
				ValidateTime()
				ValidateFloat()

DESCRIPTION:	This set of procedures Creates & pops up/down the widget
				for "Edit Parameters" menu item.

				CreateDialog	- Creates the dialog widget.
				ShowDialog		- Sets text widgets to current values
								  and pops up dialog window.
				DoneDialog		- Retrieves new values and puts in global
								  variables.
				RemoveDialog	- Pops down the dialog widget.

INPUT:			Global vars, title, subtitle, xlabel, ylabel, legend,
				xmin, xmax, ymin, ymax.

OUTPUT:			Same as above

AUTHOR:			websterc@ncar
-------------------------------------------------------------------------
*/

#include "define.h"
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>

#define TOTAL_PARMS	8

static Widget	tw[TOTAL_PARMS];
static bool		TimeModified = FALSE;

extern Widget	ParmsWindow;

static void SetReadData(), ValidateTime(), ValidateFloat();


/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
EditParms(w, clientData, callData)
Widget		w;
XtPointer	clientData;
XtPointer	callData;
{
	XmTextFieldSetString(tw[0], title);
	XmTextFieldSetString(tw[1], subtitle);
	XmTextFieldSetString(tw[2], xlabel);
	XmTextFieldSetString(tw[3], ylabel);

	XtRemoveAllCallbacks(tw[4], XmNvalueChangedCallback);
	XtRemoveAllCallbacks(tw[5], XmNvalueChangedCallback);

	sprintf(buffer, "%02d:%02d:%02d",
					UserStartTime[0], UserStartTime[1], UserStartTime[2]);
	XmTextFieldSetString(tw[4], buffer);
	sprintf(buffer, "%02d:%02d:%02d",
					UserEndTime[0], UserEndTime[1], UserEndTime[2]);
	XmTextFieldSetString(tw[5], buffer);

	XtAddCallback(tw[4], XmNvalueChangedCallback, SetReadData, NULL);
	XtAddCallback(tw[5], XmNvalueChangedCallback, SetReadData, NULL);

	SetYdialog();

	XtManageChild(ParmsWindow);
	XtPopup(XtParent(ParmsWindow), XtGrabNone);

}	/* END EDITPARMS */

/* -------------------------------------------------------------------- */
SetYdialog()
{
	sprintf(buffer, "%e", ymin);
	XmTextFieldSetString(tw[6], buffer);
	sprintf(buffer, "%e", ymax);
	XmTextFieldSetString(tw[7], buffer);

}	/* END SETYDIALOG */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
static void
SetReadData(w, clientData, callData)
Widget		w;
XtPointer	clientData;
XtPointer	callData;
{
	TimeModified = TRUE;

}	/* END SETREADDATA */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
ApplyParms(w, clientData, callData)
Widget		w;
XtPointer	clientData;
XtPointer	callData;
{
	register char	*p;

	p = XmTextFieldGetString(tw[0]);
	strcpy(title, p);
	XtFree(p);

	p = XmTextFieldGetString(tw[1]);
	strcpy(subtitle, p);
	XtFree(p);

	p = XmTextFieldGetString(tw[2]);
	strcpy(xlabel, p);
	XtFree(p);

	p = XmTextFieldGetString(tw[3]);
	strcpy(ylabel, p);
	XtFree(p);

	p = XmTextFieldGetString(tw[4]);
	sscanf(p, "%2d:%2d:%2d",
				&UserStartTime[0], &UserStartTime[1], &UserStartTime[2]);
	XtFree(p);

	p = XmTextFieldGetString(tw[5]);
	sscanf(p, "%2d:%2d:%2d",
				&UserEndTime[0], &UserEndTime[1], &UserEndTime[2]);
	XtFree(p);

	p = XmTextFieldGetString(tw[6]);
	ymin = atof(p);
	XtFree(p);

	p = XmTextFieldGetString(tw[7]);
	ymax = atof(p);
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
/* ARGSUSED */
void
DismissParmsWindow(w, clientData, callData)
Widget		w;
XtPointer	clientData;
XtPointer	callData;
{
	XtPopdown(XtParent(ParmsWindow));
	XtUnmanageChild(ParmsWindow);

}	/* END REMOVEDIALOG */

/* -------------------------------------------------------------------- */
Widget
CreateParmsWindow(parent)
Widget	parent;
{
	Widget		parmsForm, parmsRC, fw[TOTAL_PARMS], lw[TOTAL_PARMS], b[3];
	Widget		frame, cbRC;
	Arg			args[10];
	register	i, n;

	n = 0;
	parmsForm = XmCreateForm(parent, "parmsForm", args, n);

	n = 0;
	parmsRC = XmCreateRowColumn(parmsForm, "parmsRC", args, 0);
	XtManageChild(parmsRC);

	for (i = 0; i < TOTAL_PARMS; ++i)
		{
		sprintf(buffer, "form%d", i);
		fw[i] = XmCreateForm(parmsRC, buffer, NULL, 0);

		sprintf(buffer, "lbl%d", i);
		n = 0;
		XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNrightAttachment, XmATTACH_NONE); n++;
		lw[i] = XmCreateLabel(fw[i], buffer, args, n);

		sprintf(buffer, "txt%d", i);
		n = 0;
		XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNleftWidget, lw[i]); n++;
		tw[i] = XmCreateTextField(fw[i], buffer, args, n);

		if (i == 4 || i == 5)
			{
			XtAddCallback(tw[i], XmNvalueChangedCallback, SetReadData, NULL);
			XtAddCallback(tw[i], XmNlosingFocusCallback, ValidateTime, NULL);
			}

		if (i == 6 || i == 7)
			XtAddCallback(tw[i], XmNlosingFocusCallback, ValidateFloat, NULL);

		XtManageChild(lw[i]);
		XtManageChild(tw[i]);
		}
		
	XtManageChildren(fw, TOTAL_PARMS);


	/* Command buttons.
	 */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopWidget, parmsRC); n++;
	frame = XmCreateFrame(parmsForm, "buttonFrame", args, n);
	XtManageChild(frame);

	n = 0;
	cbRC = XmCreateRowColumn(frame, "buttonRC", args, n);
	XtManageChild(cbRC);

	n = 0;
	b[0] = XmCreatePushButton(cbRC, "applyButton", args, n);
	b[1] = XmCreatePushButton(cbRC, "resetButton", args, n);
	b[2] = XmCreatePushButton(cbRC, "dismissButton", args, n);
	XtAddCallback(b[0], XmNactivateCallback, ApplyParms, NULL);
	XtAddCallback(b[1], XmNactivateCallback, EditParms, NULL);
	XtAddCallback(b[2], XmNactivateCallback, DismissParmsWindow, NULL);
	XtManageChildren(b, 3);

	return(parmsForm);

}	/* END CREATEDIALOG */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
static void
ValidateTime(w, client, call)
Widget		w;
XtPointer	client;
XtPointer	call;
{
	int		hour, min, sec;
	char	*p = XmTextFieldGetString(w);

	if (strlen(p) == 0)
		return;

	hour = min = sec = 0;

	if (strchr(p, ':') != NULL)
		sscanf(p, "%u:%u:%u", &hour, &min, &sec);
	else
		sscanf(p, "%02u%02u%02u", &hour, &min, &sec);

	if (hour < 0)	hour = 0;
	if (hour > 48)	hour = 48;
	if (min < 0)	min = 0;
	if (min > 59)	min = 59;
	if (sec < 0)	sec = 0;
	if (sec > 59)	sec = 59;

	sprintf(buffer, "%02d:%02d:%02d", hour, min, sec);
	XmTextFieldSetString(w, buffer);
	XtFree(p);

}	/* END VALIDATETIME */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
static void
ValidateFloat(w, client, call)
Widget		w;
XtPointer	client;
XtPointer	call;
{
	char	*p;

	p = XmTextFieldGetString(w);

	sprintf(buffer, "%e", atof(p));
	XmTextFieldSetString(w, buffer);

	XtFree(p);

}	/* END VALIDATEFLOAT */

/* END XDIALOG.C */
