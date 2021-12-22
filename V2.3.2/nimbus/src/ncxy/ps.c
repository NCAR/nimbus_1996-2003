/*
-------------------------------------------------------------------------
OBJECT NAME:	ps.c

FULL NAME:		Produce PostScript File

ENTRY POINTS:	PrintPostScript()

STATIC FNS:		do_line_graph()
				do_bar_graph()
				do_ytics()

DESCRIPTION:	This does the PostScript printing.

INPUT:			none

OUTPUT:			none

AUTHOR:			websterc@ncar.ucar.edu
-------------------------------------------------------------------------
*/

#include "define.h"
#include <sys/types.h>
#include <time.h>
#include <pwd.h>

/* Number of pixels from 0,0 to each Border edge.  NOTE in PostScript
 * (0,0) is in the lower left corner of the paper, held at portrait.
 * However, the following constants are LABELED as if holding in
 * Portrait, (0,0) in lower right corner.
 */
#define LV	500		/* Distance from x,0 to Left Verticle Border	*/
#define RV	2100	/* Distance from x,0 to Right Vert Border		*/
#define BH	800		/* Distance from 0,y to Bottom Horiz Border		*/
#define TH	2400	/* Distance from 0,y to Top Horiz Border		*/

#define HD	(RV-LV)	/* Distance between Left and Right Border		*/
#define VD	(TH-BH)	/* Distance between Top and Bottom Border		*/

#define TICLEN				  15
#define TITLE_OFFSET		2800
#define SUBTITLE_OFFSET		2700
#define XLABEL_OFFSET		 650
#define Y_TIC_LABEL_OFFSET	 -15
#define X_TIC_LABEL_OFFSET	 -40


/* Legend Values	*/
#define YLEGEND(s)		(-720+((s+1)*40)) /* Y offset from Graph Origin	*/
#define XLEGEND_TEXT	(-300)


#define	CENTER(x, y, s)	fprintf(fp, \
		"%d (%ld) stringwidth pop 2 div sub %d moveto (%ld) show\n", \
		(x), (s), (y), (s))


static char	*show	= "(%s) s\n";
static char *rightShow  = "(%s) rs\n";
static char	*lineto = "%d %d l\n";
static char	*moveto = "%d %d m\n";
static char	*rlineto= "%d %d r\n";

static void	do_line_graph(), do_ytics();

/* -------------------------------------------------------------------- */
/* ARGSUSED */
void PrintPostScript(w, clientData, callData)
Widget		w;
XtPointer	clientData;
XtPointer	callData;
{
	FILE	*fp;
	time_t	print_time = time((time_t *)NULL);
	bool	lpr = TRUE;

	if (callData)
		CancelWarning((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

	if (vpX == NULL || vpY == NULL)
		{
		ShowError("No.");
		return;
		}

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
		if ((fp = popen("lp -o nobanner", "w")) == NULL)
			{
			HandleError("ps: can't open pipe to 'lpr'", Interactive,
									IRET);
			return;
			}



	/* Print standard header info, and quadruple number of pixels
	 */
	fprintf(fp, "%%!PS-Adobe\n");
	fprintf(fp, "%%%%Creator: ncxy\n");
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
		fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
					(HD>>1)+LV, title, TITLE_OFFSET);
		fprintf(fp, show, title);
		}

	fprintf(fp, "/Times-Roman findfont 60 scalefont setfont\n");

	if (strlen(subtitle))
		{
		fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
					(HD>>1)+LV, subtitle, SUBTITLE_OFFSET);
		fprintf(fp, show, subtitle);
		}

	if (strlen(ylabel))
		{
		fprintf(fp, "%d %d (%s) stringwidth pop 2 div sub moveto\n",
					LV-180, (VD>>1)+BH, ylabel);
		fprintf(fp, "90 rotate\n");
		fprintf(fp, show, ylabel);
		fprintf(fp, "-90 rotate\n");
		}

	if (strlen(xlabel))
		{
		fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
					(HD>>1)+LV, xlabel, XLABEL_OFFSET);
		fprintf(fp, show, xlabel);
		}

	/* Draw the bounding box for graph, then move the origin
	 * and do rotation of axis.
	 */
	fprintf(fp, "3 setlinewidth\n");
	fprintf(fp, "stroke\n");
	fprintf(fp, moveto, LV, BH);
	fprintf(fp, lineto, LV, TH);
	fprintf(fp, lineto, RV, TH);
	fprintf(fp, lineto, RV, BH);
	fprintf(fp, lineto, LV, BH);
	fprintf(fp, "stroke\n");
	fprintf(fp, "%d %d translate\n", LV, BH);

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
static void do_line_graph(fp)
FILE	*fp;		/* Output file	*/
{
	int		i, j, in;
	int		x, y, x2, y2, ticlen, nsex, nh, nm, ns;
	NR_TYPE	xscale, yscale, ratio, ui_sum, vi_sum;
	double	x_diff, value;


	/* Draw X-axis tic marks and values, and label
	 */
	fprintf(fp, "1 setlinewidth\n");
	fprintf(fp, "/Times-Roman findfont 40 scalefont setfont\n");


	ticlen = Grid ? VD : TICLEN;
	x_diff = xmax - xmin;

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

		value = xmin + (x_diff / numtics * i);
		MakeTicLabel(buffer, x_diff, value);

		fprintf(fp, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
						x, buffer, X_TIC_LABEL_OFFSET);
		fprintf(fp, show, buffer);
		}

	/* Set the scale factor to number of pixels divided by the
	 * number of divisions
	 */
	xscale = (NR_TYPE)HD / (xmax - xmin);
	yscale = (NR_TYPE)VD / (ymax - ymin);


	/* Print legend.
	 */
	for (i = 0; i < 2; ++i)
		{
		VARTBL			*vp;
		register int	ylegend = YLEGEND(i);

		if (i == 0) vp = vpX; else vp = vpY;

		sprintf(buffer, "%s, %d s/sec", vp->name, vp->OutputRate);

		fprintf(fp, moveto, XLEGEND_TEXT, ylegend-10);
		fprintf(fp, show, buffer);


        fprintf(fp, moveto, XLEGEND_TEXT+500, ylegend-10);
        sprintf(buffer, "%10.2f", VarStats[i].mean);
        fprintf(fp, rightShow, buffer);
 
        fprintf(fp, moveto, XLEGEND_TEXT+700, ylegend-10);
        sprintf(buffer, "%10.2f", VarStats[i].sigma);
        fprintf(fp, rightShow, buffer);
 
        fprintf(fp, moveto, XLEGEND_TEXT+900, ylegend-10);
        sprintf(buffer, "%10.2f", VarStats[i].variance);
        fprintf(fp, rightShow, buffer);
 
        fprintf(fp, moveto, XLEGEND_TEXT+1100, ylegend-10);
        sprintf(buffer, "%10.2f", VarStats[i].min);
        fprintf(fp, rightShow, buffer);
 
        fprintf(fp, moveto, XLEGEND_TEXT+1300, ylegend-10);
        sprintf(buffer, "%10.2f", VarStats[i].max);
        fprintf(fp, rightShow, buffer);

		if (WindBarbs && vpUI && vpVI)
			{
			fprintf(fp, moveto, XLEGEND_TEXT, YLEGEND(5));
			strcpy(buffer, "Wind Barbs");
			fprintf(fp, show, buffer);

			fprintf(fp, moveto, XLEGEND_TEXT, YLEGEND(4));
			x2 = XLEGEND_TEXT+(xscale * (10.0 / 30.0));
			fprintf(fp, lineto, x2, YLEGEND(4));

			strcpy(buffer, "10 m/s");
			fprintf(fp, moveto, x2+50, YLEGEND(4)-10);
			fprintf(fp, show, buffer);
			}

		fprintf(fp, "stroke [%d] 0 setdash\n", (i+1)<<3);
		}

	fprintf(fp, moveto, XLEGEND_TEXT+500, YLEGEND(3)-30);
	fprintf(fp, rightShow, "mean");

	fprintf(fp, moveto, XLEGEND_TEXT+700, YLEGEND(3)-30);
	fprintf(fp, rightShow, "sigma");

	fprintf(fp, moveto, XLEGEND_TEXT+900, YLEGEND(3)-30);
	fprintf(fp, rightShow, "var");

	fprintf(fp, moveto, XLEGEND_TEXT+1100, YLEGEND(3)-30);
	fprintf(fp, rightShow, "min");

	fprintf(fp, moveto, XLEGEND_TEXT+1300, YLEGEND(3)-30);
	fprintf(fp, rightShow, "max");

	fprintf(fp, "clipregion clip\n");
	fprintf(fp, "newpath\n");
	fprintf(fp, "[] 0 setdash\n");



	x = xscale * (data[X][0] - xmin);;
	y = yscale * (data[Y][0] - ymin);

	fprintf(fp, moveto, x, y);

	if (NumberElements[X] == NumberElements[Y])
		{
		for (i = 0; i < NumberElements[X]; ++i)
			{
			x = xscale * (data[X][i] - xmin);
			y = yscale * (data[Y][i] - ymin);

			if (ScatterPlot)
				{
				fprintf(fp, moveto, x, y);
				fprintf(fp, lineto, x, y+1);
				fprintf(fp, lineto, x+1, y+1);
				fprintf(fp, lineto, x+1, y);
				}
			else
				{
				fprintf(fp, lineto, x, y);

				if (WindBarbs && vpUI && vpVI && (i % 600) - 300 == 0)
					{
					ui_sum = vi_sum = 0.0;

					for (j = i - 300; j < i + 300; ++j)
						{
						ui_sum += uic[i];
						vi_sum += vic[i];
						}

					ui_sum /= 600;
					vi_sum /= 600;

					x2 = x + (xscale * (ui_sum / 30.0));
					y2 = y + (xscale * (vi_sum / 30.0));

					fprintf(fp, lineto, x2, y2);
					fprintf(fp, moveto, x, y);
					}
				}

			if (!(i % 128))
				{
				fprintf(fp, "stroke\n");
				fprintf(fp, moveto, x, y);
				}
			}
		}
	else
	if (NumberElements[X] > NumberElements[Y])
		{
		ratio = (float)NumberElements[Y] / NumberElements[X];

		for (i = 0; i < NumberElements[X]; ++i)
			{
			in = ratio * i;

			x = xscale * (data[X][i] - xmin);
			y = yscale * (data[Y][in] - ymin);

			fprintf(fp, lineto, x, y);

			if (!(i % 128))
				{
				fprintf(fp, "stroke\n");
				fprintf(fp, moveto, x, y);
				}
			}
		}
	else
	if (NumberElements[X] < NumberElements[Y])
		{
		ratio = (float)NumberElements[X] / NumberElements[Y];

		for (i = 0; i < NumberElements[Y]; ++i)
			{
			in = ratio * i;

			x = xscale * (data[X][in] - xmin);
			y = yscale * (data[Y][i] - ymin);

			fprintf(fp, lineto, x, y);

			if (!(i % 128))
				{
				fprintf(fp, "stroke\n");
				fprintf(fp, moveto, x, y);
				}
			}
		}

	fprintf(fp, "stroke [%d] 0 setdash\n", (Y+1)<<3);

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
