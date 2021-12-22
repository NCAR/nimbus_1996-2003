/*
-------------------------------------------------------------------------
OBJECT NAME:	hdr_decode.c

FULL NAME:		Decode Header

ENTRY POINTS:	DecodeHeader()

STATIC FNS:		initHDR(), initSDI(), initHoneywell(), initOphir3(),
				initPMS1D(), initPMS1Dv2(), locatePMS(), initLitton51(),
				add_file_to_???TBL(), add_name_to_???TBL(), add_derived_names(),
				check_cal_coes(), initPMS2D()

DESCRIPTION:	Read header & add variables to appropriate table.  There
				are 3 major tables here:
					- SDI table
					- Raw table (From block probes)
					- Derived table

				This module is the brunt end interface between an ADS tape
				and the rest of nimbus.

INPUT:			Header filename.

OUTPUT:			sdi, nsdi, raw, nraw, derived, nderive
				(These globals are initialized in this file)

REFERENCES:		Header API (libhdr_api.a)
				OpenProjectFile(), AccessProjectFile()
				ReadTextFile(), FreeTextFile()
				SearchList(), SearchDERIVEFTNS()
				ReadStaticLags(), SetUpDependencies(), ReadDefaultsFile()
				SortTable()

REFERENCED BY:	winput.c, nimbus.c

COPYRIGHT:		University Corporation for Atmospheric Research, 1992-96
-------------------------------------------------------------------------
*/

#include <stdlib.h>
#include <unistd.h>

#include "raf.h"
#include "nimbus.h"
#include "decode.h"
#include "ctape.h"	/* ADS header API	*/
#include "amlib.h"


typedef struct
	{
	char	*name;
	char	*frmt;
	int		type;	/* #defines used for switch's, in WINDS			*/
	int		pType;	/* probeType for 'ToggleProbe' menu, redundant	*/
	int		cnt;	/* Total # of these probes present.				*/
	} PMS;


static PMS	pms1_probes[] =
	{
	FSSP_STR,	FSSPNAMES,	FSSP,	PROBE_FSSP,	0,
	ASAS_STR,	ASASNAMES,	ASAS,	PROBE_PCASP,0,
	F300_STR,	F300NAMES,	F300,	PROBE_F300,	0,
	X200_STR,	X200NAMES,	X200,	PROBE_200X,	0,
	Y200_STR,	Y200NAMES,	Y200,	PROBE_200Y,	0,
	X260_STR,	X260NAMES,	X260,	PROBE_260X,	0,
	NULL,		NULL,		0,		0,			0
	};

static PMS	pms1v2_probes[] =
	{
	FSSP_STR,	FSSPV2NAMES,FSSP,	PROBE_FSSP,	0,
	PCAS_STR,	PCASPNAMES,	ASAS,	PROBE_PCASP,0,
	F300_STR,	F300NAMES,	F300,	PROBE_F300,	0,
	X200_STR,	X200NAMES,	X200,	PROBE_200X,	0,
	Y200_STR,	Y200NAMES,	Y200,	PROBE_200Y,	0,
	X260_STR,	X260NAMES,	X260,	PROBE_260X,	0,
	NULL,		NULL,		0,		0,			0
	};

static PMS	pms2d_probes[] =
	{
	P2D_C_STR,	NULL,	0,	PROBE_PMS2D,	0,
	P2D_P_STR,	NULL,	0,	PROBE_PMS2D,	0,
	P2D_G_STR,	NULL,	0,	PROBE_PMS2D,	0,
	P2D_H_STR,	NULL,	0,	PROBE_PMS2D,	0,
	NULL,		NULL,	0,	0,				0
	};

/* Global to ease parameter passing in this file */
static int	InertialSystemCount, GPScount;
static int	probeCnt, probeType;
static long	start, rate;
static char	*item_type, location[NAMELEN];
static char	*derivedlist[MAX_DERIVE*2];	/* DeriveNames file	*/


static RAWTBL	*add_name_to_RAWTBL();
static DERTBL	*add_name_to_DERTBL();

static void	add_file_to_RAWTBL(), add_file_to_DERTBL(), initHDR(), initSDI(),
			initHoneywell(), initOphir3(), initPMS1D(), initPMS1Dv2(),
			initLitton51(), add_derived_names(), initPMS2D();


static int	locatePMS();

extern long	INS_start;
extern int	Aircraft;

char	*ExtractHeaderIntoFile(char *);


/* -------------------------------------------------------------------- */
int DecodeHeader(char header_file[])
{
	char	*vn;
	FILE	*fp;
	char	*loc, *p;
	int		i, infd;		/* fd of header source */

	nsdi = nraw = nderive = InertialSystemCount = GPScount = 0;

	for (probeCnt = 0; pms1_probes[probeCnt].name; ++probeCnt)
		{
		pms1_probes[probeCnt].cnt = 0;
		pms1v2_probes[probeCnt].cnt = 0;
		}

	p = ExtractHeaderIntoFile(header_file);
	if (InitFlightHeader(p, CLOSE) == ERR)
		{
		sprintf(buffer, "Header decode failed, taperr = %d.", taperr);
		HandleError(buffer);
		unlink(p);
		free(p);
		return(ERR);
		}

	unlink(p);
	free(p);


	GetAircraft(&p);
	while (*p && !isdigit(*p))
		++p;
	Aircraft = atoi(p);

	GetFlightNumber(&p);
	while (*p && !isdigit(*p))
		++p;
	FlightNumber = atoi(p);

	GetProjectNumber(&ProjectNumber);

	sprintf(buffer, "%s/%s", ProjectDirectory, ProjectNumber);
	if (access(buffer, R_OK) == ERR)
		{
		sprintf(buffer, "No project directory for %s.", ProjectNumber);
		HandleError(buffer);
		return(ERR);
		}


	/* Extract ProjectName
	 */
	if ((fp = OpenProjectFile("%s/%s/ProjectName", "r", RETURN)) != NULL)
		{
		fgets(buffer, 512, fp);
		buffer[strlen(buffer)-1] = '\0';	/* Remove '\n'	*/

		ProjectName = (char *)GetMemory(strlen(buffer)+1);
		strcpy(ProjectName, buffer);
		fclose(fp);
		}
	else
		ProjectName = "";


	/* Old tapes don't set Aircraft field, so fudge it with proj_num.
	 */
/*	if (Aircraft != ELECTRA && Aircraft != KINGAIR && Aircraft != SABRELINER) */
	if (Aircraft == 0)
		{
		switch (ProjectNumber[0])
			{
			case '2':
				Aircraft = KINGAIR;
				break;
			case '7':
				Aircraft = SABRELINER;
				break;
			case '8':
				Aircraft = ELECTRA;
				break;
			}
		}


	SetLookupSuffix((char *)NULL);

	if (Aircraft != NOAA_G4)
		ReadTextFile(DERIVEDNAMES, derivedlist);
	else
		derivedlist[0] = NULL;

	ReadDefaultsFile();


	/* This is the main loop, loop through all variables in header
	 */
	for (vn = GetFirst(); vn; vn = GetNext())
		{
		GetItemType(vn, &item_type);
		GetStart(vn, &start);

		if (GetRate(vn, &rate) == ERR)
			rate = 1;

		probeCnt = 0;
		probeType = 0;
		location[0] = '\0';


		/* Initialize variable/probe into appropriate table(s).
		 */
		if (!strcmp(item_type, SDI_STR) ||
		    !strcmp(item_type, DIGOUT_STR) ||
		    !strcmp(item_type, HSKP_STR))
			{
			initSDI(vn); 
			add_derived_names(vn);
			continue;
			}



		/* Ok, it's not an analog/digital channel, must be a probe.
		 */
		if (!strcmp(item_type, HDR_STR))
			initHDR(vn);
		else
		if (!strcmp(item_type, IRS_STR))
			{
			probeCnt = InertialSystemCount;
			probeType = PROBE_IRS;

			if (GetLocation(vn, &loc) ==  OK)
				{
				location[0] = '_';
				strcpy(&location[1], loc);
				}

			strcpy(buffer, item_type);
			strcat(buffer, location);
			AddProbeToList(buffer, probeType);

			initHoneywell(vn);
			}
		else
		if (!strcmp(item_type, PMS1D_STR))
			{
			probeType = PROBE_PMS1D;

			if (GetLocation(vn, &loc) ==  OK)
				{
				location[0] = '_';
				strcpy(&location[1], loc);
				}

			initPMS1D(vn);
			}
		else
		if (!strcmp(item_type, PMS1V2_STR))
			{
			probeType = PROBE_PMS1D;

			if (GetLocation(vn, &loc) ==  OK)
				{
				location[0] = '_';
				strcpy(&location[1], loc);
				}

			initPMS1Dv2(vn);
			}
		else
		if (!strcmp(item_type, EVNT_STR))
			{
			if (GetLocation(vn, &loc) ==  OK)
				{
				location[0] = '_';
				strcpy(&location[1], loc);
				}

			add_name_to_RAWTBL("EVENT");
			}
		else
		if (!strcmp(item_type, GPS_TRIM_STR) ||
		    !strcmp(item_type, GPS_TANS2_STR) ||
		    !strcmp(item_type, GPS_TANS3_STR))
			{
			probeCnt = GPScount;
			probeType = PROBE_GPS;
			AddProbeToList(item_type, probeType);

			add_file_to_RAWTBL(GPSTANSNAMES);

			if (Aircraft != SAILPLANE)
				{
				add_derived_names(item_type);

				probeCnt = 1;
				probeType = PROBE_GUSTC;
				AddProbeToList("Corrected Winds", PROBE_GUSTC);

				add_derived_names("GUSTC");
				}

			++GPScount;
			}
		else
		if (!strcmp(item_type, PPS_GPS_STR))
			{
			probeCnt = GPScount;
			probeType = PROBE_GPS;
			AddProbeToList(item_type, probeType);

			add_file_to_RAWTBL(GPSPPSNAMES);

			++GPScount;
			}
		else
		if (!strcmp(item_type, HW_GPS_STR))
			{
			probeCnt = GPScount;
			probeType = PROBE_GPS;

			if (GetLocation(vn, &loc) ==  OK)
				{
				location[0] = '_';
				strcpy(&location[1], loc);
				}

			AddProbeToList(item_type, probeType);
			add_file_to_RAWTBL(GPSHWNAMES);

			++GPScount;
			}
		else
		if (!strcmp(item_type, DPRES_STR))
			{
			rate = 5;
			add_name_to_RAWTBL("XPSFD");
			}
		else


		/* Seldom used instruments.
		 */
		if (!strcmp(item_type, OPHIR3_STR))
			{
			probeType = PROBE_OPHIR3;
			AddProbeToList(item_type, probeType);

			initOphir3(vn);
			}
		else
		if (!strcmp(item_type, UVHYG_STR))
			{
			probeType = PROBE_UVHYG;
			AddProbeToList(item_type, probeType);

			rate = sizeof(struct Uv_blk) / sizeof(struct Uv_smp);
			add_file_to_RAWTBL(UVHYGNAMES);
			add_derived_names(item_type);
			}
		else


		/* Pretty much retired instruments, here for backwards compatibility.
		 */
		if (!strcmp(item_type, INS_STR))
			{
			probeCnt = InertialSystemCount;
			probeType = PROBE_IRS;

			AddProbeToList(item_type, probeType);

			initLitton51(vn);
			}
		else
		if (!strcmp(item_type, LRNC_STR))
			{
			add_file_to_RAWTBL(LRNCNAMES);
			add_derived_names(item_type);
			}
		else
		if (!strcmp(item_type, GPS_MAG_STR))
			{
			LogMessage("Support for Magnavox GPS has been dropped.\n");
/*			probeCnt = GPScount;
			probeType = PROBE_GPS;
			AddProbeToList(item_type, probeType);

			add_file_to_RAWTBL(GPSMNAMES);
			add_derived_names(item_type);

			++GPScount;
*/			}
		else
		if (!strcmp(item_type, DME_STR))
			{
			add_file_to_RAWTBL(DMENAMES);
			add_derived_names(item_type);
			}
		else


		/* else we're getting into some esoteric stuff.
		 */
		if (!strcmp(item_type, SER_STR))
			add_name_to_RAWTBL("SERIAL");
		else
		if (!strcmp(item_type, PMS2D_STR))
			{
			if (Mode != REALTIME)
				{
				probeType = PROBE_PMS2D;
				start = 0;
				rate = 1;

				if (GetLocation(vn, &loc) ==  OK)
					{
					location[0] = '_';
					strcpy(&location[1], loc);
					}

				initPMS2D(vn);
				}
			}
		else
		if (!strcmp(item_type, ASYNC_STR))
			{
			if (strcmp(vn, AVAPS_STR) == 0)
				{
				AVAPS = TRUE;
				for (i = 0; i < 4; ++i)
					{
					sprintf(location, "_%02d", i);
					add_file_to_RAWTBL(AVAPSNAMES);
					}
				}
			location[0] = '\0';
			}
		else
			{
			sprintf(buffer, "Unknown variable/probe %s encountered, ignoring & continuing.\n", item_type);
			LogMessage(buffer);
			}
		}


	probeType = 0;
	location[0] = '\0';


	/* Add the default derived variables.
	 */
	rate = 1;

	if (Aircraft != SAILPLANE)
		{
		add_derived_names("ALWAYS");
		AddProbeToList("Fluxes", PROBE_FLUX);
		}
	else
		add_derived_names("SAIL");

	if (Mode == REALTIME)
		{
		for (probeCnt = 0; probeCnt < 3; ++probeCnt)
			{
			sprintf(location, "_%d", probeCnt);
			add_derived_names("PRCLTRK");
			}

		location[0] = '\0';
		}


	if (Aircraft != NOAA_G4)
		FreeTextFile(derivedlist);


	/* Add some final items to 'Toggle Probe' menu.
	 */
	{
	int	cnt, i;

	for (i = 0; pms1_probes[i].name; ++i)
		cnt += pms1_probes[i].cnt + pms1v2_probes[i].cnt;

	if (cnt > 2)
		AddProbeToList("All PMS1D's", PROBE_PMS1D);
	}


	if (AccessProjectFile(USERNAMES, "r") == TRUE)
		add_file_to_DERTBL(USERNAMES);

{
int	i;
/*
for (i = 0; i < nsdi; ++i)
	printf("%-12s%5d\n", sdi[i]->name, sdi[i]->SampleRate);

for (i = 0; i < nraw; ++i)
	printf("%-12s%5d\n", raw[i]->name, raw[i]->xlate);

for (i = 0; i < nderive; ++i)
	printf("%-12s\n", derived[i]->name, derived[i]->compute);
*/
}


	sdi[nsdi] = NULL;
	raw[nraw] = NULL;
	derived[nderive] = NULL;

	SortTable((char **)sdi, 0, nsdi - 1);
	SortTable((char **)raw, 0, nraw - 1);
	SortTable((char **)derived, 0, nderive - 1);

	ReadModuloVariables();
	ReadSumVariables();
	ReadStaticLags();
	ReadDespikeFile();
	ReadDefaultDerivedOutputRate();
	SetUpDependencies();

	if ((i = SearchTable((char **)derived, nderive, "ONE")) != ERR)
		derived[i]->Output = FALSE;
	if ((i = SearchTable((char **)derived, nderive, "ZERO")) != ERR)
		derived[i]->Output = FALSE;

	return(infd);

}	/* END DECODEHEADER */

/* -------------------------------------------------------------------- */
static void initHDR(char vn[])
{
	/* These items must be added to the tables first (i.e. don't
	 * add any variables above the main loop in the fn above).
	 */
	start = 0; rate = 1;
	add_name_to_RAWTBL("HOUR");
	add_name_to_RAWTBL("MINUTE");
	add_name_to_RAWTBL("SECOND");

}	/* END IN_HDR */

/* -------------------------------------------------------------------- */
static void initSDI(char vn[])
{
	SDITBL	*cp;
	char	*type;
	float	*f;

	if (strcmp(vn, "DUMMY") == 0)
		return;

	/* Some words about PSFD.  In the original ADS, 2 variables were entered
	 * into the Sample Table, PSFD & PSFD2.  They were guaranteed to be
	 * adjacent to each other, and so I moved PSFD into the RAWTBL list (see
	 * xlpsfd().  In the new DSM ADS, PSFD was no longer inverted and they
	 * wanted to see the raw output, to complicate things, PSFD1 & PSFD2 were
	 * no longer guaranteed to be adjacent.  So the Sample Table will now have
	 * PSFD1 and PSFD2.  PSFD is placed into the RAWTBL.  CJW - 2/96
	 */

	/* Phase out of PSFD2, PSFD is a 17 bit SDI variable, contained
	 * in 2 variables PSFD2 & PSFD.  To handle it easily, we are
	 * putting it into the raw table.  xlpsfd() will pick up the
	 * high order word correctly.  CJW - 5/93
	 */
	GetADStype(&type);

	if (strcmp(vn, "PSFD2") == 0 && strcmp(type, ADS_TYPE_1_STR) == 0)
		return;


	/* By placing an SDI variable into the 'deriveftns[]' list
	 * (ninc/amlib.fns) the variable will automatically be placed
	 * into the 'raw' list instead of 'sdi' (i.e. if you have an SDI
	 * variable that requires special processing).
	 */
	if (SearchDERIVEFTNS(vn) != ERR)
		{
		RAWTBL	*rp;
		long	offset;

		rp = add_name_to_RAWTBL(vn);

		if (rp->xlate == NULL)
			{
			fprintf(stderr, "DecodeHeader fatal error: A derived variable name has been entered into the Sample Table.\n");
			fprintf(stderr, "Please remove [%s] from the Sample Table.\n", rp->name);
			exit(1);
			}


		if (GetConversionOffset(vn, &(rp->convertOffset)) == ERR ||
			atoi(ProjectNumber) == 818)
			rp->convertOffset = 0;

		GetConversionFactor(vn, &(rp->convertFactor));
		GetOrder(vn, &(rp->order));
		GetCalCoeff(vn, &f);
		memcpy((char *)rp->cof, (char *)f, (int)sizeof(float) * rp->order);
		rp->order = check_cal_coes(rp->order, rp->cof);
		GetSampleOffset(vn, &offset);
		rp->ADSoffset = offset >> 1;


		if (strcmp(rp->name, "HGM") == 0) {
			rp->order = 3;
			rp->cof[2] = 0.0;
			}

		if (strncmp(rp->name, "PSFD", 4) == 0 ||
			strcmp(rp->name, "HGM232") == 0 ||
			strcmp(rp->name, "CHGME") == 0)
			rp->DependedUpon = TRUE;

		if (strcmp(rp->name, "HGM232") == 0) {	/* Status bit for APN-232	*/
			rp = add_name_to_RAWTBL("HGM232S");
			rp->ADSoffset = offset >> 1;
			}

		if (strcmp(vn, "PSFD1") == 0) {
			rp->name[4] = '\0';
			add_derived_names("PSFD");
			}
		else
			return;
		}


	/* Ok, it's strictly nth order polynomial, put it in the SDI table.
	 */
	if (nsdi == MAX_SDI)
		{
		fprintf(stderr, "MAX_SDI reached, modify in nimbus.h and recompile.\n");
		exit(1);
		}

	cp = sdi[nsdi++] = (SDITBL *)GetMemory(sizeof(SDITBL));

	if (GetConversionOffset(vn, &(cp->convertOffset)) == ERR)
		cp->convertOffset = 0;

	GetConversionFactor(vn, &(cp->convertFactor));
	GetSampleOffset(vn, &(cp->ADSoffset));
	GetOrder(vn, &(cp->order));
	GetType(vn, &type);

	strcpy(cp->name, vn);
	cp->SampleRate	= rate;
	cp->OutputRate	= LOW_RATE;
	cp->ADSstart	= start >> 1;
	cp->ADSoffset	>>= 1;
	cp->type		= type[0];
	cp->StaticLag	= 0;
	cp->SpikeSlope	= 0;
	cp->Average		= cp->type == 'C' ? SumSDI : AverageSDI;
	cp->Dirty		= FALSE;
	cp->Modulo		= NULL;
	cp->Output		= TRUE;
	cp->DependedUpon= FALSE;

	if (strncmp(cp->name, "PSFD", 4) == 0)
		cp->DependedUpon = TRUE;

	GetCalCoeff(vn, &f);
	memcpy((char *)cp->cof, (char *)f, (int)sizeof(float) * cp->order);
	cp->order = check_cal_coes(cp->order, cp->cof);

}	/* END IN_SDI */

/* -------------------------------------------------------------------- */
/* NOTE: this proc is assuming the primary IRS probe is stored in the
 * header first.  Currently this is LEFT PIT.  xbuild.c is responsible
 * for insuring this.
 */
static void initHoneywell(char vn[])
{
	int		i;
	RAWTBL	*rp;
	char	*names[50];
	char	name[NAMELEN];

	if (InertialSystemCount == 0)
		location[0] = '\0';

	/* This is how I resolv conflicting names between IRS & INS
	 */
	SetLookupSuffix("_IRS");

	ReadTextFile(IRSNAMES, names);

	for (i = 0; names[i]; ++i)
		{
		sscanf(names[i], "%s %d", name, &rate);

		if ((rp = add_name_to_RAWTBL(name)) == (RAWTBL *)ERR)
			continue;

		if (strncmp(rp->name, "LAG", 3) != 0)
			AddVariableToRAWlagList(rp);
		}


	FreeTextFile(names);

	if (InertialSystemCount++ == 0)
		{
		add_derived_names(item_type);
		SetLookupSuffix((char *)NULL);
		add_derived_names("GUST");
		}
	else
		SetLookupSuffix((char *)NULL);

}	/* END INITHONEYWELL */

/* -------------------------------------------------------------------- */
static void initLitton51(char vn[])
{
	int		i;
	char	*names[50];
	char	name[NAMELEN];

	/* Used by adsIO.c to lag the INS block 1 full second
	 */
	LITTON51_present = TRUE;
	LITTON51_start = start;

	/* This is how I resolve conflicting names between IRS & INS
	 */
	SetLookupSuffix("_INS");

	ReadTextFile(INSNAMES, names);

	for (i = 0; names[i]; ++i)
		{
		sscanf(names[i], "%s %d", name, &rate);
		add_name_to_RAWTBL(name);
		}

	FreeTextFile(names);

	if (InertialSystemCount++ == 0)
		{
		add_derived_names(item_type);
		SetLookupSuffix((char *)NULL);
		add_derived_names("GUST");
		}
	else
		SetLookupSuffix((char *)NULL);

}	/* END IN_INS */

/* -------------------------------------------------------------------- */
static void initOphir3(char vn[])
{
	RAWTBL	*rp;
	int		i, j;
	char	*list[32];

	ReadTextFile(OPHIR3NAMES, list);

	for (i = 0; list[i]; ++i)
		{
		if ((rp = add_name_to_RAWTBL(strtok(list[i], " \t"))) == (RAWTBL *)ERR)
			continue;

		rp->convertOffset	= 0;
		rp->convertFactor	= 1.0;
		rp->order			= atoi(strtok((char *)NULL, " \t"));

		for (j = 0; j < rp->order; ++j)
			rp->cof[j] = (float)atof(strtok((char *)NULL, " \t"));
		}

	FreeTextFile(list);
	add_derived_names(item_type);

}	/* END IN_OPHIR3 */

/* -------------------------------------------------------------------- */
static void initPMS1D(char vn[])
{
	int		i, indx, nbins = 1;
	int		LOVe;		/* Length Of VEctor	*/
	RAWTBL	*rp;
	DERTBL	*dp;
	char	*probe, *names[50];
	char	name[NAMELEN], temp[NAMELEN];


	GetName(vn, &probe);

	if ((indx = locatePMS(probe, pms1_probes)) == ERR)
		{
		sprintf(buffer, "Unknown pms1d probe: %s, continuing\n", probe);
		LogMessage(buffer);
		return;
		}


	probeCnt	= pms1_probes[indx].cnt++ + pms1v2_probes[indx].cnt;
	probeType	|= pms1_probes[indx].pType;

	strcpy(buffer, probe);
	strcat(buffer, location);
	AddProbeToList(buffer, pms1_probes[indx].pType | probeCnt);

	/* For real-time WINDS.  Calls fake1d.c for nimbus.
	 */
	strcpy(temp, probe);
	strcat(temp, "_DEF");
	AddToPMS1DprobeList(probe, location, temp, pms1_probes[indx].type);


	/* Get raw NAMES.
	 */
	SetLookupSuffix("_OLD");
	ReadTextFile(pms1_probes[indx].frmt, names);

	strcpy(temp, "A");	/* Actual	*/
	strcat(temp, probe);

	for (i = 0; names[i]; ++i)
		{
		sscanf(names[i], "%s %d", name, &LOVe);
		if ((rp = add_name_to_RAWTBL(name)) == (RAWTBL *)ERR)
			continue;

		if (strcmp(name, temp) == 0)
			{
			rp->Length	= nbins = LOVe;
			rp->Average	= SumVector;
			}
		}

	FreeTextFile(names);
	SetLookupSuffix((char *)NULL);


{	/* Perform add_derived_names manually	*/

	char	*p;
	char	buff[128];

	temp[0] = 'C';		/* Concentration	*/

	if ( (p = SearchList(derivedlist, probe)) )
		{
		strcpy(buff, p);
		p = strtok(buff, " \t");

		while ( (p = strtok((char *)NULL, " \t")) )
			{
			if ((dp = add_name_to_DERTBL(p)) == (DERTBL *)ERR)
				continue;

			dp->Default_HR_OR = rate;

			/* Make sure CFSSP, C200X, etc, get proper space alloc
			 */
			if (strcmp(p, temp) == 0)
				{
				dp->Length	= nbins;
				}
			}
		}
}
	probeCnt = 0;

}	/* END INITPMS1D */

/* -------------------------------------------------------------------- */
static void initPMS1Dv2(char vn[])
{
	int		i, indx, nbins = 0, LOVe;
	RAWTBL	*rp;
	DERTBL	*dp;
	strnam	*hsk_name;
	char	name[NAMELEN], temp[NAMELEN];
	float	*cals;
	char	*p, *probe, *names[50];
	long	ps_start, ps_length;

	GetName(vn, &probe);


	if ((indx = locatePMS(probe, pms1v2_probes)) == ERR)
		{
		sprintf(buffer, "Unknown pms1d probe: %s, continuing\n", probe);
		LogMessage(buffer);
		return;
		}


	probeCnt	= pms1_probes[indx].cnt + pms1v2_probes[indx].cnt++;
	probeType	|= pms1v2_probes[indx].pType;

	strcpy(buffer, probe);
	strcat(buffer, location);
	AddProbeToList(buffer, pms1_probes[indx].pType | probeCnt);


	GetSerialNumber(vn, &p);
	AddToPMS1DprobeList(probe, location, p, pms1v2_probes[indx].type);


	/* Add Housekeeping variables to RAWTBL
	 */
	GetHouseKeepingNames(vn, (char **)&hsk_name);
	GetCalCoeff(vn, &cals);

	for (i = 0; i < P1DV2_AUX; ++i)
		{
		if (strcmp(hsk_name[i], "DUMMY") == 0 || strcmp(hsk_name[i], "") == 0)
			continue;

		if ((rp = add_name_to_RAWTBL(hsk_name[i])) == (RAWTBL *)ERR)
			continue;

		rp->order		= P1DV2_COF;

		memcpy( (char *)rp->cof,
				(char *)&cals[i * P1DV2_COF],
				(int)sizeof(float) * P1DV2_COF);
		}


	/* Read raw variable names from *.names
	 */
	ReadTextFile(pms1v2_probes[indx].frmt, names);

	strcpy(temp, "A");	/* Actual	*/
	strcat(temp, probe);

	for (i = 0; names[i]; ++i)
		{
		sscanf(names[i], "%s %d", name, &LOVe);

		if ((rp = add_name_to_RAWTBL(name)) == (RAWTBL *)ERR)
			continue;

		if (strcmp(name, temp) == 0)
			{
			rp->Length	= nbins = LOVe;
			rp->Average	= SumVector;
			}
		}

	pms1v2_probes[indx].cnt++;

	FreeTextFile(names);


	/* Check for Particle Spacing and add.
	 */
	GetParticleSpacingStart(vn, &ps_start);
	GetParticleSpacingLength(vn, &ps_length);

	if (ps_start > 0 && ps_length > 0)
		{
		temp[0] = 'P';

		if ((rp = add_name_to_RAWTBL(temp)) != (RAWTBL *)ERR)
			{
			rp->ADSstart	= ps_start >> 1;
			rp->Length		= (ps_length / rate) >> 1;
			rp->Average		= SumVector;
			}
		}


	/* Read in derived names
	 */
	strcpy(name, probe);
	if (strcmp(probe, "FSSP") == 0)
		strcat(name, "V2");


{	/* Perform add_derived_names manually	*/
	char	*p;
	char	buff[128];

	temp[0] = 'C';

	if ( (p = SearchList(derivedlist, name)) )
		{
		strcpy(buff, p);
		p = strtok(buff, " \t");

		while ( (p = strtok((char *)NULL, " \t")) )
			{
			if ((dp = add_name_to_DERTBL(p)) == (DERTBL *)ERR)
				continue;

			dp->Default_HR_OR = rate;

			/* Make sure CFSSP, C200X, etc, get proper space alloc
			 */
			if (strcmp(p, temp) == 0)
				{
				dp->Length	= nbins;
				}
			}
		}
}
	probeCnt = 0;

}	/* END INITPMS1DV2 */

/* -------------------------------------------------------------------- */
static void initPMS2D(char vn[])
{
	int		indx;
	RAWTBL	*rp;

	if ((indx = locatePMS(vn, pms2d_probes)) == ERR)
		{
		sprintf(buffer, "Unknown pms2d probe: %s, continuing\n", vn);
		LogMessage(buffer);
		return;
		}

	probeCnt = pms2d_probes[indx].cnt++;

	rp = add_name_to_RAWTBL(vn);

    rp->Output	= FALSE;
	rp->Average	= SumVector;
	rp->Length	= 3;	/* (Offset, ItemLength, nItems)	*/

	probeCnt = 0;

}	/* END INITPMS2D */


/* -------------------------------------------------------------------- */
/* Header Decode Atomic functions										*/
/* -------------------------------------------------------------------- */
static void add_derived_names(char name[])
{
	char	*p;
	char	buff[512];

	/* Find variable in derived list and add ALL associated names
	 * to derived table.
	 */
	if ( (p = SearchList(derivedlist, name)) )
		{
		strcpy(buff, p);
		p = strtok(buff, " \t");

		while ( (p = strtok((char *)NULL, " \t")) )
			add_name_to_DERTBL(p);
		}

}	/* END ADD_DERIVED_NAMES */

/* -------------------------------------------------------------------- */
static void add_file_to_DERTBL(char filename[])
{
	FILE	*fp;

	fp = OpenProjectFile(filename, "r", EXIT);

	while (fscanf(fp, "%s", buffer) != EOF)
		if (buffer[0] != COMMENT)
			add_name_to_DERTBL(buffer);

	fclose(fp);

}	/* END ADD_FILE_TO_DERTBL */

/* -------------------------------------------------------------------- */
static void add_file_to_RAWTBL(char filename[])
{
	FILE	*fp;

	fp = OpenProjectFile(filename, "r", EXIT);

	while (fscanf(fp, "%s", buffer) != EOF)
		if (buffer[0] != COMMENT)
			add_name_to_RAWTBL(buffer);

	fclose(fp);

}	/* END ADD_FILE_TO_RAWTBL */

/* -------------------------------------------------------------------- */
static RAWTBL *add_name_to_RAWTBL(char name[])
{
	int		indx;
	RAWTBL	*rp;

	if ((indx = SearchDERIVEFTNS(name)) == ERR)
		{
		sprintf(buffer, "Throwing away %s, has no decode function.\n", name);
		LogMessage(buffer);
		return((RAWTBL *)ERR);
		}

	if (nraw == MAX_RAW)
		{
		fprintf(stderr, "MAX_RAW reached, fix in nimbus.h and recompile.");
		exit(1);
		}

	rp = raw[nraw++] = (RAWTBL *)GetMemory(sizeof(RAWTBL));

	strcpy(rp->name, name);
	if (*location)
		strcat(rp->name, location);

	rp->xlate			= deriveftns[indx].xlate;

	rp->ADSstart		= start >> 1;
	rp->ADSoffset		= 1;
	rp->SampleRate		= rate;
	rp->OutputRate		= LOW_RATE;
	rp->Length			= 1;
	rp->convertOffset	= 0;
	rp->convertFactor	= 1.0;
	rp->order			= 0;
	rp->StaticLag		= 0;
	rp->SpikeSlope		= 0;
	rp->DynamicLag		= 0;

	rp->Dirty			= FALSE;
	rp->Average			= Average;
	rp->Modulo			= NULL;
	rp->Output			= TRUE;
	rp->DependedUpon	= FALSE;
	rp->ProbeType		= probeType;
	rp->ProbeCount		= probeCnt;

	return(rp);

}	/* END ADD_NAME_TO_RAWTBL */

/* -------------------------------------------------------------------- */
static DERTBL *add_name_to_DERTBL(char name[])
{
	int		indx;
	DERTBL	*dp;

	if ((indx = SearchDERIVEFTNS(name)) == ERR)
		{
		sprintf(buffer, "Throwing away %s, has no compute function.\n", name);
		LogMessage(buffer);
		return((DERTBL *)ERR);
		}

	if (nderive == MAX_DERIVE)
		{
		fprintf(stderr,"MAX_DERIVE reached, modify in nimbus.h and recompile.");
		exit(1);
		}


	/* Eliminate chemistry vars that are computed via second pass program.
	 * (Can be used for any variable).
	 */
	if (Mode == REALTIME && deriveftns[indx].compute == smissval)
		return((DERTBL *)ERR);


	dp = derived[nderive++] = (DERTBL *)GetMemory(sizeof(DERTBL));

	strcpy(dp->name, name);
	if (*location)
		strcat(dp->name, location);

	dp->compute		= deriveftns[indx].compute;

	dp->OutputRate	= LOW_RATE;
	dp->Default_HR_OR= HIGH_RATE;
	dp->Length		= 1;
	dp->ProbeType	= probeType;
	dp->ProbeCount	= probeCnt;

	/* As a kludge, .xlate field used as ProbeCount for FLUX variables.
	 */
	if (strncmp(name, "FLX", 3) == 0)
		{
		dp->ProbeCount = (int)deriveftns[indx].xlate;
		dp->ProbeType = PROBE_FLUX;
		}

	dp->ndep		= 0;
	dp->Dirty		= FALSE;
	dp->Modulo		= NULL;
	dp->Output		= TRUE;
	dp->DependedUpon= FALSE;

	return(dp);

}	/* END ADD_NAME_TO_DERTBL */

/* -------------------------------------------------------------------- */
static int locatePMS(char target[], PMS list[])
{
	int		i;

	for (i = 0; list[i].name; ++i)
		if (strcmp(target, list[i].name) == 0)
			return(i);

	return(ERR);

}	/* END LOCATEPMS */

/* -------------------------------------------------------------------- */
/* Strip out trailing 0 cal coe's
 */
check_cal_coes(int order, float *coef)
{
	do
		{
		--order;
		}
	while (order >= 0 && coef[order] == 0.0);

	return(order + 1);

}	/* END CHECK_CAL_COES */

/* END HDR_DECODE.C */
