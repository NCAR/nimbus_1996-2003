/*
-------------------------------------------------------------------------
OBJECT NAME:	diff.c

FULL NAME:		Differences

ENTRY POINTS:	ViewDifference()
				CreateDifferenceWindow()

STATIC FNS:		none

DESCRIPTION:	

REFERENCES:		XplotDiff.c

REFERENCED BY:	Callback

COPYRIGHT:		University Corporation for Atmospheric Research, 1996
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"

#include <float.h>
#include <math.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>

Widget	diffCanvas;
float	*diffData[1];
float	diffYmin, diffYmax;
STAT	DiffStats;

static Widget	Shell005, DifferenceWindow = NULL;
extern Widget	AppShell;

static void	CreateDifferenceWindow();

void	PlotDifference();

/* -------------------------------------------------------------------- */
void ComputeDiff(Widget w, XtPointer client, XtPointer call)
{
	int		i, j, nPoints;
	float	sum, sigma;
	static int	firstTime = TRUE;

	if (data[0] == NULL || data[1] == NULL ||
		NumberElements[0] != NumberElements[1])
		{
		if (DiffWinOpen)
			DiffWinControl();

		return;
		}

	if (firstTime)
		{
		CreateDifferenceWindow();
		firstTime = FALSE;
		}
	else
		free(diffData[0]);

	sprintf(diffYlabel, "(%s-%s) %s",	Variable[SelectedVarIndex[0]]->name,
										Variable[SelectedVarIndex[1]]->name,
										ylabel);

	diffData[0] = (float *)GetMemory(sizeof(float) * NumberElements[0]);

	DiffStats.min = FLT_MAX;
	DiffStats.max = -FLT_MAX;

	for (sum = 0.0, i = nPoints = 0; i < NumberElements[0]; ++i)
		{
		if (data[0][i] == MISSING_VALUE || data[1][i] == MISSING_VALUE)
			continue;

		diffData[0][i] = data[0][i] - data[1][i];

		DiffStats.min = MIN(DiffStats.min, diffData[0][i]);
		DiffStats.max = MAX(DiffStats.max, diffData[0][i]);

		sum += diffData[0][i];
		++nPoints;
		}

	diffYmin = DiffStats.min;
	diffYmax = DiffStats.max;

	DiffStats.mean = sum / nPoints;

	sigma = 0.0;

	for (j = 0; j < NumberElements[0]; ++j)
		if ((int)diffData[0][j] != MISSING_VALUE)
			sigma += pow(diffData[0][j] - DiffStats.mean, 2.0);
 
	DiffStats.variance = sigma / (nPoints - 1);
	DiffStats.sigma = (float)sqrt(sigma / (nPoints - 1));

	AutoScaleDiff();
}

/* -------------------------------------------------------------------- */
void DiffWinControl()
{
	if (DiffWinOpen)
		{
		XtUnmanageChild(DifferenceWindow);
		XtPopdown(XtParent(DifferenceWindow));
		DiffWinOpen = FALSE;
		}
	else
		{
		ComputeDiff(NULL, NULL, NULL);

		if (DifferenceWindow)
			{
			XtManageChild(DifferenceWindow);
			XtPopup(XtParent(DifferenceWindow), XtGrabNone);
			DiffWinOpen = TRUE;
			}
		}

}

/* -------------------------------------------------------------------- */
static void CreateDifferenceWindow()
{
	Cardinal n;
	Arg		args[8];

	n = 0;
	Shell005 = XtCreatePopupShell("diffShell", topLevelShellWidgetClass,
														AppShell, args, n);

	n = 0;
	DifferenceWindow = XmCreateForm(Shell005, "diffForm", args, n);


	/* Create Graphics Canvas
	*/
	n = 0;
	diffCanvas = XmCreateDrawingArea(DifferenceWindow, "diffCanvas", args, n);
	XtManageChild(diffCanvas);

	XtAddCallback(diffCanvas, XmNexposeCallback, (XtCallbackProc)PlotDifference,
												(XtPointer)NULL);

}	/* END CREATEDIFFERENCEWINDOW */

/* END DIFF.C */
