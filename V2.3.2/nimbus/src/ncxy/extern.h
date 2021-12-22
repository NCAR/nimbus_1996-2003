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

extern bool		Interactive;
extern bool		Grid;
extern bool		ScatterPlot;
extern bool		WindBarbs;

extern char		buffer[], *parms_file, *data_file, *out_file, DataPath[];
extern char		*ProjectNumber,*ProjectName,*FlightNumber,UserPath[],
				*FlightDate, *timeSeg;

extern int		nVariables;
extern VARTBL	*Variable[], *vpX, *vpY, *vpUI, *vpVI;
extern STAT		VarStats[];


extern int		NumberSeconds, NumberElements[];
extern NR_TYPE	*data[], *uic, *vic;

extern int		FileStartTime[], FileEndTime[];
extern int		UserStartTime[], UserEndTime[];

/* Parameter File Variables	*/
extern char		title[], subtitle[], xlabel[], ylabel[], legend[];
extern char		*kw_title, *kw_subtitle, *kw_xlabel, *kw_ylabel, *kw_xmin,
				*kw_xmax, *kw_ymin, *kw_ymax, *kw_legend;

extern NR_TYPE	xmin, xmax, ymin, ymax;
extern int		numtics;

/* Error messages	*/
extern char		*o_mem;

/* X vars	*/
extern Widget	canvas, varListX, varListY;

extern instanceRec	iv;


/* Procedures	*/
char	*get_legend(), *GetMemory();

void	CreateErrorBox(), ShowError(), ErrorOK();
void	CreateWarningBox(), WarnUser(), CancelWarning();
void	CreateQueryBox(), QueryUser(), QueryCancel(), ExtractAnswer();
void	CreateFileSelectionBox(), QueryFile(), ExtractFileName(),
		FileCancel(), FindMinMax();

void	EditParms(), DismissParmsWindow(), ApplyParms(), NewDataFile(),
		AddVariable(), DeleteVariable(), ToggleGrid(), DoAutoScale(), Quit(),
		GetDataFileName(), ReadParmsFileName(), WriteParmsFileName(), PrintPS(),
		DoAverage(), DoRegression(), ModifyActiveVars(), ToggleScatter(),
		ToggleWindBarb();
	
void	write_parms_CB2(), print_PS_CB2();

void	ReadDataFile(), ReadParmsFile(), WriteParmsFile(), PlotData(),
		PrintPostScript(), Regression(), RunningAverage(), AutoScale();

#endif

/* END EXTERN.H */
