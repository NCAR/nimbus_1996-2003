/*
-------------------------------------------------------------------------
OBJECT NAME:	basic.c

FULL NAME:	Basic Read Skeleton

ENTRY POINTS:	main()

STATIC FNS:	none

DESCRIPTION:	

INPUT:		Nimbus netCDF file.

OUTPUT:		

REFERENCES:	libraf.a

REFERENCED BY:	User

COMPILE:	cc -I/home/local/include basic.c -lnetcdf -lraf

COPYRIGHT:	University Corporation for Atmospheric Research, 1993
-------------------------------------------------------------------------
*/

#include <netcdf.h>
#include <stdio.h>

#include "constants.h"		/* Check /home/local/include		*/

#define MAX_VARS	2000


typedef struct
	{
	char	Name[NAMLEN];
	int	varID;
	int	nDims;
	int	dimIDs[3];
	int	nAtts;
	float	*data;
	} VAR_INFO;


VAR_INFO	*Variable[MAX_VARS];


char	buffer[4096];		/* Generic volatile string space	*/


/* -------------------------------------------------------------------- */
main(argc, argv)
int	argc;
char	*argv[];
{
	int	i, j, StartRecord, EndRecord;
	int	InputFile;
	int	nDims, nVars, nGAtts, nRecords, TimeDimension;
	float	*data_p[MAX_VARS];



	if (argc < 2)
		{
		fprintf(stderr, "Usage: skel netcdf_file\n");
		exit(1);
		}

	/* Open Input File
	 */
	if ((InputFile = ncopen(argv[1], NC_NOWRITE)) == ERR)
		{
		fprintf(stderr, "Can't open %s.\n", argv[1]);
		return(1);
		}

	ncinquire(InputFile, &nDims, &nVars, &nGAtts, &TimeDimension);
	ncdiminq(InputFile, TimeDimension, NULL, &nRecords);


	/* Set up Variable information allocate data space.
	 */
	{
	int	cnt, VarID[MAX_NC_VARS], sizes[MAX_NC_VARS];

	ncrecinq(InputFile, &nVars, VarID, sizes);


	for (i = 0, cnt = 0; i < nVars; ++i)
		{
		Variable[cnt] = (VAR_INFO *)GetMemory(sizeof(VAR_INFO));

		Variable[cnt]->varID = VarID[i];

		ncvarinq(InputFile,	Variable[cnt]->varID,
					Variable[cnt]->Name,
					(nc_type *)NULL,
					&Variable[cnt]->nDims,
					Variable[cnt]->dimIDs,
					&Variable[cnt]->nAtts);

		/* Eliminate unwanted variables, if any.
		 */
		if (	strcmp(Variable[cnt]->Name, "HOUR") != 0 &&
			strcmp(Variable[cnt]->Name, "MINUTE") != 0 &&
			strcmp(Variable[cnt]->Name, "SECOND") != 0 &&
			strcmp(Variable[cnt]->Name, "LAT") != 0)
			continue;

		Variable[cnt]->data = data_p[i] = (float *)GetMemory(sizes[i]);
		++cnt;
		}

	nVars = cnt;
	}



	/* Start reading records of data and user processing.
	 */
	StartRecord	= 60;
	EndRecord	= 70;

	for (i = StartRecord; i < EndRecord; ++i)
		{
		ncrecget(InputFile, (long)i, data_p);

		printf("%d:%d:%d - %f\n",	(int)Variable[0]->data[0],
						(int)Variable[2]->data[0],
						(int)Variable[3]->data[0],
						Variable[1]->data[0]);
		}


	ncclose(InputFile);

	return(0);

}	/* END MAIN */

/* END BASIC.C */
