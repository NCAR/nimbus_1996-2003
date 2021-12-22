/*
-------------------------------------------------------------------------
OBJECT NAME:	netcdf.c

FULL NAME:		NetCDF IO

ENTRY POINTS:	ReadInputFile()
				CloseNetCDF()

STATIC FNS:		none

DESCRIPTION:	This file has the routines necassary to Create and write
				data for distribution of NCAR/RAF aircraft data in netCDF
				format.

INPUT:			

OUTPUT:			none

REFERENCES:		none

REFERENCED BY:	LowRateLoop()

COPYRIGHT:		University Corporation for Atmospheric Research, 1993
-------------------------------------------------------------------------
*/

#include <errno.h>
#include <time.h>

#include "define.h"
#include "netcdf.h"

/* -------------------------------------------------------------------- */
ReadInputFile(fileName)
char	fileName[];
{
	VARTBL	*vp;
	int		i, indx, len, nVars, nDims, dimIDs[3], recDim;
	char	name[NAMELEN];

	ncopts = 0;
	nVariables = 0;


	/* Open Input File
	 */
	if ((InputFile = ncopen(fileName, NC_NOWRITE)) == ERR)
		{
		sprintf(buffer, "Can't open %s.\n", fileName);
		HandleError(buffer);
		return(ERR);
		}

	ncinquire(InputFile, NULL, &nVars, NULL, &recDim);
	ncdiminq(InputFile, recDim, (char *)NULL, &nRecords);


	ncattinq(InputFile, NC_GLOBAL, "ProjectName", (nc_type *)0, &len);
	ProjectName = GetMemory(len);
	ncattget(InputFile, NC_GLOBAL, "ProjectName", ProjectName);

	ncattinq(InputFile, NC_GLOBAL, "ProjectNumber", (nc_type *)0, &len);
	ProjectNumber = GetMemory(len);
	ncattget(InputFile, NC_GLOBAL, "ProjectNumber", ProjectNumber);

	ncattinq(InputFile, NC_GLOBAL, "FlightNumber", (nc_type *)0, &len);
	FlightNumber = GetMemory(len);
	ncattget(InputFile, NC_GLOBAL, "FlightNumber", FlightNumber);


	for (i = 2; i < nVars; ++i)
		{
		ncvarinq(InputFile, i, name, NULL, &nDims, dimIDs, NULL);

		vp = Variable[nVariables++] = (VARTBL *)GetMemory(sizeof(VARTBL));

		strcpy(vp->name, name);
		ncattget(InputFile, i, "units", (void *)vp->Units);
		ncattget(InputFile, i, "SampledRate", (void *)&vp->SampleRate);
		ncattget(InputFile, i, "OutputRate", (void *)&vp->OutputRate);

		vp->Output	= FALSE;
		vp->inVarID	= i;

		if (ncattget(InputFile, i, "VectorLength", (void *)&vp->VectorLength)
																	== ERR)
			vp->VectorLength = 1;
		}

	SortTable(Variable, 0, nVariables - 1);

	return(OK);

}	/* END READINPUTFILE */

/* -------------------------------------------------------------------- */
void CloseNetCDF()
{
	ncclose(InputFile);

}	/* END CLOSENETCDF */

/* END NETCDF.C */
