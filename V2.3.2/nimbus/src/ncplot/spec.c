/*
-------------------------------------------------------------------------
OBJECT NAME:	spec.c

FULL NAME:		Spectrum

ENTRY POINTS:	SpectrumControl()
				SetSegLen()
				SetWindow()
				SetDetrend()

STATIC FNS:		CreateSpectrumWindow()

DESCRIPTION:	

REFERENCES:		spctrm.c, XplotSpec.c

REFERENCED BY:	Callback

COPYRIGHT:		University Corporation for Atmospheric Research, 1996
-------------------------------------------------------------------------
*/

#include "define.h"
#include "spec.h"

#include <math.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>


Widget	specCanvas;
double	*Pxx;

static float	*detrendedData[2];
static int		K;

int		M = 1024;
int		specYmin = -6.0, specYmax = 3.0,
		specXmin = -2.0, specXmax = 3.0;

void	(*detrendFn)() = DetrendMean;
double	(*windowFn)() = Welch;


static Widget	Shell002, SpectrumWindow;
extern Widget	AppShell;

static void	CreateSpectrumWindow();


/* -------------------------------------------------------------------- */
void SpectrumControl(Widget w, XtPointer client, XtPointer call)
{
	int	i;
	static int	firstTime = TRUE;
	double		variance, pw, cf;

	if (data[0] == NULL)
		return;

	if (firstTime)
		{
		CreateSpectrumWindow();
		firstTime = FALSE;
		}
	else
		{
		if (RecomputeSpectra == FALSE)
			return;

		free(detrendedData[0]);
		free(Pxx);
		detrendedData[0] = NULL;
		Pxx = NULL;
		}


	K = NumberElements[0] / M;

	detrendedData[0] = (float *)GetMemory(sizeof(float) * ((K+1) * M));
	Pxx = (double *)GetMemory(sizeof(double) * (M+1));

	memset((char *)detrendedData[0], 0, sizeof(float) * ((K+1) * M));
	(*detrendFn)(data[0], detrendedData[0], NumberElements[0]);

	Spectrum(detrendedData[0], Pxx, K, M, windowFn);

	pw = (double)((K+1) * M) / ((double) NumberElements[0]);
	cf = (2.0*M) / (NumberElements[0] / NumberSeconds);

printf("pd_wt=%f, k=%d, m=%d, cf=%f\n", pw, K, M, cf);
	variance = 0.0;

	for (i = 1; i <= M; ++i)
		{
		Pxx[i] *= pw;
		variance += Pxx[i];
		Pxx[i] *= cf;
		}

printf("variance=%f\n", variance);

	RecomputeSpectra = FALSE;


	XtManageChild(SpectrumWindow);
	XtPopup(XtParent(SpectrumWindow), XtGrabNone);

}

/* -------------------------------------------------------------------- */
void SetSegLen(Widget w, XtPointer client, XtPointer call)
{
	M = (int)client;
	RecomputeSpectra = TRUE;
}

/* -------------------------------------------------------------------- */
void SetWindow(Widget w, XtPointer client, XtPointer call)
{
	windowFn = (void *)client;
	RecomputeSpectra = TRUE;
}

/* -------------------------------------------------------------------- */
void SetDetrend(Widget w, XtPointer client, XtPointer call)
{
	detrendFn = (void *)client;
	RecomputeSpectra = TRUE;
}

/* -------------------------------------------------------------------- */
static void CreateSpectrumWindow()
{
	Cardinal n;
	Arg		args[8];

	n = 0;
	Shell002 = XtCreatePopupShell("specShell", topLevelShellWidgetClass,
														AppShell, args, n);

	n = 0;
	SpectrumWindow = XmCreateForm(Shell002, "specForm", args, n);


	/* Create Graphics Canvas
	*/
	n = 0;
	specCanvas = XmCreateDrawingArea(SpectrumWindow, "specCanvas", args, n);
	XtManageChild(specCanvas);

	XtAddCallback(specCanvas, XmNexposeCallback, (XtCallbackProc)PlotSpectrum,
												(XtPointer)NULL);

}	/* END CREATESPECTRUMWINDOW */

/* END SPEC.C */
