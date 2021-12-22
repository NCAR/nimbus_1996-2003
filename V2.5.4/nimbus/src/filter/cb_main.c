/*
-------------------------------------------------------------------------
OBJECT NAME:	cb_main.c

FULL NAME:	Command CallBacks

ENTRY POINTS:	CancelSetup()
		CreateListLineItem()
		FillListWidget()
		LoadSetup()
		LogMessage()
		PrintSetup()
		Proceed()
		Quit()
		SaveSetup()
		SetLowRate()
		SetHighRate()
		StartProcessing()
		ToggleOutput()
		ToggleRate()
		ToggleProbe()

STATIC FNS:	checkForProductionSetup()
		displaySetupWindow()
		readHeader()
		setOutputFileName()
		stopProcessing()
		validateInputFile()
		validateOutputFile()

DESCRIPTION:	Contains callbacks for the nimbus GUI main window & setup
		window.

INPUT:			

OUTPUT:		

REFERENCES:	Everything.

REFERENCED BY:	XtAppMainLoop()

COPYRIGHT:	University Corporation for Atmospheric Research, 1993
-------------------------------------------------------------------------
*/

#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

#include "nimbus.h"
#include "decode.h"
#include "ctape.h"
#include "gui.h"
#include "vardb.h"


static char	ADSfileName[MAXPATHLEN], OutputFileName[MAXPATHLEN];
static char	list1lineFrmt[] = "%-12s %c%c   %4d  %4d   %4d  %8.3f  ";

extern FILE	*LogFile;

/* For wall clock timing	*/
static time_t	startWALL, finishWALL;
static clock_t	startCPU, finishCPU;

extern int	nDefaults;
extern DEFAULT	*Defaults[];

void		FillListWidget();
static int	validateInputFile(), validateOutputFile();

static void	checkForProductionSetup(), displaySetupWindow(),
		setOutputFileName(), readHeader(), stopProcessing();


/* -------------------------------------------------------------------- */
void CancelSetup(Widget w, XtPointer client, XtPointer call)
{
  int		i;

  CloseADSfile();

  for (i = 0; i < nsdi; ++i)
    free((char *)sdi[i]);

  for (i = 0; i < nraw; ++i)
    free((char *)raw[i]);

  for (i = 0; i < nderive; ++i)
    free((char *)derived[i]);

  for (i = 0; i < nDefaults; ++i)
    {
    free((char *)Defaults[i]->Value);
    free((char *)Defaults[i]);
    }

  FreeDataArrays();
  ReleaseFlightHeader();
  free(ProjectName);


  DismissEditWindow(NULL, NULL, NULL);
  DismissTimeSliceWindow(NULL, NULL, NULL);

  XtPopdown(XtParent(SetupWindow));
  XtUnmanageChild(SetupWindow);

  XtSetSensitive(readHeaderButton, TRUE);
  XtSetSensitive(aDSdataText, TRUE);
  XtSetSensitive(outputFileText, TRUE);

  ReleaseVarDB();
  ResetProbeList();
  Initialize();

}	/* END CANCELSETUP */

/* -------------------------------------------------------------------- */
void Proceed(Widget w, XtPointer client, XtPointer call)
{
  char	*p;

  p = XmTextFieldGetString(aDSdataText);
  strcpy(ADSfileName, p);
  XtFree(p);
  p = XmTextFieldGetString(outputFileText);
  strcpy(OutputFileName, p);
  XtFree(p);

  if (validateInputFile() == OK)
    {
    readHeader();

    if (validateOutputFile() == OK)
      displaySetupWindow();
    }

}	/* END PROCEED */

/* -------------------------------------------------------------------- */
static void displaySetupWindow()
{
  XtManageChild(SetupWindow);
  XtPopup(XtParent(SetupWindow), XtGrabNone);

}	/* END DISPLAYSETUPWINDOW */

/* -------------------------------------------------------------------- */
static void readHeader()
{
  XtSetSensitive(readHeaderButton, FALSE);
  XtSetSensitive(aDSdataText, FALSE);
  XtSetSensitive(outputFileText, FALSE);

  XmUpdateDisplay(Shell001);

  if (DecodeHeader(ADSfileName) == ERR) {
    CancelSetup(NULL, NULL, NULL);
    return;
    }

  if (FlightNumber == 0)
    HandleWarning("Flight Number is 0, a new one may be entered\nvia the 'Edit/Flight Info' menu item.", NULL, NULL);

  if (ProductionRun)
    OpenLogFile();

  sprintf(buffer, VARDB, ProjectDirectory, ProjectNumber);
  if (InitializeVarDB(buffer) == ERR) {
    LogMessage("InitializeVarDB for project specific failed, trying master file.\n");

    sprintf(buffer, VARDB, ProjectDirectory, "defaults");
    if (InitializeVarDB(buffer) == ERR) {
      fprintf(stderr, "InitializeVarDB for master file failed, this is fatal.\n");
      exit(1);
      }
    }

  InitAircraftDependencies();
/*  RunAMLIBinitializers(); */

  if (Interactive)
    {
    Arg		args[1];

    CreateProbeMenu();
    FillListWidget();

    sprintf(buffer, "%s - %s, Flight #%d",
		ProjectName, ProjectNumber, FlightNumber);

    XtSetArg(args[0], XmNtitle, buffer);
    XtSetValues(Shell001, args, 1);

    if (LogFile)
      {
      extern Widget logText;
      char	*p = XmTextGetString(logText);

      strcat(buffer, "\n");
      fprintf(LogFile, buffer);

      fprintf(LogFile, p);
      free(p);
      }

    checkForProductionSetup();
    }
  else
    {
    if (ProcessingRate == HIGH_RATE)
			XmToggleButtonSetState(highRateButton, TRUE, TRUE);

    sprintf(buffer, "%s - %s, Flight #%d\n",
		ProjectName, ProjectNumber, FlightNumber);

    LogMessage(buffer);

    checkForProductionSetup();
    LoadSetup_OK(Shell001, NULL, NULL); /* Fake it with any widget name */
    }


	/* Scan VarDB for any non-existent variables.
	 */
{
  FILE	*ofp = LogFile ? LogFile : stderr;
  int	i;

  for (i = 0; i < nsdi; ++i)
    if (VarDB_lookup(sdi[i]->name) == ERR)
      fprintf(ofp, "%s has no description or units.\n", sdi[i]->name);

  for (i = 0; i < nraw; ++i)
    if (VarDB_lookup(raw[i]->name) == ERR)
      fprintf(ofp, "%s has no description or units.\n", raw[i]->name);

  for (i = 0; i < nderive; ++i)
    if (VarDB_lookup(derived[i]->name) == ERR)
      fprintf(ofp, "%s has no description or units.\n", derived[i]->name);
}

}	/* END READHEADER */

/* -------------------------------------------------------------------- */
void SetHighRate(Widget w, XtPointer client, XmToggleButtonCallbackStruct *call)
{
	int		i;

	if (call->set == FALSE)
		return;

	ProcessingRate = HIGH_RATE;

	for (i = 0; i < nsdi; ++i)
		if (sdi[i]->SampleRate >= HIGH_RATE)
			sdi[i]->OutputRate = HIGH_RATE;

	for (i = 0; i < nraw; ++i)
		{
		if (raw[i]->SampleRate >= HIGH_RATE)
			raw[i]->OutputRate = HIGH_RATE;

		if (raw[i]->ProbeType & PROBE_PMS1D)
			raw[i]->OutputRate = raw[i]->SampleRate;
		}

	for (i = 0; i < nderive; ++i)
		derived[i]->OutputRate = derived[i]->Default_HR_OR;

	FillListWidget();
	XtSetSensitive(outputHRbutton, TRUE);

	if (ProductionRun)
		{
		char *p = strchr(OutputFileName, '.');

		if (p)
			{
			strcpy(p, "h.cdf");
			XmTextFieldSetString(outputFileText, OutputFileName);
			}
		}

}	/* END SETHIGHRATE */

/* -------------------------------------------------------------------- */
void SetLowRate(Widget w, XtPointer client, XmToggleButtonCallbackStruct *call)
{
  int		i;

  if (call->set == FALSE)
    return;

  ProcessingRate = LOW_RATE;

  for (i = 0; i < nsdi; ++i)
    sdi[i]->OutputRate = LOW_RATE;

  for (i = 0; i < nraw; ++i)
    raw[i]->OutputRate = LOW_RATE;

  for (i = 0; i < nderive; ++i)
    derived[i]->OutputRate = LOW_RATE;

  FillListWidget();
  XtSetSensitive(outputHRbutton, FALSE);

  if (ProductionRun)
    {
    char *p = strstr(OutputFileName, "h.");

    if (p)
      {
      strcpy(p, ".cdf");
      XmTextFieldSetString(outputFileText, OutputFileName);
      }
    }

}	/* END SETLOWRATE */

/* -------------------------------------------------------------------- */
void StartProcessing(Widget w, XtPointer client, XtPointer call)
{
  XmString	label;
  Arg		args[1];
  int		i, rc;
  long		btim, etim;

  DismissEditWindow(NULL, NULL, NULL);
  DismissTimeSliceWindow(NULL, NULL, NULL);
  XtSetSensitive(list1, FALSE);
  XtSetSensitive(menuBar, FALSE);

  CleanOutUnwantedVariables();
  AllocateDataArrays();
  GenerateComputeOrder();		/* for Derived variables				*/
  RunAMLIBinitializers();
  GetUserTimeIntervals();

  if (ProductionRun)			/* Do it again in case Flight# changed	*/
    setOutputFileName();

  CreateNetCDF(OutputFileName);
  InitAsyncModule(OutputFileName);

  {
  bool	firstSpike = TRUE;

  for (i = 0; i < nsdi; ++i) {
    if (sdi[i]->StaticLag != 0)
      AddVariableToSDIlagList(sdi[i]);

    if (sdi[i]->SpikeSlope > 0) {
      AddVariableToSDIdespikeList(sdi[i]);

      if (firstSpike) {
        LogMessage("Despiking enabled.\n");
        firstSpike = FALSE;
        }
      }

    if (sdi[i]->Output && VarDB_lookup(sdi[i]->name) == ERR && LogFile)
      fprintf(LogFile, "%s has no entry in the VarDB.\n", sdi[i]->name);
    }

  for (i = 0; i < nraw; ++i) {
    if (raw[i]->StaticLag != 0 || raw[i]->DynamicLag != 0)
      AddVariableToRAWlagList(raw[i]);

    if (raw[i]->SpikeSlope > 0) {
      AddVariableToRAWdespikeList(raw[i]);

      if (firstSpike) {
        LogMessage("Despiking enabled.\n");
        firstSpike = FALSE;
        }
      }

    if (raw[i]->Output && VarDB_lookup(raw[i]->name) == ERR && LogFile)
      fprintf(LogFile, "%s has no entry in the VarDB.\n", raw[i]->name);
    }

  for (i = 0; i < nderive; ++i) {
    if (derived[i]->Output && VarDB_lookup(derived[i]->name) == ERR && LogFile)
      fprintf(LogFile,"%s has no entry in the VarDB.\n", derived[i]->name);
    }
  }

  FlushXEvents();

  if (ProcessingRate == HIGH_RATE)
    InitMRFilters();

  /* Turn "Go" button into "Pause" button.
   */
  XtRemoveAllCallbacks(goButton, XmNactivateCallback);
  label = XmStringCreate("Pause", XmFONTLIST_DEFAULT_TAG);
  XtSetArg(args[0], XmNlabelString, label);
  XtSetValues(goButton, args, 1);
  XmStringFree(label);
  XtAddCallback(goButton, XmNactivateCallback, PauseProcessing, NULL);

  FlushXEvents();


  if (Mode == REALTIME)
    {
    NextTimeInterval(&btim, &etim);
    RealTimeLoop();	/* Never to return	*/
    exit(0);
    }


  startWALL	= time(NULL);
  startCPU	= clock();

  while (NextTimeInterval(&btim, &etim))
    {
    switch (ProcessingRate)
      {
      case LOW_RATE:
        rc = LowRateLoop(btim, etim);
        break;

      case HIGH_RATE:
        rc = HighRateLoop(btim, etim);
        break;
      }

    if (PauseWhatToDo == P_QUIT || rc == ERR)
      break;
    }

  finishWALL = time(NULL);
  finishCPU  = clock();

  stopProcessing();

}	/* END STARTPROCESSING */

/* -------------------------------------------------------------------- */
void stopProcessing()
{
  XmString	label;
  Arg			args[1];
  float		x;

  CloseNetCDF();

  LogDespikeInfo();

  /* Log wall clock time.
   */
  x = (float)(finishWALL - startWALL) / 60.0;

  if (x > 60.0)
    sprintf(buffer, "Processing took %.2f hour(s).\n", x / 60.0);
  else
    sprintf(buffer, "Processing took %.1f minutes.\n", x);

  LogMessage(buffer);

/*
  x = (float)(finishCPU - startCPU) / CLOCKS_PER_SEC / 60.0;
  sprintf(buffer, "CPU time took %.1f minutes.\n", x);
  LogMessage(buffer);
*/

  CloseLogFile();

  /* Turn "Pause" button back into "Go" button.
   */
  label = XmStringCreate("Go", XmFONTLIST_DEFAULT_TAG);
  XtSetArg(args[0], XmNlabelString, label);
  XtSetValues(goButton, args, 1);
  XmStringFree(label);
  XtRemoveAllCallbacks(goButton, XmNactivateCallback);
  XtAddCallback(goButton, XmNactivateCallback, StartProcessing, NULL);

  XtSetSensitive(menuBar, TRUE);
  XtSetSensitive(list1, TRUE);

}	/* END STOPPROCESSING */

/* -------------------------------------------------------------------- */
void LoadSetup(Widget w, XtPointer client, XtPointer call)
{
  sprintf(buffer, "%s/nimbus/*", getenv("HOME"));
  QueryFile("Enter file name to load:", buffer, LoadSetup_OK);

}	/* END LOADSETUP */

/* -------------------------------------------------------------------- */
static void checkForProductionSetup()
{
	int		i;
	char	*group[256];

	if (LoadProductionSetupFile == FALSE)
		return;


	/* Check for Flight Groups
	 */
	ReadTextFile(GROUPS, group);

	for (i = 0; group[i]; ++i)
		{
		if (strlen(group[i]) < (size_t)4)
			continue;

		if (atoi(group[i]) == FlightNumber)
			{
			sprintf(buffer, "%s/%s/Production/Flight_%d",
					ProjectDirectory,
					ProjectNumber,
					atoi(strrchr(group[i], '=')+1));

			LoadSetup_OK(NULL, NULL, NULL);

			break;
			}
		}

	FreeTextFile(group);


	/* Check for Production File
	 */
	sprintf(buffer, "%s/%s/Production/Flight_%d",
			ProjectDirectory,
			ProjectNumber,
			FlightNumber);

	LoadSetup_OK(NULL, NULL, NULL);

}	/* END CHECKFORPRODUCTIONSETUP */

/* -------------------------------------------------------------------- */
void SaveSetup(Widget w, XtPointer client, XtPointer call)
{
  if (ProductionRun)
    {
    sprintf(buffer, "%s/%s/Production", ProjectDirectory, ProjectNumber);

    mkdir(buffer, 0775);

    sprintf(buffer, "%s/%s/Production/Flight_%d",
			ProjectDirectory, ProjectNumber, FlightNumber);

    SaveSetup_OK(NULL, NULL, NULL);
    }
  else
    {
    sprintf(buffer, "%s/nimbus/*", getenv("HOME"));
    QueryFile("Enter file name to save setup to:", buffer, SaveSetup_OK);
    }

}	/* END SAVESETUP */

/* -------------------------------------------------------------------- */
void ToggleRate(Widget w, XtPointer client, XtPointer call)
{
	int		*pos_list, pos_cnt = 0;
	int		i, indx;
	XmString	new;

	if (XmListGetSelectedPos(list1, &pos_list, &pos_cnt) == FALSE)
		return;

	for (i = 0; i < pos_cnt; ++i)
		{
		indx = pos_list[i] - 1;

		if (indx >= nsdi+nraw)
			{
			DERTBL	*dp = derived[indx-(nsdi+nraw)];
			dp->Dirty = TRUE;

			switch (dp->OutputRate)
				{
				case LOW_RATE:
					if (ProcessingRate == HIGH_RATE)
						dp->OutputRate = HIGH_RATE;
					break;

				case HIGH_RATE:
					dp->OutputRate = LOW_RATE;
					break;
				}

			new = CreateListLineItem(dp, DERIVED);
			}
		else
		if (indx >= nsdi)
			{
			RAWTBL	*rp = raw[indx-nsdi];

			if (strcmp(rp->name, "HOUR") != 0 && strcmp(rp->name, "MINUTE") != 0
				&& strcmp(rp->name, "SECOND") != 0)
				{
				rp->Dirty = TRUE;

				switch (rp->OutputRate)
					{
					case LOW_RATE:
						rp->OutputRate = rp->SampleRate;
						break;

					case HIGH_RATE:
						rp->OutputRate = LOW_RATE;
						break;

					default:
						if (ProcessingRate == HIGH_RATE)
							rp->OutputRate = HIGH_RATE;
						else
							rp->OutputRate = LOW_RATE;
					}
				}

			new = CreateListLineItem(rp, RAW);
			}
		else
			{
			SDITBL	*sp = sdi[indx];
			sp->Dirty = TRUE;

			switch (sp->OutputRate)
				{
				case LOW_RATE:
					sp->OutputRate = sp->SampleRate;
					break;

				case HIGH_RATE:
					sp->OutputRate = LOW_RATE;
					break;

				default:
					if (ProcessingRate == HIGH_RATE)
						sp->OutputRate = HIGH_RATE;
					else
						sp->OutputRate = LOW_RATE;
				}

			new = CreateListLineItem(sp, SDI);
			}

		XmListReplaceItemsPos(list1, &new, 1, pos_list[i]);
		XmStringFree(new);
		}

	if (pos_cnt == 1)
		XmListSelectPos(list1, pos_list[0], FALSE);

	XtFree((char *)pos_list);

}	/* END TOGGLERATE */

/* -------------------------------------------------------------------- */
void ToggleOutput(Widget w, XtPointer client, XtPointer call)
{
	int		*pos_list, pos_cnt = 0;
	int		i, indx;
	XmString	new;

	if (XmListGetSelectedPos(list1, &pos_list, &pos_cnt) == FALSE)
		return;

	for (i = 0; i < pos_cnt; ++i)
		{
		indx = pos_list[i] - 1;

		if (indx >= nsdi+nraw)
			{
			DERTBL	*dp = derived[indx-(nsdi+nraw)];
			dp->Dirty = TRUE;
			dp->Output = 1 - dp->Output;

			new = CreateListLineItem(dp, DERIVED);
			}
		else
		if (indx >= nsdi)
			{
			RAWTBL	*rp = raw[indx-nsdi];

			if (strcmp(rp->name, "HOUR") != 0 && strcmp(rp->name, "MINUTE") != 0
				&& strcmp(rp->name, "SECOND") != 0)
				{
				rp->Dirty = TRUE;
				rp->Output = 1 - rp->Output;
				}

			new = CreateListLineItem(rp, RAW);
			}
		else
			{
			SDITBL	*sp = sdi[indx];
			sp->Dirty = TRUE;
			sp->Output = 1 - sp->Output;

			new = CreateListLineItem(sp, SDI);
			}

		XmListReplaceItemsPos(list1, &new, 1, pos_list[i]);
		XmStringFree(new);
		}

	if (pos_cnt == 1)
		XmListSelectPos(list1, pos_list[0], FALSE);

	XtFree((char *)pos_list);

}	/* END TOGGLEOUTPUT */

/* -------------------------------------------------------------------- */
void Quit(Widget w, XtPointer client, XtPointer call)
{
  CloseNetCDF();
  CloseRemoveLogFile();
  exit(0);
}

/* -------------------------------------------------------------------- */
static validateInputFile()
{
  if (Mode == REALTIME)
    return(OK);

  if (strlen(ADSfileName) == 0 || access(ADSfileName, R_OK) == ERR)
    {
    HandleError("Non-existent input file.");
    return(ERR);
    }

  return(OK);

}	/* END VALIDATEINPUTFILE */

/* -------------------------------------------------------------------- */
static validateOutputFile()
{
  FILE	*fp;
  struct stat statBuf;

  if (strlen(OutputFileName) == 0 || ProductionRun)
    setOutputFileName();


  /* If it's a directory, then gen up our own name.
   */
  if (stat(OutputFileName, &statBuf) != ERR && S_ISDIR(statBuf.st_mode))
    setOutputFileName();


  /* Make sure we have a .cdf extension.
   */
  if (strcmp(&OutputFileName[strlen(OutputFileName)-4], ".cdf") != 0)
    {
    strcat(OutputFileName, ".cdf");
    XmTextFieldSetString(outputFileText, OutputFileName);
    }


  /* If it doesn't exist, then go away happy.
   */
  if (access(OutputFileName, F_OK) == ERR && errno == ENOENT)
    	return(OK);

  /* If we can't access it for writing, generate perm denied.
   */
  if (access(OutputFileName, W_OK) == ERR && errno == EACCES)
    {
    HandleError("Permission denied on output file.");
    return(ERR);
    }

  /* Else if it exists, warn user.
   */
  HandleWarning("Output file exists.", displaySetupWindow, CancelSetup);
  return(ERR);

}	/* END VALIDATEOUTPUTFILE */

/* -------------------------------------------------------------------- */
static void setOutputFileName()
{
  char	*s;
  char	*p = ProcessingRate == LOW_RATE ? "%srf%02d.cdf" : "%srf%02dh.cdf";

  if (ProductionRun)
    s = getenv("PROD_DATA");
  else
    s = getenv("DATA_DIR");


  if (s) {
    strcpy(OutputFileName, s);
    strcat(OutputFileName, "/");
    }

  sprintf(buffer, p, ProjectNumber, FlightNumber);
  strcat(OutputFileName, buffer);

  XmTextFieldSetString(outputFileText, OutputFileName);

}	/* END SETOUTPUTFILENAME */

/* -------------------------------------------------------------------- */
XmString CreateListLineItem(void *pp, int var_type)
{
	int			i;
	SDITBL		*sp;
	RAWTBL		*rp;
	DERTBL		*dp;
	char		tmp[16];

	switch (var_type)
		{
		case SDI:
			sp = (SDITBL *)pp;

			sprintf(buffer, list1lineFrmt,
				sp->name,
				sp->Dirty ? '*' : ' ',
				sp->Output ? 'Y' : 'N',
				sp->SampleRate,
				sp->OutputRate,
				sp->StaticLag,
				sp->SpikeSlope);

			for (i = 0; i < sp->order; ++i)
				{
				sprintf(tmp, "%10.4f", sp->cof[i]);
				strcat(buffer, tmp);
				}

			break;

		case RAW:
			rp = (RAWTBL *)pp;

			sprintf(buffer, list1lineFrmt,
				rp->name,
				rp->Dirty ? '*' : ' ',
				rp->Output ? 'Y' : 'N',
				rp->SampleRate,
				rp->OutputRate,
				rp->StaticLag,
				rp->SpikeSlope);

			for (i = 0; i < rp->order; ++i)
				{
				sprintf(tmp, "%10.4f", rp->cof[i]);
				strcat(buffer, tmp);
				}

			break;

		case DERIVED:
			dp = (DERTBL *)pp;

			sprintf(buffer, list1lineFrmt,
				dp->name,
				dp->Dirty ? '*' : ' ',
				dp->Output ? 'Y' : 'N',
				0,
				dp->OutputRate,
				0,
				0.0);

			buffer[20] = 'N'; buffer[21] = 'A';
			buffer[33] = 'N'; buffer[34] = 'A';
			memcpy(&buffer[40], "NA    ", 5);

			for (i = 0; i < dp->ndep; ++i)
				{
				if (i > 0)
					strcat(buffer, ", ");

				sprintf(tmp, "%s", dp->depend[i]);
				strcat(buffer, tmp);
				}

			break;
		}

	return(XmStringCreateLocalized(buffer));

}	/* END CREATELISTLINEITEM */

/* -------------------------------------------------------------------- */
void FillListWidget()
{
  int		i, cnt;
  XmString	items[MAX_SDI+MAX_RAW+MAX_DERIVE];

  static int	firstTime = TRUE;

  if (firstTime)
    XmListDeleteAllItems(list1);

  cnt = 0;

  for (i = 0; i < nsdi; ++i)
    items[cnt++] = CreateListLineItem(sdi[i], SDI);

  for (i = 0; i < nraw; ++i)
    items[cnt++] = CreateListLineItem(raw[i], RAW);

  for (i = 0; i < nderive; ++i)
    items[cnt++] = CreateListLineItem(derived[i], DERIVED);


  if (firstTime)
    XmListAddItems(list1, items, cnt, 1);
  else
    XmListReplaceItemsPos(list1, items, cnt, 1);


  for (i = 0; i < cnt; ++i)
    XmStringFree(items[i]);

  if (firstTime)
    {
    XmListSelectPos(list1, 1, FALSE);
    firstTime = FALSE;
    }

}	/* END FILLLISTWIDGET */

/* -------------------------------------------------------------------- */
void PrintSetup(Widget w, XtPointer client, XtPointer call)
{
  int		i, j;
  FILE	*fp;
  SDITBL	*sp;
  RAWTBL	*rp;
  DERTBL	*dp;

  if ((fp = popen("lpr", "w")) == NULL)
    {
    HandleError("PrintList: can't open pipe to 'lpr'");
    return;
    }


  fprintf(fp, "%s - %s, Flight #%d\n\n", ProjectName, ProjectNumber, FlightNumber);

  fprintf(fp, "Name       Output  SR    OR     Lag   Spike Slope\n");
  fprintf(fp, "--------------------------------------------------------------------------------\n");

  for (i = 0; (sp = sdi[i]); ++i)
    {
    fprintf(fp, list1lineFrmt,
			sp->name,
			sp->Dirty ? '*' : ' ',
			sp->Output ? 'Y' : 'N',
			sp->SampleRate,
			sp->OutputRate,
			sp->StaticLag,
			sp->SpikeSlope);

    for (j = 0; j < sp->order; ++j)
      fprintf(fp, "%14e", sp->cof[j]);

    fprintf(fp, "\n");
    }


  for (i = 0; (rp = raw[i]); ++i)
    {
    fprintf(fp, list1lineFrmt,
			rp->name,
			rp->Dirty ? '*' : ' ',
			rp->Output ? 'Y' : 'N',
			rp->SampleRate,
			rp->OutputRate,
			rp->StaticLag,
			rp->SpikeSlope);

    for (j = 0; j < rp->order; ++j)
      fprintf(fp, "%14e", rp->cof[j]);

    fprintf(fp, "\n");
    }


  for (i = 0; (dp = derived[i]); ++i)
    {
    sprintf(buffer, list1lineFrmt,
			dp->name,
			dp->Dirty ? '*' : ' ',
			dp->Output ? 'Y' : 'N',
			0,
			dp->OutputRate,
			0, 0.0);

    buffer[33] = 'N'; buffer[34] = 'A';
    fprintf(fp, buffer);

    for (j = 0; j < dp->ndep; ++j)
      {
      if (j > 0)
        {
        fprintf(fp, ", ");

        if (j % 5 == 0)
          fprintf(fp, "\n\t\t\t\t\t ");
        }

      fprintf(fp, "%s", dp->depend[j]);
      }

    fprintf(fp, "\n");
    }

  fprintf(fp, "\n\nDefaults:\n\n");

  for (i = 0; i < nDefaults; ++i)
    {
    if (Defaults[i]->Used == FALSE)
      continue;

    fprintf(fp, "%s :", Defaults[i]->Name);

    if (Defaults[i]->nValues > 5)
      fprintf(fp, "\n\t");

    for (j = 0; j < Defaults[i]->nValues; ++j)
      {
      if (j > 0 && j % 5 == 0)
        fprintf(fp, "\n\t");

      fprintf(fp, "%14e", Defaults[i]->Value[j]);
      }

    fprintf(fp, "\n");
    }


  pclose(fp);

}	/* END PRINTSETUP */

/* -------------------------------------------------------------------- */
void ToggleProbe(Widget w, XtPointer client, XtPointer call)
{
  int		i,
		cat  = (int)client & 0xf0000000,
		type = (int)client & 0x0ffffff0,
		cnt  = (int)client & 0x0000000f;

  for (i = 0; raw[i]; ++i)
    if ((cat && raw[i]->ProbeType & cat) || (raw[i]->ProbeType & type &&
		raw[i]->ProbeCount == cnt))
      {
      raw[i]->Dirty = TRUE;
      raw[i]->Output = 1 - raw[i]->Output;
      }

  for (i = 0; derived[i]; ++i)
    if ((cat && derived[i]->ProbeType & cat) ||
	(derived[i]->ProbeType & type && derived[i]->ProbeCount == cnt))
      {
      derived[i]->Dirty = TRUE;
      derived[i]->Output = 1 - derived[i]->Output;
      }

  FillListWidget();

}	/* END TOGGLEPROBE */

/* -------------------------------------------------------------------- */
void LogMessage(char msg[])
{
  XmTextPosition	position;
  extern Widget	logText;

  if (Interactive)
    {
    position = XmTextGetInsertionPosition(logText);
    XmTextInsert(logText, position, msg);

    position += strlen(msg);
    XmTextShowPosition(logText, position);
    XmTextSetInsertionPosition(logText, position);
    }
  else
    fprintf(stderr, msg);

  if (LogFile)
    fprintf(LogFile, msg);

}	/* END LOGMESSAGE */

/* END CB_MAIN.C */
