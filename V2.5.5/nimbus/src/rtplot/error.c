/*
-------------------------------------------------------------------------
OBJECT NAME:	error.c

FULL NAME:	Handle Errors

DESCRIPTION:	This procudures displays errors based on wether we are in
		interactive mode or not.  For interactive mode it displays
		an X window, otherwise a fprintf to stderr.  The second
		parameter tells the procedure wether to return or do an
		exit.

INPUT:		Message, Weather to exit or return.

OUTPUT:		Error message.

AUTHOR:		websterc@ncar
-------------------------------------------------------------------------
*/

#include "define.h"

void HandleError(s, interactiv, status)
char	s[];
bool	interactiv;
char	status;
{
	if (!interactiv)
		{
		fprintf(stderr, "%s\n", s);

		if (status == IRET)
			status = EXIT;
		}
	else
		{
		ShowError(s);

		if (status == IRET)
			status = RETURN;
		}

	if (status == RETURN)
		return;
	else
		exit(1);

}	/* END HANDLERROR */

/* END ERROR.C */
