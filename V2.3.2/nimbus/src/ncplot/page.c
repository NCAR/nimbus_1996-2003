/*
-------------------------------------------------------------------------
OBJECT NAME:	page.c

FULL NAME:		Command CallBacks

ENTRY POINTS:	PageForward()
				PageBackward()

STATIC FNS:		none

DESCRIPTION:	

REFERENCES:	

REFERENCED BY:	Callback

COPYRIGHT:		University Corporation for Atmospheric Research, 1996
-------------------------------------------------------------------------
*/

#include "define.h"


static int EOFreached = FALSE, SavedNumberSeconds;

/* -------------------------------------------------------------------- */
void PageForward(Widget w, XtPointer client, XtPointer call)
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

	if (memcmp((char *)st, (char *)FileEndTime, sizeof(st)) >= 0)
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
	else
		EOFreached = FALSE;

	ReadData(NULL, NULL, NULL);

	if (AutoScaling)
		AutoScale(SCALE_YAXIS);

	PlotData(NULL, NULL, NULL);

}	/* END PAGEFORWARD */

/* -------------------------------------------------------------------- */
void PageBackward(Widget w, XtPointer client, XtPointer call)
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

	ReadData(NULL, NULL, NULL);
	if (AutoScaling)
		AutoScale(SCALE_YAXIS);
	PlotData(NULL, NULL, NULL);

}	/* END PAGEBACKWARD */

/* END PAGE.C */
