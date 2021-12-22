/*
-------------------------------------------------------------------------
OBJECT NAME:	ticlabel.c

FULL NAME:		Generate Various Types of Tic Mark Labels.

ENTRY POINTS:	MakeTimeTicLabel()
				MakeTicLabel()
				MakeLogTicLabel()

STATIC FNS:		none

DESCRIPTION:	

INPUT:		

OUTPUT:		
-------------------------------------------------------------------------
*/

#include "define.h"
#include <string.h>

/* -------------------------------------------------------------------- */
void MakeTicLabel(char buffer[], double diff, double value)
{
	long	i_diff = (long)diff;

	if (diff < 0.01)
		sprintf(buffer, "%f", value);
	else
	if (diff < 0.1)
		sprintf(buffer, "%.4f", value);
	else
	if (diff < 1.0)
		sprintf(buffer, "%.3f", value);
	else
	if (i_diff % 10 == 0)
		sprintf(buffer, "%ld", (long)value);
	else
	if (diff < 10.0)
		sprintf(buffer, "%.2f", value);
	else
	if (diff < 40.0)
		sprintf(buffer, "%.1f", value);
	else
		sprintf(buffer, "%ld", (long)value);

}	/* MakeTicLabel */

/* -------------------------------------------------------------------- */
void MakeLogTicLabel(char buffer[], int value)
{
		sprintf(buffer, "10^%d", value);
}

/* -------------------------------------------------------------------- */
void MakeTimeTicLabel(char buffer[], int indx)
{
	int		nsex, nh, nm, ns;

	nsex = indx * NumberSeconds / numtics;

	nh = nsex / 3600;
	nm = (nsex - (nh * 3600)) / 60;
	ns = nsex - (nh * 3600) - (nm * 60);

	if ((ns += UserStartTime[2]) > 59) { ns -= 60; nm += 1; }
	if ((nm += UserStartTime[1]) > 59) { nm -= 60; nh += 1; }
	if ((nh += UserStartTime[0]) > 23) { nh -= 24; }

	sprintf(buffer, "%02d:%02d:%02d", nh, nm, ns);

}

/* TICLABEL.C */
