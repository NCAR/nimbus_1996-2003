/*
-------------------------------------------------------------------------
OBJECT NAME:	ascii.c

FULL NAME:		ASCII Window & callbacks

ENTRY POINTS:	ViewASCII()
				SetASCIIdata()

STATIC FNS:		CreateASCIIwindow()
				DismissASCII()
				SaveASCII()
				Save_OK()
				PrintASCII()
				formatTitle()
				formatLine()

DESCRIPTION:	

REFERENCES:		none

REFERENCED BY:	Callback, Xplot.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1996
-------------------------------------------------------------------------
*/

#include "define.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>

static Widget Shell004, ASCIIwindow, asciiText;

static void
	CreateASCIIwindow(),
	SaveASCII(Widget w, XtPointer client, XtPointer call),
	Save_OK(Widget w, XtPointer client, XmFileSelectionBoxCallbackStruct *call),
	PrintASCII(Widget w, XtPointer client, XtPointer call),
	DismissASCII(Widget w, XtPointer client, XtPointer call);

static char *formatTitle(char *);
static char *formatLine(char *, int, int, int, int, int);

extern Widget	AppShell;


/* -------------------------------------------------------------------- */
static void DismissASCII(Widget w, XtPointer client, XtPointer call)
{
	AsciiWinOpen = FALSE;

	XtUnmanageChild(ASCIIwindow);
	XtPopdown(XtParent(ASCIIwindow));

}

/* -------------------------------------------------------------------- */
void ViewASCII(Widget w, XtPointer client, XtPointer call)
{
	AsciiWinOpen = TRUE;

	SetASCIIdata(NULL, NULL, NULL);
	XtManageChild(ASCIIwindow);
	XtPopup(XtParent(ASCIIwindow), XtGrabNone);
 
}

/* -------------------------------------------------------------------- */
void SetASCIIdata(Widget w, XtPointer client, XtPointer call)
{
	int		i, hour, min, sec, msec;
	int		msecCnt;

	static bool	firstTime = TRUE;

	if (NumberDataSets == 0)
		{
		if (AsciiWinOpen)
			DismissASCII(NULL, NULL, NULL);
			
		return;
		}

	if (firstTime) {
		CreateASCIIwindow();
		firstTime = FALSE;
		}

	XmTextSetString(asciiText, formatTitle(buffer));


	hour = UserStartTime[0];
	min = UserStartTime[1];
	sec = UserStartTime[2];
	msec = 0;
	msecCnt = 1000 / (NumberElements[0] / NumberSeconds);

	for (i = 0; i < nASCIIpoints; ++i)
		{
		formatLine(buffer, i, hour, min, sec, msec);
		XmTextInsert(asciiText, XmTextGetLastPosition(asciiText), buffer);

		if ((msec += msecCnt) > 999) {
			msec = 0;
			if (++sec > 59) {
				sec = 0;
				if (++min > 59) {
					min = 0;
					if (++hour > 23)
						hour = 0;
					}
				}
			}
		}

}	/* END VIEWASCII */

/* -------------------------------------------------------------------- */
static void CreateASCIIwindow()
{
	Arg         args[32];
	Cardinal    n;
	Widget      drFrame, drRC, b[3];

	n = 0;
	Shell004 = XtCreatePopupShell("asciiShell",
						topLevelShellWidgetClass, AppShell, args, n);

	n = 0;
	ASCIIwindow = XmCreateForm(Shell004, "asciiForm", args, n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	drFrame = XmCreateFrame(ASCIIwindow, "buttonFrame", args, n);
	XtManageChild(drFrame);

	n = 0;
	drRC = XmCreateRowColumn(drFrame, "buttonRC", args, n);
	XtManageChild(drRC);


	n = 0;
	b[0] = XmCreatePushButton(drRC, "saveButton", args, n);
	XtAddCallback(b[0], XmNactivateCallback, SaveASCII, NULL);

	n = 0;
	b[1] = XmCreatePushButton(drRC, "printButton", args, n);
	XtAddCallback(b[1], XmNactivateCallback, PrintASCII, NULL);

	n = 0;
	b[2] = XmCreatePushButton(drRC, "dismissButton", args, n);
	XtAddCallback(b[2], XmNactivateCallback, DismissASCII, ASCIIwindow);

	XtManageChildren(b, 3);


	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, drFrame); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	asciiText = XmCreateScrolledText(ASCIIwindow, "asciiText", args, n);
	XtManageChild(asciiText);

}	/* END CREATEASCIIWINDOW */

/* -------------------------------------------------------------------- */
static void SaveASCII(Widget w, XtPointer client, XtPointer call)
{
	sprintf(buffer, "%s/*", getenv("DATA_DIR"));
	QueryFile("Enter file name to save:", buffer, Save_OK);

}

/* -------------------------------------------------------------------- */
static void Save_OK(Widget w, XtPointer client, XmFileSelectionBoxCallbackStruct *call)
{
	FILE	*fp;
	char    *file;
	int     i, hour, min, sec, msec;
	int     msecCnt;
 

	ExtractFileName(call->value, &file);
	FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

	if ((fp = fopen(file, "w")) == NULL)
		{
		sprintf(buffer, "Save: can't open %s.", file);
		ShowError(buffer);
		return;
		}

 
	fprintf(fp, formatTitle(buffer));
 
 
	hour = UserStartTime[0];
	min = UserStartTime[1];
	sec = UserStartTime[2];
	msec = 0;
	msecCnt = 1000 / (NumberElements[0] / NumberSeconds);
 
 
	for (i = 0; i < NumberElements[0]; ++i)
		{
		formatLine(buffer, i, hour, min, sec, msec);
		fprintf(fp, buffer);
 
		if ((msec += msecCnt) > 999) {
            msec = 0;
            if (++sec > 59) {
                sec = 0;
                if (++min > 59) {
                    min = 0;
                    if (++hour > 23)
                        hour = 0;
                    }
                }
            }
        }

	fclose(fp);

}	/* END SAVE_OK */

/* -------------------------------------------------------------------- */
static void PrintASCII(Widget w, XtPointer client, XtPointer call)
{
	FILE    *fp;
	int     i, hour, min, sec, msec;
	int     msecCnt;
	char	*p;

	if ((p = getenv("LPDEST")) != NULL)
		printf("Output being sent to %s.\n", p);

	if ((fp = popen(lpCommand, "w")) == NULL)
		{
		ShowError("PrintASCII: can't open pipe to 'lp'");
		return;
		}


	fprintf(fp, formatTitle(buffer));
 
 
	hour = UserStartTime[0];
	min = UserStartTime[1];
	sec = UserStartTime[2];
	msec = 0;
	msecCnt = 1000 / (NumberElements[0] / NumberSeconds);
 
 
	for (i = 0; i < NumberElements[0]; ++i)
		{
		formatLine(buffer, i, hour, min, sec, msec);
		fprintf(fp, buffer);
 
		if ((msec += msecCnt) > 999) {
            msec = 0;
            if (++sec > 59) {
                sec = 0;
                if (++min > 59) {
                    min = 0;
                    if (++hour > 23)
                        hour = 0;
                    }
                }
            }
		}

	pclose(fp);

}	/* END PRINTASCII */

/* -------------------------------------------------------------------- */
static char *formatTitle(char buff[]) 
{
	int		lrOffset, i, varCnt = 0;
	VARTBL	*vp;

    lrOffset = (NumberElements[0] == NumberSeconds) ? 16 : 19;
    memset(buff, ' ', 20 * (NumberDataSets+1));
 
    for (i = 0; i < NumberDataSets; ++i)
        {
        vp = Variable[SelectedVarIndex[i]];
 
        if (NumberElements[i] == NumberElements[0])
            memcpy(&buff[lrOffset+(14*varCnt++)], vp->name, strlen(vp->name));
        }
 
    strcpy(&buff[lrOffset+(varCnt*14)], "\n");

	return(buff);

}	/* END FORMATTITLE */

/* -------------------------------------------------------------------- */
static char *formatLine(
		char buff[],							/* Output buffer	*/
		int indx,								/* Record index		*/
		int hour, int min, int sec, int msec)	/* Time stamp		*/
{
	int		i;
	char	tempBuff[32];

	if (NumberElements[0] == NumberSeconds)
		sprintf(buff, "%02d:%02d:%02d   ", hour, min, sec);
	else
		sprintf(buff, "%02d:%02d:%02d.%03d   ", hour, min, sec, msec);

	for (i = 0; i < NumberDataSets; ++i)
		{
		if (NumberElements[i] == NumberElements[0])
			{
			sprintf(tempBuff, asciiFormat, data[i][indx]);
			strcat(buff, tempBuff);
			}
		}

	strcat(buff, "\n");

	return(buff);

}

/* END ASCII.C */
