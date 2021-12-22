/*
-------------------------------------------------------------------------
OBJECT NAME:	ccb.c

FULL NAME:		Command CallBacks

ENTRY POINTS:	

STATIC FNS:		none

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	XtAppMainLoop()

COPYRIGHT:		University Corporation for Atmospheric Research, 1994
-------------------------------------------------------------------------
*/

#include "define.h"
#include <unistd.h>


/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
Freeze(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	Frozen = !Frozen;

}	/* END FREEZE */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
DoAutoScale(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	AutoScale(SCALE_YAXIS);

	PlotData((Widget)NULL, (XtPointer)NULL,(XmDrawingAreaCallbackStruct *)NULL);

}	/* END AUTOSCALE */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
ToggleCounts(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	if (Counts)
		{
		Counts = FALSE;
		strcpy(ylabel, "Engineering");
		}
	else
		{
		Counts = TRUE;
		strcpy(ylabel, "Counts");
		}

}	/* END TOGGLECOUNTS */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
ToggleFlash(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	FlashMode = !FlashMode;

}	/* END TOGGLEFLASH */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
ToggleGrid(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	Grid = !Grid;
	PlotData((Widget)NULL, (XtPointer)NULL,(XmDrawingAreaCallbackStruct *)NULL);

}	/* END TOGGLEGRID */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
ModifyActiveVars(widget, clientData, callData)
Widget		widget;
XtPointer	clientData;
XtPointer	callData;
{
	int		i;
	int		*pos_list, pos_cnt;

	if (XmListGetSelectedPos(varList, &pos_list, &pos_cnt) == FALSE)
		return;

	pos_list[0] -= 1;

	if (pos_list[0] >= nsdi)
		{
		pos_list[0] -= nsdi;

		for (i = 0; i < nVariables; ++i)
			if (strcmp(Variable[i].name, raw[pos_list[0]]->name) == 0)
				{
				DeleteVariable(i);
				return;
				}

		if (NumberDataSets + 1 > MAXDATASETS)
			{
			ShowError("Out of data sets.", RETURN);
			return;
			}
		else
			AddVariable(pos_list[0] + nsdi);
		}
	else
		{
		for (i = 0; i < nVariables; ++i)
			if (strcmp(Variable[i].name, sdi[pos_list[0]]->name) == 0)
				{
				DeleteVariable(i);
				return;
				}

		if (NumberDataSets + 1 > MAXDATASETS)
			{
			ShowError("Out of data sets.", RETURN);
			return;
			}
		else
			AddVariable(pos_list[0]);
		}

}	/* END MODIFYACTIVEVARS */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
Quit(widget, clientData, callData)
Widget		widget;
XtPointer	clientData, callData;
{
	exit(0);

}	/* END QUIT */

/* END CCB.C */
