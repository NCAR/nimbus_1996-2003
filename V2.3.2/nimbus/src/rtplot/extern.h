/*
-------------------------------------------------------------------------
OBJECT NAME:	extern.h

FULL NAME:	Global Variable External Declarations

DESCRIPTION:	

AUTHOR:		websterc@ncar
-------------------------------------------------------------------------
*/

#ifndef EXTERN_H
#define EXTERN_H

extern bool		Counts, Grid, Frozen, FlashMode;

extern char		buffer[], *winds_path, DataPath[], *ProjectNumber, *ProjectName,
				*FlightDate;

extern SDITBL  *sdi[];          /* SDI (aka raw) variable list  */
extern RAWTBL  *raw[];          /* Alphabeticly sorted pointers */
extern DEFAULT *Defaults[];

extern int		nsdi, nraw, nVariables;
extern VARTBL	Variable[];


extern int		NumberDataSets, NumberSeconds, NumberElements[], CurrentDataSet;
extern int		Aircraft, FlightNumber;

extern char		*ADSrecord;
extern NR_TYPE	*SampledData;

extern NR_TYPE	*plotData[];


/* Parameter File Variables	*/
extern char		title[], subtitle[], xlabel[], ylabel[], legend[];
extern char		*kw_title, *kw_subtitle, *kw_xlabel, *kw_ylabel, *kw_xmin,
				*kw_xmax, *kw_ymin, *kw_ymax, *kw_legend;

extern NR_TYPE	ymin, ymax, smallest_y, biggest_y;
extern int		numtics;

/* Error messages	*/
extern char		*o_mem;

/* X vars	*/
extern Widget	canvas, varList;

extern instanceRec	iv;


/* Procedures	*/
char	*get_legend();
void	*GetMemory();

void	CreateErrorBox(), ShowError(), ErrorOK();
void	CreateWarningBox(), WarnUser(), CancelWarning();
void	CreateQueryBox(), QueryUser(), QueryCancel(), ExtractAnswer();
void	CreateFileSelectionBox(), QueryFile(), ExtractFileName(),
		FileCancel(), FindMinMax();

void	EditParms(), DismissParmsWindow(), ApplyParms(), AddVariable(),
		DeleteVariable(), ToggleGrid(), DoAutoScale(), Quit(), ToggleFlash(),
		PlotData(), AutoScale(), ToggleCounts(), ModifyActiveVars();
	
#endif

/* END EXTERN.H */
