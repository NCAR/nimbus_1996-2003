/*
-------------------------------------------------------------------------
OBJECT NAME:	cb_edit.c

FULL NAME:	Edit Window CallBacks

ENTRY POINTS:	EditVariable()
		GetNewEditVariable()
		ApplyVariableMods()
		DismissEditWindow()
		VerifyLagText()
		MapCalCoe()


STATIC FNS:	set_edit_window_data()

DESCRIPTION:	Contains callbacks for the nimbus GUI.

INPUT:			

OUTPUT:		

REFERENCES:	none

REFERENCED BY:	XtAppMainLoop()

COPYRIGHT:	University Corporation for Atmospheric Research, 1993-2000
-------------------------------------------------------------------------
*/

#include <sys/param.h>

#include <Xm/List.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

#include "nimbus.h"
#include "decode.h"
#include "gui.h"
#include "vardb.h"


static int	VariableType, ListPosition;
static SDITBL	*sp;
static RAWTBL	*rp;
static DERTBL	*dp;

static void set_edit_window_data(
		char	name[],
		bool	output,
		int	outputRate,
		int	lag,
		NR_TYPE	spike,
		char	*dq);

extern char *dataQuality[];

extern Widget slOpMenu;


/* -------------------------------------------------------------------- */
void EditVariable(Widget w, XtPointer client, XmListCallbackStruct *call)
{
  int	i, indx;

  if (w == list1)
    {
    indx = (ListPosition = call->item_position) - 1;

    if (indx < nsdi)
      {
      sp = sdi[indx];
      VariableType = SDI;
      }
    else
    if ((indx -= nsdi) < nraw)
      {
      rp = raw[indx];
      VariableType = RAW;
      }
    else
      {
      dp = derived[indx - nraw];
      VariableType = DERIVED;
      }
    }


  switch (VariableType)
    {
    case SDI:
      set_edit_window_data(sp->name, sp->Output, sp->OutputRate,
                           sp->StaticLag, sp->SpikeSlope, sp->DataQuality);

      for (i = 0; i < sp->order; ++i)
        {
        sprintf(buffer, "%e", sp->cof[i]);
        XmTextFieldSetString(ev_text[i], buffer);
        XtSetSensitive(ev_text[i], TRUE);
        }

      for (; i < MAXDEPEND; ++i)
        {
        XmTextFieldSetString(ev_text[i], "");
        XtSetSensitive(ev_text[i], TRUE);
        }

      break;

    case RAW:
      set_edit_window_data(rp->name, rp->Output, rp->OutputRate,
                           rp->StaticLag, rp->SpikeSlope, rp->DataQuality);

      for (i = 0; i < rp->order; ++i)
        {
        sprintf(buffer, "%e", rp->cof[i]);
        XmTextFieldSetString(ev_text[i], buffer);
        XtSetSensitive(ev_text[i], TRUE);
        }

      for (; i < MAXDEPEND; ++i)
        {
        XmTextFieldSetString(ev_text[i], "");
        XtSetSensitive(ev_text[i], FALSE);
        }

      break;

    case DERIVED:
      set_edit_window_data(dp->name, dp->Output, dp->OutputRate,
			   ERR, 0.0, dp->DataQuality);

      for (i = 0; i < dp->ndep; ++i)
        {
        XmTextFieldSetString(ev_text[i], dp->depend[i]);
        XtSetSensitive(ev_text[i], TRUE);
        }

      for (; i < MAXDEPEND; ++i)
        {
        XmTextFieldSetString(ev_text[i], "");
        XtSetSensitive(ev_text[i], FALSE);
        }
      break;
      }


  XtManageChild(EditWindow);
  XtPopup(XtParent(EditWindow), XtGrabNone);

}	/* END EDITVARIABLE */

/* -------------------------------------------------------------------- */
void ApplyVariableMods(Widget w, XtPointer client, XtPointer call)
{
  int		i;
  bool		output;
  int		outputRate, lag;
  char		*p, *dq;
  float		f;
  NR_TYPE	spike;
  Arg		args[3];
  XmString	new;

  output = XmToggleButtonGetState(outputVarYes);

  if (XmToggleButtonGetState(outputLRbutton))
    outputRate = LOW_RATE;
  else
  if (XmToggleButtonGetState(outputSRbutton))
    outputRate = SAMPLE_RATE;
  else
  if (XmToggleButtonGetState(outputHRbutton))
    outputRate = HIGH_RATE;
  else
    HandleError("Impossible, no outputRate set.");

  p = XmTextFieldGetString(lagText);
  lag = atoi(p);
  XtFree(p);

  p = XmTextFieldGetString(spikeText);
  spike = atof(p);
  XtFree(p);

  /* Extract "Data Quality" */
  XtSetArg(args[0], XmNlabelString, &new);
  XtGetValues(XmOptionButtonGadget(slOpMenu), args, 1);
  XmStringGetLtoR(new, XmSTRING_DEFAULT_CHARSET, &p);
  dq = SearchDataQuality(p);


  switch (VariableType)
    {
    case SDI:
      sp->Dirty = TRUE;
      sp->Output = output;

      if ((sp->OutputRate = outputRate) == 0)
        sp->OutputRate = sp->SampleRate;

      sp->StaticLag = lag;
      sp->SpikeSlope = spike;
      sp->DataQuality = dq;
      sp->order = 0;

      for (i = MAX_COF-1; i >= 0; --i)
        {
        p = XmTextFieldGetString(ev_text[i]);
        f = atof(p);
        XtFree(p);

        if (f == 0.0 && sp->order == 0)
          continue;

        sp->cof[i] = f;
        ++sp->order;
        }

      new = CreateListLineItem((void *)sp, SDI);
      break;

    case RAW:
      rp->Dirty = TRUE;
      rp->Output = output;

      if ((rp->OutputRate = outputRate) == SAMPLE_RATE)
        rp->OutputRate = rp->SampleRate;

      /* PMS1D don't go to 25hz, keep them back at 10. */
      if ((rp->ProbeType & PROBE_PMS1D) && outputRate == HIGH_RATE)
        rp->OutputRate = rp->SampleRate;

      rp->StaticLag = lag;
      rp->SpikeSlope = spike;
      rp->DataQuality = dq;

      for (i = 0; i < rp->order; ++i)
        {
        p = XmTextFieldGetString(ev_text[i]);
        rp->cof[i] = atof(p);
        XtFree(p);
        }

      new = CreateListLineItem((void *)rp, RAW);
      break;

    case DERIVED:
      dp->Dirty = TRUE;
      dp->Output = output;
      dp->OutputRate = outputRate;
      dp->DataQuality = dq;

      /* PMS1D don't go to 25hz, keep them back at 10. */
      if ((dp->ProbeType & PROBE_PMS1D) && outputRate == HIGH_RATE)
        dp->OutputRate = dp->Default_HR_OR;

      for (i = 0; i < dp->ndep; ++i)
        {
        p = XmTextFieldGetString(ev_text[i]);
        strcpy(dp->depend[i], p);
        DependIndexLookup(dp, i);
        XtFree(p);
        }

      new = CreateListLineItem((void *)dp, DERIVED);
      break;
    }

  XmListReplaceItemsPos(list1, &new, 1, ListPosition);
  XmListSelectPos(list1, ListPosition, FALSE);

}	/* END APPLYVARIABLEMODS */

/* -------------------------------------------------------------------- */
void DismissEditWindow(Widget w, XtPointer client, XtPointer call)
{
  XtPopdown(XtParent(EditWindow));
  XtUnmanageChild(EditWindow);
}

/* -------------------------------------------------------------------- */
void VerifyLagText(Widget w, XtPointer client, XmTextVerifyCallbackStruct *call)
{
  int	i;

  for (i = 0; i < call->text->length; ++i)
    if (!isdigit(call->text->ptr[i]))
      {
      call->doit = FALSE;
      return;
      }

}	/* END VERIFYLAGTEXT */

/* -------------------------------------------------------------------- */
static void set_edit_window_data(
		char	name[],
		bool	output,
		int	outputRate,
		int	lag,
		NR_TYPE	spike,
		char	*dq)
{
  int		pos, n;
  Arg		args[3];
  XmString	ns;

  strcpy(buffer, name);
  strcat(buffer, " - ");
  strcat(buffer, VarDB_GetTitle(name));
  ns = XmStringCreateLocalized(buffer);

  n = 0;
  XtSetArg(args[n], XmNlabelString, ns); n++;
  XtSetValues(varNameLabel, args, n);
  XmStringFree(ns);

  XmToggleButtonSetState(outputVarYes, output, FALSE);
  XmToggleButtonSetState(outputVarNo, !output, FALSE);

  XmToggleButtonSetState(outputLRbutton, FALSE, FALSE);
  XmToggleButtonSetState(outputSRbutton, FALSE, FALSE);
  XmToggleButtonSetState(outputHRbutton, FALSE, FALSE);

  switch (outputRate)
    {
    case LOW_RATE:
      XmToggleButtonSetState(outputLRbutton, TRUE, FALSE);
      break;

    case HIGH_RATE:
      XmToggleButtonSetState(outputHRbutton, TRUE, FALSE);
      break;

    default:
      /* Cheesy hack to determnine if derived or raw.  Derived don't have
       * SampleRate....
       */
      if (lag == ERR)
        XmToggleButtonSetState(outputHRbutton, TRUE, FALSE);
      else
        XmToggleButtonSetState(outputSRbutton, TRUE, FALSE);
      break;
    }


  ns = XmStringCreateLocalized(SearchDataQuality(dq));
  XtSetArg(args[0], XmNlabelString, ns);
  XtSetValues(XmOptionButtonGadget(slOpMenu), args, 1);
  XmStringFree(ns);


  if (VariableType == DERIVED)
    {
    XmTextFieldSetString(lagText, "NA");
    XmTextFieldSetString(spikeText, "NA");
    XtSetSensitive(lagText, FALSE);
    XtSetSensitive(spikeText, FALSE);
    XtSetSensitive(outputSRbutton, FALSE);
    }
  else
    {
    XtSetSensitive(lagText, TRUE);
    XtSetSensitive(spikeText, TRUE);
    XtSetSensitive(outputSRbutton, TRUE);

    sprintf(buffer, "%d", lag);
    XmTextFieldSetString(lagText, buffer);
    pos = XmTextFieldGetLastPosition(lagText);
    XmTextFieldSetInsertionPosition(lagText, pos);

    sprintf(buffer, "%.2e", spike);
    XmTextFieldSetString(spikeText, buffer);
    pos = XmTextFieldGetLastPosition(spikeText);
    XmTextFieldSetInsertionPosition(spikeText, pos);
    }

}	/* END SETEDITVAR */

/* -------------------------------------------------------------------- */
void MapCalCof(Widget w, XtPointer client, XtPointer call)
{
  char	*p;

  p = XmTextFieldGetString(w);

  if (VariableType == DERIVED)
    {
    strupr(p);
    XmTextFieldSetString(w, p);
    }
  else
    {
    sprintf(buffer, "%e", atof(p));
    XmTextFieldSetString(w, buffer);
    }

  XtFree(p);

}	/* END MAPCALCOE */

/* END CB_EDIT.C */
