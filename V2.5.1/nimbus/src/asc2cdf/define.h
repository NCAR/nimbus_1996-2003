#define NO_NETCDF_2

#include <netcdf.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BUFFSIZE            8192
#define MAX_VARS            500
#define DEFAULT_TI_LENGTH   (20)
#define MISSING_VALUE       (-32767.0)

extern char buffer[];
extern int  ncid, varid[], nVariables, timeOffsetID;
extern float scaleFactors[], missingVals[];
extern char *time_vars[];
extern time_t BaseTime;

extern int	SkipNlines, unitsLineNum;

void CreateNASANetCDF(FILE *fp), CreatePlainNetCDF(FILE *fp);
void SetNASABaseTime(void), SetPlainBaseTime(void);
