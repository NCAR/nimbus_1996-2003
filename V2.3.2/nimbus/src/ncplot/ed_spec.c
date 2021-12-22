/*
-------------------------------------------------------------------------
OBJECT NAME:	ed_spec.c

FULL NAME:		Callbacks for Spectra Parms Window

ENTRY POINTS:	CreateSpecParmsWindow()
				EditSpecParms()

STATIC FNS:		SetSpecDefaults()
				ApplySpecParms()

DESCRIPTION:	

INPUT:			none

OUTPUT:			none

AUTHOR:			websterc@ncar
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"

#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>

#define TOTAL_PARMS	8

static Widget	SpecParmsWindow, spYmin, spYmax, slOpMenu, winOpMenu, dtOpMenu;


struct ccb_info
	{
	char	*name;
	void	*client;
	};

static struct ccb_info	segLenInfo[] = {
	"128",		(void *)128,
	"256",		(void *)256,
	"512",		(void *)512,
	"1024",		(void *)1024,
	"2048",		(void *)2048,
	"4096",		(void *)4096,
	NULL,		NULL
	};

static struct ccb_info	windowInfo[] = {
	"Bartlett",	(void *)Bartlett,
	"Blackman",	(void *)Blackman,
	"Hamming",	(void *)Hamming,
	"Hanning",	(void *)Hanning,
	"Parzen",	(void *)Parzen,
	"Square",	(void *)Square,
	"Welch",	(void *)Welch,
	NULL,		NULL
	};

static struct ccb_info	detrendInfo[] = {
	"None",		(void *)DetrendNone,
	"Mean",		(void *)DetrendMean,
	"Linear",	(void *)DetrendLinear,
	NULL,		NULL
	};


static void	SetSpecDefaults(),
		ApplySpecParms(Widget w, XtPointer client, XtPointer call);

extern int	specYmin, specYmax;



/* -------------------------------------------------------------------- */
void EditSpecParms(Widget w, XtPointer clientData, XtPointer callData)
{
	XtManageChild(SpecParmsWindow);
	XtPopup(XtParent(SpecParmsWindow), XtGrabNone);

	SetSpecDefaults();

}	/* END EDITSPECPARMS */

/* -------------------------------------------------------------------- */
static void ApplySpecParms(Widget w, XtPointer client, XtPointer call)
{
	char	*p;

	p = XmTextFieldGetString(spYmin);
	specYmin = atoi(p);
	XtFree(p);
	sprintf(buffer, "%d", specYmin);
	XmTextFieldSetString(spYmin, buffer);

	p = XmTextFieldGetString(spYmax);
	specYmax = atoi(p);
	XtFree(p);
	sprintf(buffer, "%d", specYmax);
	XmTextFieldSetString(spYmax, buffer);

	SpectrumControl(NULL, NULL, NULL);
	PlotSpectrum(NULL, NULL, NULL);

}	/* END APPLYSPECPARMS */

/* -------------------------------------------------------------------- */
void CreateSpecParmsWindow(Widget parent)
{
	int			i, n;
	Widget		frame, parmsRC, opMenRC, yAxRC, label;
	Widget		slPD, winPD, dtPD;
	Widget		slButts[6], winButts[8], dtButts[8];
	Widget		cbRC, b[3];
	Arg			args[10];
	XmString	name;

	n = 0;
	SpecParmsWindow = XmCreateRowColumn(parent, "specParmsRC", args, n);

	n = 0;
	frame	= XmCreateFrame(SpecParmsWindow, "specFrame", args, n);
	parmsRC	= XmCreateRowColumn(frame, "specRC", args, n);
	XtManageChild(frame);
	XtManageChild(parmsRC);

	opMenRC = XmCreateRowColumn(parmsRC, "opMenRC", args, n);
	yAxRC = XmCreateRowColumn(parmsRC, "yAxRC", args, n);
	XtManageChild(opMenRC);
	XtManageChild(yAxRC);


	/* Segment Length Op Menu.
	 */
    n = 0;
    slPD = XmCreatePulldownMenu(opMenRC, "slPullDown", args, n);

    n = 0;
    XtSetArg(args[n], XmNsubMenuId, slPD); ++n;
    slOpMenu = XmCreateOptionMenu(opMenRC, "slOpMenu", args, n);
    XtManageChild(slOpMenu);


	for (i = 0; segLenInfo[i].name; ++i)
		{
		name = XmStringCreateLocalized(segLenInfo[i].name);

		n = 0;
		XtSetArg(args[n], XmNlabelString, name); ++n;
		slButts[i] = XmCreatePushButton(slPD, "opMenB", args, n);
		XtAddCallback(	slButts[i], XmNactivateCallback, SetSegLen,
						(XtPointer)segLenInfo[i].client);

		XmStringFree(name);
		}
 
    XtManageChildren(slButts, i);



	/* Window Function Op Menu.
	 */
    n = 0;
    winPD = XmCreatePulldownMenu(opMenRC, "winPullDown", args, n);
 
    n = 0;
    XtSetArg(args[n], XmNsubMenuId, winPD); ++n;
    winOpMenu = XmCreateOptionMenu(opMenRC, "winOpMenu", args, n);
    XtManageChild(winOpMenu);


	for (i = 0; windowInfo[i].name; ++i)
		{
		name = XmStringCreateLocalized(windowInfo[i].name);

		n = 0;
		XtSetArg(args[n], XmNlabelString, name); ++n;
		winButts[i] = XmCreatePushButton(winPD, "opMenB", args, n);
		XtAddCallback(	winButts[i], XmNactivateCallback, SetWindow,
						(XtPointer)windowInfo[i].client);

		XmStringFree(name);
		}
 
    XtManageChildren(winButts, i);



	/* Detrend Function Op Menu.
	 */
    n = 0;
    dtPD = XmCreatePulldownMenu(opMenRC, "dtPullDown", args, n);
 
    n = 0;
    XtSetArg(args[n], XmNsubMenuId, dtPD); ++n;
    dtOpMenu = XmCreateOptionMenu(opMenRC, "dtOpMenu", args, n);
    XtManageChild(dtOpMenu);


	for (i = 0; detrendInfo[i].name; ++i)
		{
		name = XmStringCreateLocalized(detrendInfo[i].name);

		n = 0;
		XtSetArg(args[n], XmNlabelString, name); ++n;
		dtButts[i] = XmCreatePushButton(dtPD, "opMenB", args, n);
		XtAddCallback(	dtButts[i], XmNactivateCallback, SetDetrend,
						(XtPointer)detrendInfo[i].client);

		XmStringFree(name);
		}
 
    XtManageChildren(dtButts, i);


	/* Y Axis labels.
	 */
	n = 0;
	label = XmCreateLabel(yAxRC, "spYminLbl", args, n);
	XtManageChild(label);

	n = 0;
	spYmin = XmCreateTextField(yAxRC, "spYminTxt", args, n);
	XtManageChild(spYmin);

	n = 0;
	label = XmCreateLabel(yAxRC, "spYmaxLbl", args, n);
	XtManageChild(label);

	n = 0;
	spYmax = XmCreateTextField(yAxRC, "spYmaxTxt", args, n);
	XtManageChild(spYmax);



	/* Command buttons.
	 */
	n = 0;
	frame = XmCreateFrame(SpecParmsWindow, "buttonFrame", args, n);
	XtManageChild(frame);

	n = 0;
	cbRC = XmCreateRowColumn(frame, "buttonRC", args, n);
	XtManageChild(cbRC);

	n = 0;
	b[0] = XmCreatePushButton(cbRC, "applyButton", args, n);
	b[1] = XmCreatePushButton(cbRC, "resetButton", args, n);
	b[2] = XmCreatePushButton(cbRC, "dismissButton", args, n);
	XtAddCallback(b[0], XmNactivateCallback, ApplySpecParms, NULL);
	XtAddCallback(b[1], XmNactivateCallback, EditSpecParms, NULL);
	XtAddCallback(b[2], XmNactivateCallback, DismissWindow, SpecParmsWindow);
	XtManageChildren(b, 3);

}	/* END CREATESPECPARMSWINDOW */

/* -------------------------------------------------------------------- */
static void SetSpecDefaults()
{
	int			i, x;
	Arg         args[4];
	XmString    name;

	extern int		M;
	extern void		(*detrendFn)();
	extern double	(*windowFn)();


	/* Set default values for above widgets.  These should jive with what's
	 * set in spec.c, SpectrumControl().
	 */
	for (x = i = 0; segLenInfo[i].name; ++i)
		if ((int)segLenInfo[i].client == M)
			x = i;

	name = XmStringCreateLocalized(segLenInfo[x].name);
	XtSetArg(args[0], XmNlabelString, name);
	XtSetValues(XmOptionButtonGadget(slOpMenu), args, 1);
	XmStringFree(name);

	for (x = i = 0; windowInfo[i].name; ++i)
		if ((int)windowInfo[i].client == (int)windowFn)
			x = i;

	name = XmStringCreateLocalized(windowInfo[x].name);
	XtSetArg(args[0], XmNlabelString, name);
	XtSetValues(XmOptionButtonGadget(winOpMenu), args, 1);
	XmStringFree(name);

	for (x = i = 0; detrendInfo[i].name; ++i)
		if ((int)detrendInfo[i].client == (int)detrendFn)
			x = i;

	name = XmStringCreateLocalized(detrendInfo[x].name);
	XtSetArg(args[0], XmNlabelString, name);
	XtSetValues(XmOptionButtonGadget(dtOpMenu), args, 1);
	XmStringFree(name);

	sprintf(buffer, "%d", specYmin);
	XmTextFieldSetString(spYmin, buffer);
	sprintf(buffer, "%d", specYmax);
	XmTextFieldSetString(spYmax, buffer);

}

/* END ED_SPEC.C */
