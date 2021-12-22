/*
-------------------------------------------------------------------------
OBJECT NAME:	ccb.c

FULL NAME:	Command CallBacks

ENTRY POINTS:	

STATIC FNS:	none

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	XtAppMainLoop()

COPYRIGHT:	University Corporation for Atmospheric Research, 1994
-------------------------------------------------------------------------
*/

#include "define.h"
#include <unistd.h>


/* -------------------------------------------------------------------- */
void Freeze(Widget w, XtPointer client, XtPointer call)
{
  Frozen = !Frozen;

}	/* END FREEZE */

/* -------------------------------------------------------------------- */
void DoAutoScale(Widget w, XtPointer client, XtPointer call)
{
  AutoScale(SCALE_YAXIS);

  PlotData((Widget)NULL, (XtPointer)NULL,(XmDrawingAreaCallbackStruct *)NULL);

}	/* END AUTOSCALE */

/* -------------------------------------------------------------------- */
void ToggleUnits(Widget w, XtPointer client, XtPointer call)
{
  switch ((Units = (int)client))
    {
    case COUNTS:
      strcpy(ylabel, "Counts");
      break;

    case VOLTS:
      strcpy(ylabel, "Volts");
      break;

    case ENGINEERING:
      strcpy(ylabel, "Engineering");
      break;
    }

  SetYLabelPixmap();

}	/* END TOGGLECOUNTS */

/* -------------------------------------------------------------------- */
void ToggleFlash(Widget w, XtPointer client, XtPointer call)
{
  FlashMode = !FlashMode;

}	/* END TOGGLEFLASH */

/* -------------------------------------------------------------------- */
void ToggleGrid(Widget w, XtPointer client, XtPointer call)
{
  Grid = !Grid;
  PlotData((Widget)NULL, (XtPointer)NULL,(XmDrawingAreaCallbackStruct *)NULL);

}	/* END TOGGLEGRID */

/* -------------------------------------------------------------------- */
void ModifyActiveVars(Widget w, XtPointer client, XtPointer call)
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
void Quit(Widget w, XtPointer client, XtPointer call)
{
  exit(0);

}	/* END QUIT */

/* END CCB.C */
