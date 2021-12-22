/*
-------------------------------------------------------------------------
OBJECT NAME:	define.h

FULL NAME:		Include File to Include the Include Files

DESCRIPTION:	
-------------------------------------------------------------------------
*/

#ifndef DEFINE_H
#define DEFINE_H

#define _NO_PROTO

#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>

#include "constants.h"

#define BUFFSIZE	2048
#define PATH_LEN	1024

#define	SPACE		' '

#define MAX_VARIABLES	1024
#define MAXDATASETS		2

#define TITLESIZE		128
#define LEGENDSIZE		(TITLESIZE<<1)

#define NUMTICMARKS		10
#define MAXPOLYDEG		5	/* Maximum degree of polynomial regress	*/

/* Values for "graph_type"		*/
#define LINEGRAPH		0
#define BARGRAPH		1

/* Which Axis?	*/
#define X		0
#define Y		1

#define XI		0x01
#define YI		0x02
#define UI		0x04
#define VI		0x08

/* Values for "axies_to_scale"		*/
#define SCALE_XAXIS		0x01
#define SCALE_YAXIS		0x02

/* Values for "HandleError"		*/
#define RETURN			0	/* Continue after displaying error	*/
#define EXIT			1	/* Exit after displaying error		*/
#define IRET			2	/* if (interactive) return else exit(1)	*/

/* Values for reading Font Resources	*/
#define XtNtitleFont		"titleFont"
#define XtCTitleFont		"Titlefont"
#define XtNsubtitleFont		"subtitleFont"
#define XtCSubtitleFont		"Subtitlefont"
#define XtNticFont			"ticFont"
#define XtCTicFont			"Ticfont"


typedef float	NR_TYPE;


/* Struct for reading font resources from resource file	*/
typedef struct _insRec
	{
	char	*titleFont;
	char	*subtitleFont;
	char	*ticFont;
	} instanceRec;

typedef struct
	{
	char    name[NAMLEN];
	int     inVarID;        /* netCDF variable ID       */
	int     OutputRate;
	int     nPoints;
	} VARTBL;

typedef struct
	{
	float	min;
	float	max;
	float	mean;
	float	sigma;
	float	variance;
	} STAT;


#include "extern.h"

#endif

/* END DEFINE.H */
