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
GetDataFileName(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	QueryFile("Enter Data file to read:", DataPath, NewDataFile);

}

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
ReadParmsFileName(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	QueryFile("Enter Parameter file to read:", UserPath, ReadParmsFile);

}

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
WriteParmsFileName(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	QueryFile("Enter Parameter ouput file name:", UserPath, write_parms_CB2);

}

/* ARGSUSED */
void
write_parms_CB2(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
	ExtractFileName(((XmFileSelectionBoxCallbackStruct *)
					callData)->value, &parms_file);

	if (access(parms_file, F_OK) == 0)
		{
		sprintf(buffer, "Overwrite file %s", parms_file);
		WarnUser(buffer, WriteParmsFile, NULL);
		}
	else
		WriteParmsFile((Widget)NULL,(XtPointer)NULL,(XtPointer)NULL);

}	/* END WRITE_PARMS_CB2 */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
PrintPS(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	if (data_file == NULL)
		{
		HandleError("No data.", Interactive, RETURN);
		return;
		}

	if (clientData)
		QueryFile("Enter PostScript output file name:", (char *)NULL,
							print_PS_CB2);
	else
		{
		out_file = NULL;
		PrintPostScript((Widget)NULL,(XtPointer)NULL,(XtPointer)NULL);
		if (Interactive)
			PlotData((Widget)NULL, (XtPointer)NULL,
					(XmDrawingAreaCallbackStruct *)NULL);
		}

}	/* END PRINT_PS */

/* ARGSUSED */
void
print_PS_CB2(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{

	FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
	ExtractFileName(((XmFileSelectionBoxCallbackStruct *)
					callData)->value, &out_file);

	if (access(out_file, F_OK) == 0)
		{
		sprintf(buffer, "Overwrite file %s", out_file);
		WarnUser(buffer, PrintPostScript, NULL);
		}
	else
		PrintPostScript((Widget)NULL,(XtPointer)NULL,(XtPointer)NULL);

}	/* END PRINT_PS_CB2 */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
DoAutoScale(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	AutoScale(SCALE_XAXIS | SCALE_YAXIS);

	if (Interactive)
		PlotData((Widget)NULL, (XtPointer)NULL,
					(XmDrawingAreaCallbackStruct *)NULL);

}	/* END AUTOSCALE */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
ToggleScatter(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	ScatterPlot = !ScatterPlot;

	if (Interactive)
		PlotData((Widget)NULL, (XtPointer)NULL,
					(XmDrawingAreaCallbackStruct *)NULL);

}	/* END TOGGLESCATTER */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
ToggleWindBarb(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	WindBarbs = !WindBarbs;

	if (WindBarbs)
		ReadData(UI | VI);

	if (Interactive)
		PlotData((Widget)NULL, (XtPointer)NULL,
					(XmDrawingAreaCallbackStruct *)NULL);

}	/* END TOGGLEWINDBARB */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
ToggleGrid(widget, clientData, callData)
Widget          widget;
XtPointer       clientData, callData;
{
	Grid = !Grid;

	if (Interactive)
		PlotData((Widget)NULL, (XtPointer)NULL,
					(XmDrawingAreaCallbackStruct *)NULL);

}	/* END TOGGLEGRID */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
ModifyActiveVars(widget, clientData, callData)
Widget		widget;
XtPointer	clientData;
XtPointer	callData;
{
	VARTBL	*vp;
	int		position;
	int		*pos_list, pos_cnt;

	if (XmListGetSelectedPos(widget, &pos_list, &pos_cnt) == FALSE)
		return;

	if (widget == varListX)
		{
		vpX = Variable[(position = pos_list[0] - 1)];
		ReadData(XI);
		AutoScale(SCALE_XAXIS);
		}
	else
		{
		vpY = Variable[(position = pos_list[0] - 1)];
		ReadData(YI);
		AutoScale(SCALE_YAXIS);
		}


	if (vpX && vpY)
		{
		if (uic == NULL && vic && NULL)
			ReadData(UI | VI);

		AutoScale(SCALE_XAXIS | SCALE_YAXIS);
		PlotData(NULL, NULL, NULL);
		}

}	/* END MODIFYACTIVEVARS */

/* -------------------------------------------------------------------- */
static int EOFreached = FALSE, SavedNumberSeconds;

/* ARGSUSED */
void
PageForward(widget, clientData, callData)
Widget		widget;
XtPointer	clientData, callData;
{
	int		st[3];

	memcpy((char *)st, (char *)UserStartTime, sizeof(st));
	st[2] += NumberSeconds;

	while (st[2] > 59)
		{
		st[1] += 1;
		st[2] -= 60;
		}

	while (st[1] > 59)
		{
		st[0] += 1;
		st[1] -= 60;
		}

	if (memcmp((char *)st, (char *)FileEndTime, sizeof(st)) > 0)
		return;

	memcpy((char *)UserStartTime, (char *)st, sizeof(st));

	UserEndTime[2] += NumberSeconds;

	while (UserEndTime[2] > 59)
		{
		UserEndTime[1] += 1;
		UserEndTime[2] -= 60;
		}

	while (UserEndTime[1] > 59)
		{
		UserEndTime[0] += 1;
		UserEndTime[1] -= 60;
		}

	if (memcmp((char *)UserEndTime, (char *)FileEndTime, sizeof(st)) > 0)
		{
		memcpy((char *)UserEndTime, (char *)FileEndTime, sizeof(st));
		EOFreached = TRUE;
		SavedNumberSeconds = NumberSeconds;
		}

	ReadData(XI | YI | UI | VI);
	AutoScale(SCALE_XAXIS | SCALE_YAXIS);
	PlotData(NULL, NULL, NULL);

}	/* END PAGEFORWARD */

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void
PageBackward(widget, clientData, callData)
Widget		widget;
XtPointer	clientData, callData;
{
	if (EOFreached)
		{
		EOFreached = FALSE;
		NumberSeconds = SavedNumberSeconds;
		}

	UserStartTime[2] -= NumberSeconds;

	while (UserStartTime[2] < 0)
		{
		UserStartTime[1] -= 1;
		UserStartTime[2] += 60;
		}

	while (UserStartTime[1] < 0)
		{
		UserStartTime[0] -= 1;
		UserStartTime[1] += 60;
		}

	if (memcmp((char *)UserStartTime, (char *)FileStartTime, 3*sizeof(int)) < 0)
		memcpy((char *)UserStartTime, (char *)FileStartTime, 3*sizeof(int));


	memcpy((char *)UserEndTime, (char *)UserStartTime, 3*sizeof(int));
	UserEndTime[2] += NumberSeconds;

	while (UserEndTime[2] > 59)
		{
		UserEndTime[1] += 1;
		UserEndTime[2] -= 60;
		}

	while (UserEndTime[1] > 59)
		{
		UserEndTime[0] += 1;
		UserEndTime[1] -= 60;
		}

	ReadData(XI | YI | UI | VI);
	AutoScale(SCALE_XAXIS | SCALE_YAXIS);
	PlotData(NULL, NULL, NULL);

}	/* END PAGEBACKWARD */

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
