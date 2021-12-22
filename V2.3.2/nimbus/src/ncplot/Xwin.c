/*
-------------------------------------------------------------------------
OBJECT NAME:	Xwin.c

FULL NAME:		X window Stuff

TYPE:			X11/R5 Motif 1.2

DESCRIPTION:	This creates the initial X window, menus and all associated
				widgets.  Also calls CreateFile(), CreateError(),
				CreateQuery().

INPUT:			none

OUTPUT:			none

AUTHOR:			websterc@ncar
-------------------------------------------------------------------------
*/

#include "define.h"

#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <Xm/CascadeB.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>

struct menu
	{
	char		*title;
	void		(*callback)();
	XtPointer	callData;
	} ;

static struct menu	fileMenu[] = {
		"newData", GetDataFileName, NULL,
		"readParms", ReadParmsFileName, NULL,
		"writeParms", WriteParmsFileName, NULL,
		"savePS", PrintPS, NULL,
		"print", PrintPS, NULL,
		"quit", Quit, NULL,
		NULL, NULL, NULL };

static struct menu	legendMenu[] = {
		"editParms", EditPlotParms, NULL,
		"editSpec", EditSpecParms, NULL,
		NULL, NULL, NULL };

static struct menu	viewMenu[] = {
		"viewASCII", ViewASCII, NULL,
		"viewDiff", DiffWinControl, NULL,
		"viewSpectra", SpectrumControl, NULL,
		NULL, NULL, NULL };

static struct menu	optionMenu[] = {
		"toggleGrid", ToggleGrid, NULL,
		"autoScale", DoAutoScale, NULL,
		"toggleAS", ToggleAutoScaling, NULL,
		NULL, NULL, NULL };

static struct
	{
	char		*title;
	struct menu	*sub;
	} main_menu[] = {
		"File", fileMenu,
		"Edit", legendMenu,
		"View", viewMenu,
		"Options", optionMenu,
		NULL, NULL };

void	AddVariable(), DeleteVariable(), PageForward(), PageBackward();


/* -------------------------------------------------------------------- */
Widget CreateMainWindow(parent)
Widget	parent;
{
	Widget	menubar, form, menu[5], bttn[50], menu_button[5], fwd, bkd,
			frame, brc;
	Arg		args[9];
	int		n, i, j;

	n = 0;
	form = XmCreateForm(parent, "mainForm", args, n);

	n = 0;
	menubar = XmCreateMenuBar(form, "menuBar", args, n);
	XtManageChild(menubar);


	for (i = 0; main_menu[i].title; ++i)
		{
		n = 0;
		menu[i] = XmCreatePulldownMenu(menubar, main_menu[i].title, args, n);

		n = 0;
		XtSetArg(args[n], XmNsubMenuId, menu[i]); ++n;
		menu_button[i] = XmCreateCascadeButton(menubar, main_menu[i].title,
																	args, n);

		for (j = 0; main_menu[i].sub[j].title; ++j)
			{
			n = 0;
			bttn[j] = XmCreatePushButton(menu[i],
									main_menu[i].sub[j].title, args, n);
			XtAddCallback(bttn[j], XmNactivateCallback,
										main_menu[i].sub[j].callback,
										main_menu[i].sub[j].callData);
			}

		XtManageChildren(bttn, j);
		}

	XtManageChildren(menu_button, i);


	/* Create Graphics Canvas
	 */
	canvas = XmCreateDrawingArea(form, "canvas", NULL, 0);
	XtAddCallback(canvas, XmNexposeCallback, (XtCallbackProc)PlotData,
							(XtPointer)NULL);

	XtManageChild(canvas);
	XtAddCallback(canvas, XmNinputCallback, Zoom, NULL);


	/* Page Fwd & Bkwd buttons.
	 */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget, canvas); n++;
	frame = XmCreateFrame(form, "buttonFrame", args, n);
	XtManageChild(frame);

	n = 0;
	brc = XmCreateRowColumn(frame, "pgButtRC", args, n);
	XtManageChild(brc);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget, canvas); n++;
	bkd = XmCreatePushButton(brc, "pageBkd", args, n);
	XtAddCallback(bkd, XmNactivateCallback, PageBackward, NULL);
	XtManageChild(bkd);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, bkd); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget, canvas); n++;
	fwd = XmCreatePushButton(brc, "pageFwd", args, n);
	XtAddCallback(fwd, XmNactivateCallback, PageForward, NULL);
	XtManageChild(fwd);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, fwd); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget, canvas); n++;
	varList = XmCreateScrolledList(form, "varList", args, n);
	XtAddCallback(varList, XmNdefaultActionCallback, ModifyActiveVars, NULL);
	XtManageChild(varList);

	return(form);

}	/* END CREATEMAINWINDOW */

/* END XWIN.C */
