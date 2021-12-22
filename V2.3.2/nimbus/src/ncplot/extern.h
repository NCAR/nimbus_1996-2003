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

extern bool		Interactive, Grid, AutoScaling, DataChanged, DiffWinOpen,
				SpecWinOpen, AsciiWinOpen, RecomputeSpectra, YaxisRescaled;

extern char		buffer[], *parms_file, *data_file, *out_file, DataPath[],
				*timeSeg, *ProjectNumber, *ProjectName, *FlightNumber,
				UserPath[], *FlightDate;

extern int		nVariables;
extern VARTBL	*Variable[];
extern STAT		VarStats[];


extern int		NumberDataSets, NumberSeconds, NumberElements[], CurrentDataSet;
extern int		SelectedVarIndex[];
extern NR_TYPE	*data[];

extern int		FileStartTime[], FileEndTime[];
extern int		UserStartTime[], UserEndTime[];

/* Parameter File Variables	*/
extern char		title[], subtitle[], xlabel[], ylabel[], legend[], diffYlabel[],
				asciiFormat[], lpCommand[];

extern int		nASCIIpoints;
extern char		*kw_title, *kw_subtitle, *kw_xlabel, *kw_ylabel, *kw_xmin,
				*kw_xmax, *kw_ymin, *kw_ymax, *kw_legend;

extern NR_TYPE	ymin, ymax, smallest_y, biggest_y;
extern int		numtics;

/* X vars	*/
extern Widget	canvas, varList;

extern instanceRec	iv;


/* Procedures	*/
char	*get_legend();
void	*GetMemory();

void	Initialize(), processArgs();

void	CreateErrorBox(), ShowError(), ErrorOK();
void	CreateWarningBox(), WarnUser(), CancelWarning();
void	CreateQueryBox(), QueryUser(), QueryCancel(), ExtractAnswer();
void	CreateFileSelectionBox(), QueryFile(), ExtractFileName(),
		FileCancel();

void	NewDataFile(Widget w, XtPointer client, XtPointer call),
		ReadData(Widget w, XtPointer client, XtPointer call),
		AddVariable(int indx), DeleteVariable(int indx),
		ReduceData(int start, int newNumberSeconds);

void	Quit(Widget w, XtPointer client, XtPointer call),
		DismissWindow(Widget w, XtPointer client, XtPointer call),
		ModifyActiveVars(Widget w, XtPointer client, XtPointer call),
		ToggleGrid(Widget w, XtPointer client, XtPointer call),
		DoAutoScale(Widget w, XtPointer client, XtPointer call),
		ToggleAutoScaling(Widget w, XtPointer client, XtPointer call),
		PrintPS(Widget w, XtPointer client, XtPointer call),
		ReadParmsFileName(Widget w, XtPointer client, XtPointer call),
		WriteParmsFileName(Widget w, XtPointer client, XtPointer call),
		GetDataFileName(Widget w, XtPointer client, XtPointer call);

void	EditPlotParms(Widget w, XtPointer client, XtPointer call),
		SetYdialog(), SetYlabel(char *s), CreateParmsWindow(Widget parent),
		SetSubtitle();

void	PageForward(Widget w, XtPointer client, XtPointer call),
		PageBackward(Widget w, XtPointer client, XtPointer call);

void	ComputeDiff(Widget w, XtPointer client, XtPointer call),
		DiffWinControl();

void	Zoom(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *evt),
		PlotData(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *call),
		PlotDifference(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *call),
		PlotSpectrum(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *call);


void	ViewASCII(Widget w, XtPointer client, XtPointer call),
		SetASCIIdata(Widget w, XtPointer client, XtPointer call);

void	SpectrumControl(Widget w, XtPointer client, XtPointer call),
		SetSegLen(Widget w, XtPointer client, XtPointer call),
		SetWindow(Widget w, XtPointer client, XtPointer call),
		SetDetrend(Widget w, XtPointer client, XtPointer call),
		EditSpecParms(Widget w, XtPointer client, XtPointer call);

void	PrintPostScript(Widget w, XtPointer client, XtPointer call);

void	AutoScale(char axis), AutoScaleDiff();
void	HandleError(char s[], bool interactiv, char status);
void	SortTable(char **table, int beg, int end);

void	MakeTicLabel(char buffer[], double diff, double value),
		MakeLogTicLabel(char buffer[], int value),
		MakeTimeTicLabel(char buffer[], int indx);

void	ReadParmsFile(Widget w, XtPointer client, XtPointer call),
		WriteParmsFile(Widget w, XtPointer client, XtPointer call);

int		SearchTable(char *table[], int ntable, char target[]);

#endif

/* END EXTERN.H */
