/*
-------------------------------------------------------------------------
OBJECT NAME:	misc.c

FULL NAME:	Calculate Spacing & Bar Width

DESCRIPTION:	

INPUT:		

OUTPUT:		
-------------------------------------------------------------------------
*/

#include "define.h"
#include <string.h>

/* -------------------------------------------------------------------- */
MakeTicLabel(buffer, diff, value)
char	buffer[];	/* Place to put Tic Label	*/
double	diff;		/* max - min for axis		*/
double	value;		/* Actual value for Tic label	*/
{
	long	i_diff = (long)diff;

	if (diff < 0.01)
		sprintf(buffer, "%lf", value);
	else
	if (diff < 0.1)
		sprintf(buffer, "%.4lf", value);
	else
	if (diff < 1.0)
		sprintf(buffer, "%.3lf", value);
	else
	if (i_diff % 10 == 0)
		sprintf(buffer, "%ld", (long)value);
	else
	if (diff < 10.0)
		sprintf(buffer, "%.2lf", value);
	else
	if (diff < 40.0)
		sprintf(buffer, "%.1lf", value);
	else
		sprintf(buffer, "%ld", (long)value);

}	/* MakeTicLabel */

/* MISC.C */
