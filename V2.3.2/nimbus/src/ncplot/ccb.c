/*
-------------------------------------------------------------------------
OBJECT NAME:	ccb.c

FULL NAME:		Command CallBacks

ENTRY POINTS:	

STATIC FNS:		write_parms_CB2()
				print_PS_CB2()

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	XtAppMainLoop()

COPYRIGHT:		University Corporation for Atmospheric Research, 1994
-------------------------------------------------------------------------
*/

#include "define.h"
#include <unistd.h>

#include <Xm/List.h>


static void write_parms_CB2(Widget w, XtPointer client, XtPointer call),
			print_PS_CB2(Widget w, XtPointer client, XtPointer call);


/* -------------------------------------------------------------------- */
void GetDataFileName(Widget w, XtPointer client, XtPointer call)
{
	QueryFile("Enter Data file to read:", DataPath, NewDataFile);

}

/* -------------------------------------------------------------------- */
void ReadParmsFileName(Widget w, XtPointer client, XtPointer call)
{
	QueryFile("Enter Parameter file to read:", UserPath, ReadParmsFile);

}

/* -------------------------------------------------------------------- */
void WriteParmsFileName(Widget w, XtPointer client, XtPointer call)
{
	QueryFile("Enter Parameter ouput file name:", UserPath, write_parms_CB2);

}

static void write_parms_CB2(Widget w, XtPointer client, XtPointer call)
{
	FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
	ExtractFileName(((XmFileSelectionBoxCallbackStruct *)
					call)->value, &parms_file);

	if (access(parms_file, F_OK) == 0)
		{
		sprintf(buffer, "Overwrite file %s", parms_file);
		WarnUser(buffer, WriteParmsFile, NULL);
		}
	else
		WriteParmsFile((Widget)NULL,(XtPointer)NULL,(XtPointer)NULL);

}	/* END WRITE_PARMS_CB2 */

/* -------------------------------------------------------------------- */
void PrintPS(Widget w, XtPointer client, XtPointer call)
{
	if (data_file == NULL)
		{
		HandleError("No data.", Interactive, RETURN);
		return;
		}

	if (client)
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

static void print_PS_CB2(Widget w, XtPointer client, XtPointer call)
{

	FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
	ExtractFileName(((XmFileSelectionBoxCallbackStruct *)
					call)->value, &out_file);

	if (access(out_file, F_OK) == 0)
		{
		sprintf(buffer, "Overwrite file %s", out_file);
		WarnUser(buffer, PrintPostScript, NULL);
		}
	else
		PrintPostScript((Widget)NULL,(XtPointer)NULL,(XtPointer)NULL);

}	/* END PRINT_PS_CB2 */

/* -------------------------------------------------------------------- */
void ToggleAutoScaling(Widget w, XtPointer client, XtPointer call)
{
	AutoScaling = !AutoScaling;

	if (AutoScaling)
		DoAutoScale(NULL, NULL, NULL);

}	/* END TOGGLEAUTOSCALE */

/* -------------------------------------------------------------------- */
void DoAutoScale(Widget w, XtPointer client, XtPointer call)
{
	AutoScale(SCALE_YAXIS);
	YaxisRescaled = TRUE;

	if (Interactive)
		PlotData((Widget)NULL, (XtPointer)NULL,
					(XmDrawingAreaCallbackStruct *)NULL);

}	/* END AUTOSCALE */

/* -------------------------------------------------------------------- */
void ToggleGrid(Widget w, XtPointer client, XtPointer call)
{
	Grid = !Grid;

	if (!Grid)
		DataChanged = TRUE;

	if (Interactive)
		PlotData((Widget)NULL, (XtPointer)NULL,
					(XmDrawingAreaCallbackStruct *)NULL);

}	/* END TOGGLEGRID */

/* -------------------------------------------------------------------- */
void ModifyActiveVars(Widget w, XtPointer client, XtPointer call)
{
	VARTBL	*vp;
	int		position;
	int		*pos_list, pos_cnt;

	if (XmListGetSelectedPos(varList, &pos_list, &pos_cnt) == FALSE)
		return;

	vp = Variable[(position = pos_list[0] - 1)];

	if (vp->Output == TRUE)
		{
		vp->Output = FALSE;

		DeleteVariable(position);
		}
	else
		{
		if (NumberDataSets + 1 > MAXDATASETS)
			{
			ShowError("Out of data sets.", Interactive, RETURN);
			return;
			}

		vp->Output = TRUE;

		AddVariable(position);
		}

	PlotData(NULL, NULL, NULL);

}	/* END MODIFYACTIVEVARS */

/* -------------------------------------------------------------------- */
void DismissWindow(Widget w, XtPointer client, XtPointer call)
{
	XtPopdown(XtParent((Widget)client));
	XtUnmanageChild((Widget)client);

}

/* -------------------------------------------------------------------- */
void Quit(Widget w, XtPointer client, XtPointer call)
{
	exit(0);

}	/* END QUIT */

/* END CCB.C */
