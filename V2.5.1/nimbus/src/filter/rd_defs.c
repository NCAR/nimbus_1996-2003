/*
-------------------------------------------------------------------------
OBJECT NAME:	rd_defs.c

FULL NAME:		Read Defaults Object

ENTRY POINTS:	ReadDefaultsFile()
				GetDefaultsValue()

STATIC FNS:		process_line()

DESCRIPTION:	The Defaults file contains constants for various probe
				calculations.

INPUT:			none

OUTPUT:			

REFERENCES:		OpenProjectFile()

REFERENCED BY:	hdr_decode.c, cb_main(), various AMLIB fn's.

COPYRIGHT:		University Corporation for Atmospheric Research, 1992
-------------------------------------------------------------------------
*/

#include <stdio.h>

#include "nimbus.h"
#include "decode.h"

#define BUFF_SIZE		256
#define TOKENS			"\t "
#define ARRAY_TOKENS	", \t\n"


extern int		nDefaults;
extern DEFAULT	*Defaults[];

static NR_TYPE	zero = 0.0;

static void	process_line();

char		*strupr();


/* -------------------------------------------------------------------- */
void ReadDefaultsFile()
{
	FILE	*fp;

	nDefaults = 0;

	fp = OpenProjectFile(DEFAULTS, "r", EXIT);

	while (fgets(buffer, BUFF_SIZE, fp) != NULL)
		{
		if (buffer[0] == COMMENT || strlen(buffer) < (size_t)3)
			continue;

		process_line(buffer, fp);
		}

	fclose(fp);

}	/* END READDEFAULTSFILE */

/* -------------------------------------------------------------------- */
NR_TYPE *GetDefaultsValue(char target[])
{
	int		i;
 
	for (i = 0; i < nDefaults; ++i)
		if (strcmp(Defaults[i]->Name, target) == 0)
            {
            Defaults[i]->Used = TRUE;
            return(Defaults[i]->Value);
            }
 
	sprintf(buffer, "Request for non-existent default, %s.\n", target);
	LogMessage(buffer);
 
	return(&zero);
 
}	/* END GETDEFAULTSVALUE */

/* -------------------------------------------------------------------- */
static void process_line(char *line_p, FILE *fp)
{
	int		i;
	char	*name, *nxt, *is_array;

	Defaults[nDefaults] = (DEFAULT *)GetMemory(sizeof(DEFAULT));

	name = strupr(strtok(line_p, TOKENS));
	is_array = strchr(name, '[');

	if (is_array)
		{
		Defaults[nDefaults]->nValues = atoi(is_array+1);

		*is_array = '\0';
		strcpy(Defaults[nDefaults]->Name, name);
		*is_array = '[';
		}
	else
		{
		Defaults[nDefaults]->nValues = 1;
		strcpy(Defaults[nDefaults]->Name, name);
		}

	Defaults[nDefaults]->Dirty	= FALSE;
	Defaults[nDefaults]->Used	= FALSE;
	Defaults[nDefaults]->Value	= (NR_TYPE *)GetMemory(NR_SIZE *
											Defaults[nDefaults]->nValues);

	if (is_array)
		{
		for (i = 0; i < Defaults[nDefaults]->nValues; ++i)
			{
			nxt = strtok((char *)NULL, ARRAY_TOKENS);

			if (nxt == NULL || nxt[0] == COMMENT)
				{
				fgets(line_p, BUFF_SIZE, fp);
				nxt = strtok(line_p, ARRAY_TOKENS);
				}

			Defaults[nDefaults]->Value[i] = (float)atof(nxt);
			}
		}
	else
		Defaults[nDefaults]->Value[0] =
								(float)atof(strtok((char *)NULL, TOKENS));

	++nDefaults;

}	/* END PROCESS_LINE */

/* END RD_DEFS.C */
