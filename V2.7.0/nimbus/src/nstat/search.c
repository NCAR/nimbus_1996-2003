/*
-------------------------------------------------------------------------
OBJECT NAME:	search.c

FULL NAME:		Searches

ENTRY POINTS:	SearchTable()
				SearchTableSansLocation()

DESCRIPTION:	Search for target in list.  The last pointer in the list
				array must be NULL.  It only compares the first word of
				what the list items points to.

INPUT:			An array of pointers and target pointer.

OUTPUT:			pointer to located item or NULL

REFERENCES:		none

REFERENCED BY:	hdr_decode.c, order.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include <ctype.h>

#include "define.h"


/* -------------------------------------------------------------------- */
SearchTable(table, ntable, target)
char	*table[];
int		ntable;		/* Number in list	*/
char	target[];
{
	register int	beg, mid, end, rc;

	if (ntable == 0)
		return(ERR);

	beg = 0;
	end = ntable - 1;

	do
		{
		mid = (end + beg) >> 1;

		if ((rc = strcmp(target, table[mid])) == 0)
			return(mid);

		if (rc < 0)
			end = mid - 1;
		else
			beg = mid + 1;
		}
	while (beg <= end);

	return(ERR);

}	/* END SEARCHTABLE */

/* -------------------------------------------------------------------- */
SearchTableSansLocation(table, ntable, target)
char	*table[];
int		ntable;		/* Number in list	*/
char	target[];
{
	register int	beg, mid, end, rc;
	char			*p, c;

	if (ntable == 0)
		return(ERR);

	beg = 0;
	end = ntable - 1;

	do
		{
		mid = (end + beg) >> 1;

		if ( (p = strchr(table[mid], '_')) )
			{
			c = *p;
			*p = '\0';
			}

		rc = strcmp(target, table[mid]);

		if (p)
			*p = c;


		if (rc == 0)
			return(mid);

		if (rc < 0)
			end = mid - 1;
		else
			beg = mid + 1;
		}
	while (beg <= end);

	return(ERR);

}	/* END SEARCHTABLESANSLOCATION */

/* END SEARCH.C */
