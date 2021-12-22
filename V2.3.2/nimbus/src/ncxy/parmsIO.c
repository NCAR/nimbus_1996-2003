/*
-------------------------------------------------------------------------
OBJECT NAME:	parmsIO.c

FULL NAME:		Parameter File IO

ENTRY POINTS:	ReadParmsFile()
				WriteParmsFile()

STATIC FNS:		none

DESCRIPTION:	These 2 procedures read and write the parameter file to
				and from disk.

INPUT:			none

OUTPUT:			none

AUTHOR:			websterc@ncar
-------------------------------------------------------------------------
*/

#include "define.h"
#include <string.h>

/* --------------------------------------------------------------------- */
void ReadParmsFile(w, clientData, callData)
Widget		w;
XtPointer	clientData, callData;
{
	FILE	*fp;
	char	*value;
	bool	error_ctl, ymin_set, xmin_set, xmax_set, ymax_set;

	xmin_set = xmax_set = ymin_set = ymax_set = FALSE;

	/* If we are interactiive and being called from main then the X
	 * window has not been created so we need to fake HandlError into
	 * thinking we are not interactive.
	 */
	if (w)
		error_ctl = Interactive;
	else
		error_ctl = FALSE;

	/* Get the file name if this was a callback from the menu
	 */
	if (callData)
		{
		FileCancel((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

		ExtractFileName(((XmFileSelectionBoxCallbackStruct *)
			callData)->value, &parms_file);
		}

	if ((fp = fopen(parms_file, "r")) == NULL)
		{
		sprintf(buffer, "Can't open parameter file %s",
								parms_file);

		HandleError(buffer, error_ctl, IRET);
		return;
		}


	/* Check each line for a key word
	 */
	while (fgets(buffer, BUFFSIZE, fp))
		{
		if (*buffer == '#' || (value = strchr(buffer, '=')) == NULL)
			continue;

		*value++ = '\0';
		value[strlen(value)-1] = '\0';

		while (*value == SPACE || *value == '\t')
			++value;

		if (strcmp(buffer, kw_legend) == 0)
			{
			int		indx;
			char	*p;

			if ((p = strtok(value, ",")) == NULL)
				continue;

			do
				{
				if ((indx = SearchTable(Variable, nVariables, p)) == ERR)
					{
					sprintf(&buffer[500], "Variable %s not in file.", p);
					ShowError(&buffer[500]);
					}
				else
					if (vpX == NULL)
						vpX = Variable[indx];
					else
						vpY = Variable[indx];
				}
			while ((p = strtok(NULL, ",")));

			continue;
			}

		if (strcmp(buffer, kw_title) == 0)
			{
			strcpy(title, value);
			continue;
			}

		if (strcmp(buffer, kw_subtitle) == 0)
			{
			strcpy(subtitle, value);
			continue;
			}

		if (strcmp(buffer, kw_xlabel) == 0)
			{
			strcpy(xlabel, value);
			continue;
			}

		if (strcmp(buffer, kw_ylabel) == 0)
			{
			strcpy(ylabel, value);
			continue;
			}

		if (strcmp(buffer, kw_xmin) == 0)
			{
			sscanf(value, "%2d:%2d:%2d", &UserStartTime[0],
					&UserStartTime[1], &UserStartTime[2]);
			xmin_set = TRUE;
			continue;
			}

		if (strcmp(buffer, kw_xmax) == 0)
			{
			sscanf(value, "%2d:%2d:%2d", &UserEndTime[0],
					&UserEndTime[1], &UserEndTime[2]);
			xmax_set = TRUE;
			continue;
			}

		if (strcmp(buffer, kw_ymin) == 0)
			{
			ymin = atof(value);
			ymin_set = TRUE;
			continue;
			}

		if (strcmp(buffer, kw_ymax) == 0)
			{
			ymax = atof(value);
			ymax_set = TRUE;
			continue;
			}
		}

}	/* END READPARMSFILE */

/* --------------------------------------------------------------------- */
/* ARGSUSED */
void WriteParmsFile(w, clientData, callData)
Widget		w;
XtPointer	clientData, callData;
{
	int		i;
	FILE	*fp;

	if (callData)
		CancelWarning((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

	if ((fp = fopen(parms_file, "w")) == NULL)
		{
		sprintf(buffer, "parmsIO: can't open %s", parms_file);

		HandleError(buffer, Interactive, RETURN);
		return;
		}

	fprintf(fp, "%s=%s\n", kw_title, title);
	fprintf(fp, "%s=%s\n", kw_subtitle, subtitle);
	fprintf(fp, "%s=%s\n", kw_xlabel, xlabel);
	fprintf(fp, "%s=%s\n", kw_ylabel, ylabel);
	fprintf(fp, "%s=%s,%s\n", kw_legend, vpX->name, vpY->name);

	fprintf(fp, "%s=%02d:%02d:%02d\n", kw_xmin, UserStartTime[0],
						UserStartTime[1], UserStartTime[2]);
	fprintf(fp, "%s=%02d:%02d:%02d\n", kw_xmax, UserEndTime[0], UserEndTime[1],
								UserEndTime[2]);
	fprintf(fp, "%s=%lf\n", kw_ymin, ymin);
	fprintf(fp, "%s=%lf\n", kw_ymax, ymax);

	fclose(fp);

}	/* END WRITEPARMSFILE */

/* END PARMSIO.C */
