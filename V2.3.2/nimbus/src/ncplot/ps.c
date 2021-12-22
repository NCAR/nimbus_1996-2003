/*
-------------------------------------------------------------------------
OBJECT NAME:	ps.c

FULL NAME:		Produce PostScript File

ENTRY POINTS:	PrintPostScript()

STATIC FNS:		do_line_graph()
				do_ytics()

DESCRIPTION:	This does the PostScript printing.

INPUT:		none

OUTPUT:		none

AUTHOR:		websterc@ncar
-------------------------------------------------------------------------
*/

#include "define.h"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>

/* Number of pixels from 0,0 to each Border edge.  NOTE in PostScript
 * (0,0) is in the lower left corner of the paper, held at portrait.
 * However, the following constants are LABELED as if holding in
 * Landscape, (0,0) in lower right corner.
 */
#define LV	2600	/* Distance from x,0 to Left Verticle Border	*/
#define RV	300		/* Distance from x,0 to Right Vert Border	*/
#define BH	400		/* Distance from 0,y to Bottom Horiz Border	*/
#define TH	2000	/* Distance from 0,y to Top Horiz Border	*/

#define HD	(LV-RV)	/* Distance between Left and Right Border	*/
#define VD	(TH-BH)	/* Distance between Top and Bottom Border	*/

#define TICLEN				  15
#define TITLE_OFFSET		2250
#define SUBTITLE_OFFSET		2150
#define XLABEL_OFFSET		 300
#define Y_TIC_LABEL_OFFSET	 -15
#define X_TIC_LABEL_OFFSET	 -40


/* Legend Values	*/
#define YLEGEND(s)		(-320+((s+1)*40)) /* Y offset from Graph Origin	*/
#define XLEGEND_START	(-480)
#define XLEGEND_END		(-330)
#define XLEGEND_TEXT	(-300)
#define BOXSIZE			(35)


#define	CENTER(x, y, s)	fprintf(fp, \
		"%d (%ld) stringwidth pop 2 div sub %d moveto (%ld) show\n", \
		(x), (s), (y), (s))


static char	*show	= "(%s) s\n";
static char	*rightShow	= "(%s) rs\n";
static char	*lineto = "%d %d l\n";
static char	*moveto = "%d %d m\n";

static void	do_line_graph(FILE *fp), do_ytics();

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void PrintPostScript(Widget w, XtPointer client, XtPointer call)
{
	FILE	*fp;
	time_t	print_time = time((time_t *)NULL);
	bool	lpr = TRUE;

	if (call)
		CancelWarning((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

	if (out_file)
		{
		if ((fp = fopen(out_file, "w")) == NULL)
			{
			sprintf(buffer, "Can't open output file %s", out_file);
			HandleError(buffer, Interactive, IRET);
			return;
			}

		lpr = FALSE;
		}
	else
		if ((fp = popen(lpCommand, "w")) == NULL)
			{
			HandleError("ps: can't open pipe to 'lp'", Interactive,
									IRET);
			return;
			}



	/* Print standard header info, and quadruple number of pixels
	 */
	fprintf(fp, "%%!PS-Adobe\n");
	fprintf(fp, "%%%%Creator: ncplot\n");
	fprintf(fp, "%%%%For: %s\n", (getpwuid(getuid()))->pw_name);
	fprintf(fp, "%%%%Title: %s\n", title);
	fprintf(fp, "%%%%CreationDate: %s", ctime(&print_time));
	fprintf(fp, "%%%%Pages: 1\n");
	fprintf(fp, "%%%%DocumentFonts: Times-Roman\n");
	fprintf(fp, "%%%%BoundingBox: 0 0 612 792\n");
	fprintf(fp, "%%%%EndComments\n");
	fprintf(fp, "%%%%Page: 1 1\n");
	fprintf(fp, "/m {moveto} bind def\n");
	fprintf(fp, "/l {lineto} bind def\n");
	fprintf(fp, "/r {rlineto} bind def\n");
	fprintf(fp, "/s {show} bind def\n");
	fprintf(fp, "/rs {dup stringwidth pop 120 exch sub 0 rmoveto show");
	fprintf(fp, "} bind def\n");
	fprintf(fp, "/clipregion { 0 0 m 0 %d l\n", VD);
	fprintf(fp, "%d %d l %d 0 l closepath } def\n", HD, VD, HD);
	fprintf(fp, "save\n");
	fprintf(fp, ".25 .25 scale\n");

	/* Print Titles & Labels
	 */
	if (strlen(title))
		{
		fprintf(fp, "/Times-Roman findfont 80 scalefont setfont\n");
		fprintf(fp, "%d %d (%s) stringwidth pop 2 div add moveto\n",
					TITLE_OFFSET, (HD>>1)+RV, title);
		fprintf(fp, "-90 rotate\n");
		fprintf(fp, show, title);
		fprintf(fp, "90 rotate\n");
		}

	fprintf(fp, "/Times-Roman findfont 60 scalefont setfont\n");

	if (strlen(subtitle))
		{
		fprintf(fp, "%d %d (%s) stringwidth pop 2 div add moveto\n",
					SUBTITLE_OFFSET, (HD>>1)+RV, subtitle);
		fprintf(fp, "-90 rotate\n");
		fprintf(fp, show, subtitle);
		fprintf(fp, "90 rotate\n");
		}

	if (strlen(ylabel))
		{
		fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
						(VD>>1)+BH, ylabel, LV+180);
		fprintf(fp, show, ylabel);
		}

	if (strlen(xlabel))
		{
		fprintf(fp, "%d %d (%s) stringwidth pop 2 div add moveto\n",
					XLABEL_OFFSET, (HD>>1)+RV, xlabel);
		fprintf(fp, "-90 rotate\n");
		fprintf(fp, show, xlabel);
		fprintf(fp, "90 rotate\n");
		}

	/* Draw the bounding box for graph, then move the origin
	 * and do rotation of axis.
	 */
	fprintf(fp, "3 setlinewidth\n");
	fprintf(fp, "stroke\n");
	fprintf(fp, moveto, BH, LV);
	fprintf(fp, lineto, TH, LV);
	fprintf(fp, lineto, TH, RV);
	fprintf(fp, lineto, BH, RV);
	fprintf(fp, lineto, BH, LV);
	fprintf(fp, "stroke\n");
	fprintf(fp, "%d %d translate\n", BH, LV);
	fprintf(fp, "270 rotate\n");

	do_ytics(fp);
	fprintf(fp, "1 setlinewidth\n");
	fprintf(fp, "stroke 0 0 moveto\n");

	do_line_graph(fp);

	fprintf(fp, "stroke\n");
	fprintf(fp, "showpage\n");
	fprintf(fp, "restore\n");

	if (lpr)
		pclose(fp);
	else
		fclose(fp);

}	/* END PRINTPOSTSCRIPT */

/* -------------------------------------------------------------------- */
static void do_line_graph(FILE *fp)
{
	char	*p;
	int		i;
	int		x, y, ticlen, nsex, nh, nm, ns;
	NR_TYPE	xscale, yscale, halfSecond;


	/* Draw X-axis tic marks and values, and label
	 */
	fprintf(fp, "1 setlinewidth\n");
	fprintf(fp, "/Times-Roman findfont 40 scalefont setfont\n");


	ticlen = Grid ? VD : TICLEN;

	for (i = 0; i <= numtics; ++i)
		{
		x = HD / numtics * i;
		fprintf(fp, moveto, x, 0);
		fprintf(fp, lineto, x, ticlen);

		if (!Grid)
			{
			fprintf(fp, moveto, x, VD-TICLEN);
			fprintf(fp, lineto, x, VD);
			}

		if (i % 2)
			continue;

		nsex = i * NumberSeconds / numtics;

		nh = nsex / 3600;
		nm = (nsex - (nh * 3600)) / 60;
		ns = nsex - (nh * 3600) - (nm * 60);

		if ((ns += UserStartTime[2]) > 59) { ns -= 60; nm += 1; }
		if ((nm += UserStartTime[1]) > 59) { nm -= 60; nh += 1; }
		if ((nh += UserStartTime[0]) > 23) { nh -= 24; }

		sprintf(buffer, "%02d:%02d:%02d", nh, nm, ns);

		fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
						x, buffer, X_TIC_LABEL_OFFSET);
		fprintf(fp, show, buffer);
		}

	/* Print legend.
	 */
	for (CurrentDataSet = 0; CurrentDataSet < NumberDataSets; ++CurrentDataSet)
		{
		VARTBL			*vp = Variable[SelectedVarIndex[CurrentDataSet]];
		register int	ylegend = YLEGEND(CurrentDataSet);

		sprintf(buffer, "%s, %d s/sec", vp->name, vp->OutputRate);

		fprintf(fp, moveto, XLEGEND_START, ylegend);
		fprintf(fp, lineto, XLEGEND_END, ylegend);
		fprintf(fp, moveto, XLEGEND_TEXT, ylegend-10);
		fprintf(fp, show, buffer);

		fprintf(fp, moveto, XLEGEND_TEXT+500, ylegend-10);
		sprintf(buffer, "%10.2f", VarStats[CurrentDataSet].mean);
		fprintf(fp, rightShow, buffer);

		fprintf(fp, moveto, XLEGEND_TEXT+700, ylegend-10);
		sprintf(buffer, "%10.2f", VarStats[CurrentDataSet].sigma);
		fprintf(fp, rightShow, buffer);

		fprintf(fp, moveto, XLEGEND_TEXT+900, ylegend-10);
		sprintf(buffer, "%10.2f", VarStats[CurrentDataSet].variance);
		fprintf(fp, rightShow, buffer);

		fprintf(fp, moveto, XLEGEND_TEXT+1100, ylegend-10);
		sprintf(buffer, "%10.2f", VarStats[CurrentDataSet].min);
		fprintf(fp, rightShow, buffer);

		fprintf(fp, moveto, XLEGEND_TEXT+1300, ylegend-10);
		sprintf(buffer, "%10.2f", VarStats[CurrentDataSet].max);
		fprintf(fp, rightShow, buffer);

		fprintf(fp, moveto, XLEGEND_TEXT+1500, ylegend-10);
		sprintf(buffer, "%s", VarStats[CurrentDataSet].units);
		fprintf(fp, rightShow, buffer);

		fprintf(fp, "stroke [%d] 0 setdash\n", (CurrentDataSet+1)<<3);
		}

	fprintf(fp, moveto, XLEGEND_TEXT+500, YLEGEND(NumberDataSets+1)-30);
	fprintf(fp, rightShow, "mean");

	fprintf(fp, moveto, XLEGEND_TEXT+700, YLEGEND(NumberDataSets+1)-30);
	fprintf(fp, rightShow, "sigma");

	fprintf(fp, moveto, XLEGEND_TEXT+900, YLEGEND(NumberDataSets+1)-30);
	fprintf(fp, rightShow, "var");

	fprintf(fp, moveto, XLEGEND_TEXT+1100, YLEGEND(NumberDataSets+1)-30);
	fprintf(fp, rightShow, "min");

	fprintf(fp, moveto, XLEGEND_TEXT+1300, YLEGEND(NumberDataSets+1)-30);
	fprintf(fp, rightShow, "max");

/*	strcpy(buffer, "mean      sigma          var          min           max");
	fprintf(fp, moveto, XLEGEND_TEXT+500, YLEGEND(NumberDataSets+1)-30);
	fprintf(fp, show, buffer);
*/
	fprintf(fp, "clipregion clip\n");
	fprintf(fp, "newpath\n");
	fprintf(fp, "[] 0 setdash\n");

	for (CurrentDataSet = 0; CurrentDataSet < NumberDataSets; ++CurrentDataSet)
		{
		/* Set the scale factor to number of pixels divided by the
		 * number of divisions
		 */
		xscale = (NR_TYPE)HD / NumberElements[CurrentDataSet];
		yscale = (NR_TYPE)VD / (ymax - ymin);

		if (NumberElements[CurrentDataSet] == NumberSeconds)
			halfSecond = HD / NumberSeconds / 2;
		else
			halfSecond = 0.0;

		x = halfSecond;
		y = yscale * (data[CurrentDataSet][0] - ymin);

		fprintf(fp, moveto, x, y);

		for (i = 0; i < NumberElements[CurrentDataSet]; ++i)
			{
			if ((int)data[CurrentDataSet][i] == MISSING_VALUE)
				{
				while ((int)data[CurrentDataSet][i] == MISSING_VALUE)
					++i;

				p = moveto;
				}
			else
				p = lineto;

			x = xscale * i + halfSecond;
			y = yscale * (data[CurrentDataSet][i] - ymin);

			fprintf(fp, p, x, y);

			if (!(i % 128))
				{
				fprintf(fp, "stroke\n");
				fprintf(fp, moveto, x, y);
				}
			}

		fprintf(fp, "stroke [%d] 0 setdash\n", (CurrentDataSet+1)<<3);
		}

}	/* END DO_LINE_GRAPH */

/* -------------------------------------------------------------------- */
static void do_ytics(fp)
FILE    *fp;
{
	register	i;
	int		y;
	int		ticlen;
	double		value, y_diff;

	fprintf(fp, "/Times-Roman findfont 40 scalefont setfont\n");
	fprintf(fp, "1 setlinewidth\n");

	/* Draw Y-axis tic marks & corresponding values
	*/
	ticlen = Grid ? HD : TICLEN;
	y_diff = ymax - ymin;

	for (i = 0; i <= numtics; ++i)
		{
		y = i * VD / numtics;

		fprintf(fp, moveto, 0, y);
		fprintf(fp, lineto, ticlen, y);

		if (!Grid)
			{
			fprintf(fp, moveto, HD-TICLEN, y);
			fprintf(fp, lineto, HD, y);
			}

		value = ymin + (y_diff / numtics * i);

		MakeTicLabel(buffer, y_diff, value);

		fprintf(fp, "%d (%s) stringwidth pop sub %d moveto (%s) show\n",
				Y_TIC_LABEL_OFFSET, buffer, y-15, buffer);
		}

}       /* END DO_YTICS */

/* END PS.C */
