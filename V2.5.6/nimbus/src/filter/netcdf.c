/*
-------------------------------------------------------------------------
OBJECT NAME:	netcdf.c

FULL NAME:	NetCDF IO

ENTRY POINTS:	CreateNetCDF()
		WriteNetCDF()
		WriteNetCDF_MRF()
		SyncNetCDF()
		CloseNetCDF()
		SetBaseTime()
		QueueMissingData()
		WriteMissingRecords()

STATIC FNS:	none

DESCRIPTION:	This file has the routines necassary to Create and write
		data for distribution of NCAR/RAF aircraft data in netCDF
		format.

INPUT:			

OUTPUT:		none

REFERENCES:	pms1d.c

REFERENCED BY:	LowRateLoop(), HighRateLoop()

COPYRIGHT:	University Corporation for Atmospheric Research, 1993-8
-------------------------------------------------------------------------
*/

#include <errno.h>
#include <stdio.h>
#include <time.h>

#include "nimbus.h"
#include "decode.h"
#include "gui.h"
#include "ctape.h"
#include "netcdf.h"
#include "queue.h"
#include "vardb.h"

#define DEFAULT_TI_LENGTH	(19 * MAX_TIME_SLICES)

struct missDat	/* (Time gap) / (missing data) information */
	{
	int		hour;
	int		minute;
	int		second;
	int		nRecords;
	} ;

static int		fd = -1;
static int		baseTimeID;
static struct tm	StartFlight;
static void		*data_p[MAX_SDI+MAX_RAW+MAX_DERIVE];
static long		recordNumber = 0;
static float		TimeOffset = 0.0;

static Queue	*missingRecords;
static void	WriteMissingRecords();

extern NR_TYPE	*SampledData, *AveragedData, *HighRateData;
extern FILE	*LogFile;

int	FlightDate[3]; /* HACK: for amlib/xlate/time.c */


/* -------------------------------------------------------------------- */
void SetBaseTime(struct Hdr_blk *hdr)
{
  struct tm		*gt;
  char			s1[10], s2[10];

  static bool	firstTime = TRUE;
  static time_t	BaseTime;

  StartFlight.tm_hour	= ntohs(hdr->hour);
  StartFlight.tm_min	= ntohs(hdr->minute);
  StartFlight.tm_sec	= ntohs(hdr->second);
  StartFlight.tm_isdst	= -1;

  /* Account for circular buffer spin up in [lr|hr]loop.c
   */
  if (Mode != REALTIME)
    if (ProcessingRate == LOW_RATE)
      StartFlight.tm_sec += 2;
    else
      StartFlight.tm_sec += 14;


  if (firstTime)
    {
    firstTime = FALSE;

    BaseTime = mktime(&StartFlight);
    gt = gmtime(&BaseTime);

    sprintf(s1, "%04d%02d%02d", StartFlight.tm_year + 1900,
    StartFlight.tm_mon,
    StartFlight.tm_mday);

    sprintf(s2, "%04d%02d%02d", gt->tm_year + 1900, gt->tm_mon, gt->tm_mday);

    if (strcmp(s2, s1) > 0)
      gt->tm_hour += 24;
    else
      if (strcmp(s2, s1) < 0)
        gt->tm_hour -= 24;

    StartFlight.tm_hour += StartFlight.tm_hour - gt->tm_hour;

    BaseTime = mktime(&StartFlight);
    ncvarput1(fd, baseTimeID, NULL, (void *)&BaseTime);
    }
  else
    {
    time_t	NewTime;

    NewTime = mktime(&StartFlight);
    gt = gmtime(&NewTime);

    sprintf(s1, "%04d%02d%02d", StartFlight.tm_year + 1900,
            StartFlight.tm_mon, StartFlight.tm_mday);

    sprintf(s2, "%04d%02d%02d", gt->tm_year + 1900, gt->tm_mon, gt->tm_mday);

    if (strcmp(s2, s1) > 0)
      gt->tm_hour += 24;
    else
      if (strcmp(s2, s1) < 0)
        gt->tm_hour -= 24;

    StartFlight.tm_hour += StartFlight.tm_hour - gt->tm_hour;

    NewTime = mktime(&StartFlight);
    TimeOffset = NewTime - BaseTime;
    }

}	/* END SETBASETIME */

/* -------------------------------------------------------------------- */
void CreateNetCDF(char fileName[])
{
  int		i, j, indx, catIdx;
  SDITBL	*sp;
  RAWTBL	*rp;
  DERTBL	*dp;

  int	timeOffsetID;
  int	ndims, dims[3], TimeDim,
	LowRateDim, HighRateDim, Dim2Hz, Dim5Hz, Dim10Hz, Dim50Hz, Dim250Hz,
	Dim1000Hz, Vector16Dim, Vector32Dim, Vector64Dim,
	AsyncDim;

  char	*p;
  float	missing_val = MISSING_VALUE;


  fd = nccreate(fileName, NC_CLOBBER);

  if (ProductionRun)
    chmod(fileName, 0644);
  else
    chmod(fileName, 0666);


  /* Dimensions.
   */
  TimeDim	= ncdimdef(fd, "Time", NC_UNLIMITED);
  LowRateDim	= ncdimdef(fd, "sps1", 1);

  Dim2Hz = Dim5Hz = Dim10Hz = HighRateDim = Dim50Hz = Dim250Hz = Dim1000Hz =
  AsyncDim = Vector16Dim = Vector32Dim = Vector64Dim = ERR;


  /* Global Attributes.
   */
  strcpy(buffer, "NCAR Research Aviation Facility");
  ncattput(fd,NC_GLOBAL, "Source", NC_CHAR, strlen(buffer)+1, (void *)buffer);

  strcpy(buffer, "P.O. Box 3000, Boulder, CO 80307-3000");
  ncattput(fd,NC_GLOBAL, "Address", NC_CHAR, strlen(buffer)+1, (void *)buffer);

  strcpy(buffer, "(303) 497-1030");
  ncattput(fd,NC_GLOBAL, "Phone", NC_CHAR, strlen(buffer)+1, (void *)buffer);

  strcpy(buffer, "NCAR-RAF/nimbus");
  ncattput(fd, NC_GLOBAL, "Conventions", NC_CHAR,
           strlen(buffer)+1, (void *)buffer);

  strcpy(buffer, "1.1");
  ncattput(fd, NC_GLOBAL, "Version", NC_CHAR, strlen(buffer)+1, (void *)buffer);


  if (!ProductionRun)
    {
    strcpy(buffer, "This file contains PRELIMINARY DATA that are NOT to be used for critical analysis.");

    ncattput(fd, NC_GLOBAL, "WARNING", NC_CHAR,
		strlen(buffer)+1, (void *)buffer);
    }


  {
  time_t	t;
  struct tm	tm;

  t = time(0);
  tm = *gmtime(&t);
  strftime(buffer, 128, "%h %d %R UTC %Y", &tm);
  ncattput(fd, NC_GLOBAL, "DateProcessed", NC_CHAR,
  strlen(buffer)+1, (void *)buffer);

  if (LogFile)
    fprintf(LogFile, "Processed on: %s\n", buffer);
  }

  if (ProjectName)
    {
    ncattput(fd, NC_GLOBAL, "ProjectName", NC_CHAR,
    strlen(ProjectName)+1, (void *)ProjectName);
    }

  GetAircraft(&p);
  ncattput(fd, NC_GLOBAL, "Aircraft", NC_CHAR, strlen(p)+1, (void *)p);

  GetProjectNumber(&p);
  ncattput(fd, NC_GLOBAL, "ProjectNumber", NC_CHAR, strlen(p)+1, (void *)p);

  GetFlightNumber(&p);
  strupr(strcpy(buffer, p));
  ncattput(fd, NC_GLOBAL, "FlightNumber", NC_CHAR, strlen(p)+1, (void *)p);

  if (Mode == REALTIME)
    {
    time_t	x = time(NULL);
    gmtime_r(&x, &StartFlight);
    StartFlight.tm_year += 1900;  /* will be subtracted off later	*/
    p = "UTC";
    }
  else
    {
    GetHeaderDate(&p);

    sscanf(p, "%d/%d/%d", &StartFlight.tm_mon, &StartFlight.tm_mday, &StartFlight.tm_year);
    FlightDate[0] = StartFlight.tm_mon;  /* HACK: for amlib/xlate/time.c */
    FlightDate[1] = StartFlight.tm_mday;
    FlightDate[2] = StartFlight.tm_year;

    strcpy(buffer, p);
    GetTimeZone(&p);
    }


  if (strlen(p)) {
    strcat(buffer, " ");
    strcat(buffer, p);
    }

  ncattput(fd, NC_GLOBAL, "FlightDate", NC_CHAR, strlen(buffer)+1, (void *)buffer);

  if (LogFile)
    fprintf(LogFile, "Flight Date: %s\n", buffer);


  /* Will be updated later.
   */
  memset(buffer, ' ', DEFAULT_TI_LENGTH);
  ncattput(fd, NC_GLOBAL, "TimeInterval", NC_CHAR, DEFAULT_TI_LENGTH, (void *)buffer);


  /* Write out Categories.
   */
  {
  char *list[128];

  catIdx = VarDB_GetCategoryList(list);

  buffer[0] = '\0';

  for (i = 1; i < catIdx; ++i)	/* Skip category "None"	*/
    {
    strcat(buffer, list[i]);
    if (i != catIdx - 1)
      strcat(buffer, ",");
    }

  ncattput(fd, NC_GLOBAL, "Categories", NC_CHAR, strlen(buffer)+1, (void *)buffer);
  }


  /* Write out Defaults as Global Attrribute
   */
/*
  {
  char	temp[64];

  extern int		nDefaults;
  extern DEFAULT	*Defaults[];

  buffer[0] = '\0';

  for (i = 0; i < nDefaults; ++i)
    if (Defaults[i]->Used)
      {
      if (i > 0)
        strcat(buffer, " : ");

      sprintf(temp, "%s=", Defaults[i]->Name);
      strcat(buffer, temp);

      for (j = 0; j < Defaults[i]->nValues; ++j)
        {
        if (j > 0)
          strcat(buffer, ",");

        sprintf(temp, "%e", Defaults[i]->Value[j]);
        strcat(buffer, temp);
        }
      }

  ncattput(fd, NC_GLOBAL, "Defaults", NC_CHAR, strlen(buffer)+1, buffer);
  }
*/


  /* First dimension is time dimension.
   * Second is Rate in Hz.
   * Third is Vector Length.
   */
  dims[0] = TimeDim;


  /* Time Variables.  WARNING: Many programs downstream of here are
   * expecting these two variables to be first in the netCDF file.
   */
  baseTimeID = ncvardef(fd, "base_time", NC_LONG, 0, 0);
  strcpy(buffer, "Seconds since Jan 1, 1970.");
  ncattput(fd, baseTimeID, "long_name", NC_CHAR, strlen(buffer)+1, buffer);
  StartFlight.tm_year -= 1900;
  StartFlight.tm_mon -= 1;

  timeOffsetID = ncvardef(fd, "time_offset", NC_FLOAT, 1, dims);
  strcpy(buffer, "Seconds since base_time.");
  ncattput(fd, timeOffsetID, "long_name", NC_CHAR, strlen(buffer)+1, buffer);


  /* SDI variables.
   */
  indx = 1;	/* Index for data_p	*/

  /* For each variable:
   *	- Set dimensions
   *	- define variable
   *	- Set attributes
   *	- Set data_pointer
   */
  for (i = 0; i < nsdi; ++i)
    {
    if ((sp = sdi[i])->Output == FALSE)
      continue;

    ndims = 2;

    switch (sp->OutputRate)
      {
      case 1:
        ndims = 1;
        break;

      case 5:
        if (Dim5Hz == ERR)
          Dim5Hz = ncdimdef(fd, "sps5", 5);

        dims[1] = Dim5Hz;
        break;

      case 25:
        if (HighRateDim == ERR)
          HighRateDim = ncdimdef(fd, "sps25", 25);

        dims[1] = HighRateDim;
        break;

      case 50:
        if (Dim50Hz == ERR)
          Dim50Hz = ncdimdef(fd, "sps50", 50);

        dims[1] = Dim50Hz;
        break;

      case 250:
        if (Dim250Hz == ERR)
          Dim250Hz = ncdimdef(fd, "sps250", 250);

        dims[1] = Dim250Hz;
        break;

      case 1000:
        if (Dim1000Hz == ERR)
          Dim1000Hz = ncdimdef(fd, "sps1000", 1000);

        dims[1] = Dim1000Hz;
        break;

      default:
        sprintf(buffer, "Variable %s has unsupported output rate of %d, setting to 1.\n", sp->name, sp->OutputRate);
        LogMessage(buffer);
        sp->OutputRate = 1;
        ndims = 1;
      }


    sp->varid = ncvardef(fd, sp->name, NC_FLOAT, ndims, dims);

    p = VarDB_GetUnits(sp->name);
    ncattput(fd, sp->varid, "units", NC_CHAR, strlen(p)+1, p);
    p = VarDB_GetTitle(sp->name);
    ncattput(fd, sp->varid, "long_name", NC_CHAR, strlen(p)+1, p);

    p = VarDB_GetCategoryName(sp->name);
    ncattput(fd, sp->varid, "Category", NC_CHAR, strlen(p)+1, p);

    ncattput(fd, sp->varid, "missing_value", NC_FLOAT, 1, &missing_val);
    ncattput(fd, sp->varid, "SampledRate", NC_LONG, 1, &sp->SampleRate);
    ncattput(fd, sp->varid, "OutputRate", NC_LONG, 1, &sp->OutputRate);
    ncattput(fd, sp->varid, "CalibrationCoefficients", NC_FLOAT,
             sp->order, sp->cof);


    if (sp->OutputRate == LOW_RATE)
      data_p[indx++] = (void *)&AveragedData[sp->LRstart];
    else
    if (sp->OutputRate == sp->SampleRate)
      data_p[indx++] = (void *)&SampledData[sp->SRstart];
    else
      data_p[indx++] = (void *)&HighRateData[sp->HRstart];
    }



  /* Raw/"block probe" variables.
   */
  for (i = 0; i < nraw; ++i)
    {
    if ((rp = raw[i])->Output == FALSE)
      continue;

    ndims = 2;

    switch (rp->OutputRate)
      {
      case 1:
        dims[1] = LowRateDim;
        ndims = 1;
        break;

      case 5:
        if (Dim5Hz == ERR)
          Dim5Hz = ncdimdef(fd, "sps5", 5);

        dims[1] = Dim5Hz;
        break;

      case 10:
        if (Dim10Hz == ERR)
          Dim10Hz = ncdimdef(fd, "sps10", 10);

        dims[1] = Dim10Hz;
        break;

      case 25:
        if (HighRateDim == ERR)
          HighRateDim = ncdimdef(fd, "sps25", 25);

        dims[1] = HighRateDim;
        break;

      case 50:
        if (Dim50Hz == ERR)
          Dim50Hz = ncdimdef(fd, "sps50", 50);

        dims[1] = Dim50Hz;
        break;

      default:
        sprintf(buffer, "Variable %s has unsupported output rate of %d, setting to 1.\n", rp->name, rp->OutputRate);
        LogMessage(buffer);
        rp->OutputRate = 1;
        ndims = 1;
      }


    if (rp->Length > 1)
      {
      ndims = 3;

      if (rp->Length == 3)	/* Async vector	*/
        {
        if (AsyncDim == ERR)
          AsyncDim = ncdimdef(fd, "Async", 3);

        dims[2] = AsyncDim;
        }
      else
      if (rp->Length <= 16)
        {
        if (Vector16Dim == ERR)
          Vector16Dim = ncdimdef(fd, "Vector16", 16);

        dims[2] = Vector16Dim;
        }
      else
      if (rp->Length <= 32)
        {
        if (Vector32Dim == ERR)
          Vector32Dim = ncdimdef(fd, "Vector32", 32);

        dims[2] = Vector32Dim;
        }
      else
        {
        if (Vector64Dim == ERR)
          Vector64Dim = ncdimdef(fd, "Vector64", 64);

        dims[2] = Vector64Dim;
        }
      }


    rp->varid = ncvardef(fd, rp->name, NC_FLOAT, ndims, dims);

    p = VarDB_GetUnits(rp->name);
    ncattput(fd, rp->varid, "units", NC_CHAR, strlen(p)+1, p);
    p = VarDB_GetTitle(rp->name);
    ncattput(fd, rp->varid, "long_name", NC_CHAR, strlen(p)+1, p);

    p = VarDB_GetCategoryName(rp->name);
    ncattput(fd, rp->varid, "Category", NC_CHAR, strlen(p)+1, p);

    ncattput(fd, rp->varid, "missing_value", NC_FLOAT, 1, &missing_val);
    ncattput(fd, rp->varid, "SampledRate", NC_LONG, 1, &rp->SampleRate);
    ncattput(fd, rp->varid, "OutputRate", NC_LONG, 1, &rp->OutputRate);
    ncattput(fd, rp->varid, "VectorLength", NC_LONG, 1, &rp->Length);
    if (rp->order > 0)
      ncattput(fd, rp->varid, "CalibrationCoefficients", NC_FLOAT,
               rp->order, rp->cof);

    if (rp->Length > 3)
      AddPMS1dAttrs(fd, rp);

    CheckAndAddAttrs(fd, rp->varid, rp->name);

    if (rp->OutputRate == LOW_RATE)
      data_p[indx++] = (void *)&AveragedData[rp->LRstart];
    else
    if (rp->OutputRate == rp->SampleRate && rp->OutputRate != ProcessingRate)
      data_p[indx++] = (void *)&SampledData[rp->SRstart];
    else
      data_p[indx++] = (void *)&HighRateData[rp->HRstart];
    }



  /* Derived variables.
   */
  for (i = 0; i < nderive; ++i)
    {
    if ((dp = derived[i])->Output == FALSE)
      continue;

    ndims = 2;

    switch (dp->OutputRate)
      {
      case 1:
        dims[1] = LowRateDim;
        ndims = 1;
        break;

      case 5:
        if (Dim5Hz == ERR)
          Dim5Hz = ncdimdef(fd, "sps5", 5);

        dims[1] = Dim5Hz;
        break;

      case 10:
        if (Dim10Hz == ERR)
          Dim10Hz = ncdimdef(fd, "sps10", 10);

        dims[1] = Dim10Hz;
        break;

      case 25:
        if (HighRateDim == ERR)
          HighRateDim = ncdimdef(fd, "sps25", 25);

        dims[1] = HighRateDim;
        break;

      case 50:
        if (Dim50Hz == ERR)
          Dim50Hz = ncdimdef(fd, "sps50", 50);

        dims[1] = Dim50Hz;
        break;

      case 250:
        if (Dim250Hz == ERR)
          Dim250Hz = ncdimdef(fd, "sps250", 250);

        dims[1] = Dim250Hz;
        break;

      default:
        sprintf(buffer, "Variable %s has unsupported output rate of %d, setting to 1.\n", dp->name, dp->OutputRate);
        LogMessage(buffer);
        dp->OutputRate = 1;
        ndims = 1;
      }


    if (dp->Length > 1)
      {
      ndims = 3;

      if (dp->Length <= 16)
        {
        if (Vector16Dim == ERR)
          Vector16Dim = ncdimdef(fd, "Vector16", 16);

        dims[2] = Vector16Dim;
        }
      else
      if (dp->Length <= 32)
        {
        if (Vector32Dim == ERR)
          Vector32Dim = ncdimdef(fd, "Vector32", 32);

        dims[2] = Vector32Dim;
        }
      else
        {
        if (Vector64Dim == ERR)
          Vector64Dim = ncdimdef(fd, "Vector64", 64);

        dims[2] = Vector64Dim;
        }
      }


    dp->varid = ncvardef(fd, dp->name, NC_FLOAT, ndims, dims);

    p = VarDB_GetUnits(dp->name);
    ncattput(fd, dp->varid, "units", NC_CHAR, strlen(p)+1, p);
    p = VarDB_GetTitle(dp->name);
    ncattput(fd, dp->varid, "long_name", NC_CHAR, strlen(p)+1, p);

    p = VarDB_GetCategoryName(dp->name);
    ncattput(fd, dp->varid, "Category", NC_CHAR, strlen(p)+1, p);

    ncattput(fd, dp->varid, "missing_value", NC_FLOAT, 1, &missing_val);
    ncattput(fd, dp->varid, "OutputRate", NC_LONG, 1, &dp->OutputRate);
    ncattput(fd, dp->varid, "VectorLength", NC_LONG, 1, &dp->Length);

    sprintf(buffer, "%d", dp->ndep);
    for (j = 0; j < dp->ndep; ++j)
      {
      strcat(buffer, " ");
      strcat(buffer, dp->depend[j]);
      }

    ncattput(fd, dp->varid, "Dependencies",NC_CHAR,strlen(buffer)+1,buffer);

    CheckAndAddAttrs(fd, dp->varid, dp->name);

    if (dp->OutputRate == LOW_RATE)
      data_p[indx++] = (void *)&AveragedData[dp->LRstart];
    else
      data_p[indx++] = (void *)&HighRateData[dp->HRstart];
    }

  data_p[0] = (void *)&TimeOffset;

  ncendef(fd);

}	/* END CREATENETCDF */

/* -------------------------------------------------------------------- */
void WriteNetCDF()
{
  struct missDat	*dp;
  static int		errCnt = 0;

  if (ncrecput(fd, recordNumber, data_p) == ERR)
    {
    if (errno == ENOSPC)
      {
      LogMessage("Disk full, closing file.\n");
      CloseNetCDF();
      PauseFlag = TRUE;
      PauseWhatToDo = P_QUIT;

      return;
      }

    fprintf(stderr,
            "WriteNetCDF: ncrecput failure, RecordNumber = %d, errno = %d\n",
            recordNumber, errno);

    if (++errCnt > 8)
      {
      fprintf(stderr,
              "Too many write errors, closing file and exiting...\n");
      CloseNetCDF();
      exit(1);
      }
    }

  TimeOffset += 1.0;
  ++recordNumber;

  if ( (dp = FrontQueue(missingRecords)) )
    {
    int		hour, min, sec;

    hour = SampledData[raw[SearchTable((char **)raw, nraw, "HOUR")]->SRstart];
    min = SampledData[raw[SearchTable((char **)raw, nraw, "MINUTE")]->SRstart];
    sec = SampledData[raw[SearchTable((char **)raw, nraw, "SECOND")]->SRstart];

    if (hour == dp->hour && min == dp->minute && sec == dp->second)
      WriteMissingRecords();
    }

}	/* END WRITENETCDF */

/* -------------------------------------------------------------------- */
void WriteNetCDF_MRF()
{
  int		i, indx = 1;
  SDITBL	*sp;
  RAWTBL	*rp;


  /* We need to reset SampleRate indices, because for HighRate, SampleData
   * is from a circular buffer.
   */
  for (i = 0; i < nsdi; ++i)
    if ((sp = sdi[i])->Output)
      {
      if (sp->OutputRate == sp->SampleRate)
        data_p[indx] = (void *)&SampledData[sp->SRstart];

      ++indx;
      }

  for (i = 0; i < nraw; ++i)
    if ((rp = raw[i])->Output)
      {
      if (rp->OutputRate == rp->SampleRate && rp->OutputRate != ProcessingRate)
        data_p[indx] = (void *)&SampledData[rp->SRstart];

      ++indx;
      }


  WriteNetCDF();

}	/* END WRITENETCDF_MRF */

/* -------------------------------------------------------------------- */
void QueueMissingData(int h, int m, int s, int nRecords)
{
  struct missDat	*dp;
  static int		firstTime = TRUE;

  if (firstTime)
    {
    missingRecords = CreateQueue();
    firstTime = FALSE;
    }

  dp = (struct missDat *)GetMemory(sizeof(struct missDat));

  dp->hour = h;
  dp->minute = m;
  dp->second = s;
  dp->nRecords = nRecords;

  EnQueue(missingRecords, (void *)dp);

}	/* END QUEUEMISSINGDATA */

/* -------------------------------------------------------------------- */
static void WriteMissingRecords()
{
  int		i, ind = 1;
  NR_TYPE	*d, hour, minute, second;
  void	*ldp[MAX_SDI+MAX_RAW+MAX_DERIVE];
  struct missDat	*dp;

  dp = FrontQueue(missingRecords);
  d = (float *)GetMemory(sizeof(float) * 1000);
  /* 1000 is fastest sampling rate */

  for (i = 0; i < 1000; ++i)
    d[i] = MISSING_VALUE;

  ldp[0] = (void *)&TimeOffset;

  for (i = 0; i < nsdi; ++i)
    if (sdi[i]->Output)
      ldp[ind++] = (void *)d;

  for (i = 0; i < nraw; ++i)
    {
    if (raw[i]->Output == FALSE)
      continue;

    if (strcmp("HOUR", raw[i]->name) == 0)
      {
      ldp[ind++] = &hour;
      hour = dp->hour;
      }
    else
    if (strcmp("MINUTE", raw[i]->name) == 0)
      {
      ldp[ind++] = &minute;
      minute = dp->minute;
      }
    else
    if (strcmp("SECOND", raw[i]->name) == 0)
      {
      ldp[ind++] = &second;
      second = dp->second;
      }
    else
      ldp[ind++] = (void *)d;
    }

  for (i = 0; i < nderive; ++i)
    if (derived[i]->Output)
      ldp[ind++] = (void *)d;

  for (i = 0; i < dp->nRecords; ++i)
    {
    if (++second > 59.0)
      {
      second = 0.0;
      if (++minute > 59.0)
        {
        minute = 0.0;
        if (++hour > 23.0)
          hour = 0.0;
        }
      }

    ncrecput(fd, recordNumber, ldp);

    TimeOffset += 1.0;
    ++recordNumber;
    }

  DeQueue(missingRecords);
  free(d);

}	/* WRITEMISSINGRECORDS */

/* -------------------------------------------------------------------- */
void SyncNetCDF()
{
  FormatTimeSegmentsForOutputFile(buffer);
  ncattput(fd, NC_GLOBAL, "TimeInterval", NC_CHAR, strlen(buffer), (void *)buffer);

  ncsync(fd);

}	/* END SYNCNETCDF */

/* -------------------------------------------------------------------- */
void CloseNetCDF()
{
  int		len;

  if (fd == ERR)
    return;

  FormatTimeSegmentsForOutputFile(buffer);

  if ((len = strlen(buffer) + 1) > DEFAULT_TI_LENGTH)
    ncredef(fd);

  ncattput(fd, NC_GLOBAL, "TimeInterval", NC_CHAR, len, (void *)buffer);
  ncclose(fd);
  fd = ERR;

  LogMessage("Time interval(s) completed : ");
  LogMessage(buffer);
  LogMessage("\n");

}	/* END CLOSENETCDF */

/* END NETCDF.C */
