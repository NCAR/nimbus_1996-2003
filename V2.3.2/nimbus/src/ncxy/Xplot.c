/*
-------------------------------------------------------------------------
OBJECT NAME:	Xplot.c

FULL NAME:		Plot Graph in X window

ENTRY POINTS:	PlotData()

STATIC FNS:		set_clipping()
				do_ytics()
				PlotLines()
				PlotWindBarbs()

DESCRIPTION:	This is the Expose event procedure to regenerate the
				whole image.

INPUT:			none

OUTPUT:			none

AUTHOR:			websterc@ncar
-------------------------------------------------------------------------
*/

#include "define.h"
#include <X11/Xutil.h>
#include <Xm/DrawingA.h>

/* NOTE: In an X window the origin is the upper left corner with X
 *	going accross, and Y going down.  I have also noticed that
 *	there are 83 pixels/inch across and 82 pixels/inch down.
 */
#define HD      ((int)(83*5.5))	/* Dis between Left and Right Border	*/
#define VD      ((int)(83*5.5))	/* Dis between Top and Bottom Border	*/

#define LV		((int)(83*2))	/* 83 pixels times 2 inches		*/
#define RV		(LV+HD)
#define TH		((int)(83*1.5))
#define BH		(TH+VD)

#define TICLEN				10
#define TITLE_OFFSET		50
#define SUBTITLE_OFFSET		75
#define XLABEL_OFFSET		(BH+50)
#define YLABEL_OFFSET		(LV-75)
#define Y_TIC_LABEL_OFFSET	5
#define X_TIC_LABEL_OFFSET	20

/* Legend Values	*/
#define YLEGEND(s)      (688-(s*12))   /* Y offset from Graph Origin   */
#define XLEGEND_TEXT    (55)


static GC		gc;
static XGCValues	values;

static Display	*dpy;
static Window	win;

static Pixmap	in_pm, out_pm;
static XImage	*im_in, *im_out;

static void	PlotLines(), PlotWindBarbs(), do_ytics(), set_clipping();


/* -------------------------------------------------------------------- */
/* ARGSUSED */
void PlotData(w, clientData, callData)
Widget				w;
XtPointer			clientData;
XmDrawingAreaCallbackStruct	*callData;
{
	static bool		first_time = TRUE;

	Arg				args[9];
	int				len, offset, depth;

	XFontStruct		*font_info;


	/* If there are more Expose Events on the queue, then ignore this
	 * one.
	 */
	if (callData && ((XExposeEvent *)callData->event)->count > 0)
		return;


	/* Set default Graphics Context stuff and get the GC
	 */
	if (first_time)
		{
		int				n;
		unsigned long	valuemask;

		dpy = XtDisplay(canvas);
		win = XtWindow(canvas);

		n = 0;
		XtSetArg(args[n], XtNforeground, &values.foreground); ++n;
		XtSetArg(args[n], XtNbackground, &values.background); ++n;
		XtSetArg(args[n], XtNdepth, &depth); ++n;
		XtGetValues(canvas, args, n);

		values.line_width = 1;
		valuemask = GCLineWidth | GCForeground | GCBackground;

		gc = XCreateGC(dpy, RootWindowOfScreen(XtScreen(canvas)),
							valuemask, &values);
		}


	/* Set clipping to whole window */
	XSetClipMask(dpy, gc, None);

	XClearWindow(dpy, win);
	XDrawRectangle(dpy, win, gc, LV, TH, HD, VD);
	XFlush(dpy);


	/* Display title, then do subtitle and ylabel
	 */
	if ((len = strlen(title)))
		if ((font_info = XLoadQueryFont(dpy, iv.titleFont)) == NULL)
			{
			HandleError("Can't load font", Interactive, RETURN);
			return;
			}
		else
			{
			XSetFont(dpy, gc, font_info->fid);
			offset = LV + (HD>>1) - (XTextWidth(font_info, title, len)>>1);
			XDrawString(dpy, win, gc, offset, TITLE_OFFSET, title, len);
			XUnloadFont(dpy, font_info->fid);
			}

	if ((font_info = XLoadQueryFont(dpy, iv.subtitleFont)) == NULL)
		{
		HandleError("Xplot: can't load font", Interactive, RETURN);
		return;
		}
	else
		{
		XSetFont(dpy, gc, font_info->fid);

		if ((len = strlen(subtitle)))
			{
			offset = LV + (HD>>1) - (XTextWidth(font_info, subtitle, len)>>1);
			XDrawString(dpy, win, gc, offset, SUBTITLE_OFFSET, subtitle, len);
			}

		/* For the y axis label, we must rotate the text manually
		 * Setup initial pixmaps for rotation, this is done once
		 * because rotation is incredibly slow.
		 */
		if (first_time)
			{
			int	width = 500, height;

			height = font_info->max_bounds.ascent + 
					 font_info->max_bounds.descent;

			in_pm  = XCreatePixmap(dpy, win, width, height, depth);
			out_pm = XCreatePixmap(dpy, win, height, width, depth);

			im_out = XGetImage(dpy, out_pm, 0,0, height,width, -1,XYPixmap);

			first_time = FALSE;
			}

		if ((len = strlen(ylabel)))
			{
			register	i, j;
			int			ascent, descent, limit, char_size,
						pix_len = XTextWidth(font_info, ylabel, len);

			/* Fix this weird bug where one character doesn't show up.
			 */
			if (len == 1)
				{
				len = 3;
				ylabel[1] = ylabel[0]; ylabel[0] = ylabel[2] = ' '; ylabel[3]=0;
				}

			ascent	= font_info->max_bounds.ascent;
			descent	= font_info->max_bounds.descent;
			char_size = ascent + descent;
			limit	= (pix_len > char_size) ? pix_len : char_size;

			/* Create a pixmap, draw string to it and read
			 * it back out as an Image
			 */
			XSetForeground(dpy, gc, values.background);
			XFillRectangle(dpy, in_pm, gc, 0, 0, limit, char_size);
			XSetForeground(dpy, gc, values.foreground);
			XDrawString(dpy, in_pm, gc, 0, ascent, ylabel, len);

			im_in = XGetImage(dpy, in_pm, 0, 0, limit, char_size, -1, XYPixmap);

			/* Rotate pixmap
			 */
			for (i = 0; i < char_size; i++)
				for (j = 0; j < limit; j++)
					XPutPixel(im_out, i, limit-j-1, XGetPixel(im_in, j, i));

			/* Write out the rotated text
			 */
			offset = TH + (VD >> 1) - (pix_len >> 1);

			XPutImage(dpy, win, gc, im_out, 0, 0, YLABEL_OFFSET - ascent,
											offset, char_size, pix_len);
			}

		if (strlen(xlabel))
			{
			int	offset, len = strlen(xlabel);

			offset = LV + (HD>>1) - (XTextWidth(font_info, xlabel, len)>>1);
			XDrawString(dpy, win, gc, offset, XLABEL_OFFSET, xlabel, len);
			}

		XUnloadFont(dpy, font_info->fid);
		}

	if ((font_info = XLoadQueryFont(dpy, iv.ticFont)) == NULL)
		{
		HandleError("Xplot: can't load font", Interactive, RETURN);
		return;
		}
	else
		{
		XSetFont(dpy, gc, font_info->fid);
		do_ytics(dpy, win, gc, font_info);
		}

	XFlush(dpy);

	if (vpX && vpY)
		{
		PlotLines(dpy, win, gc, font_info);

		if (WindBarbs && vpUI && vpVI)
			PlotWindBarbs(dpy, win, gc);
		}

	XUnloadFont(dpy, font_info->fid);

}	/* END PLOT_DATA */

/* -------------------------------------------------------------------- */
static void PlotLines(dpy, win, gc, font_info)
Display		*dpy;
Window		win;
GC			gc;
XFontStruct	*font_info;
{
	int			i, n, cnt, nPts, reqSize;
	NR_TYPE		xscale, yscale;
	XPoint		*pts;
	char		dash_list[4];

	/* Draw X-axis tic marks and #'s
	 */
{
	register int	offset;
	int				ticlen;
	double			x_diff, value;

	ticlen = Grid ? TH : BH-TICLEN;
	x_diff = xmax - xmin;

	for (i = 0; i <= numtics; ++i)
		{
		if ((offset = LV + ((float)HD / numtics) * i) > RV)
			offset = RV;

		XDrawLine(dpy, win, gc, offset, BH, offset, ticlen);

		if (!Grid)
			XDrawLine(dpy, win, gc, offset, TH, offset, TH+TICLEN);

		value = xmin + (x_diff / numtics * i);
		MakeTicLabel(buffer, x_diff, value);

		offset -= (XTextWidth(font_info, buffer, strlen(buffer))>>1);
		XDrawString(dpy, win, gc, offset, BH+X_TIC_LABEL_OFFSET,
					buffer, strlen(buffer));
		}
}

	/* Plot data
	 */
	xscale = (NR_TYPE)HD / (xmax - xmin);
	yscale = (NR_TYPE)VD / (ymax - ymin);

	nPts = MAX(NumberElements[X], NumberElements[Y]);
	pts = (XPoint *)GetMemory(nPts * sizeof(XPoint));

	/* Calculate points for lines.  3 loops, repeat points if SampleRates are
	 * not identical.
	 */
	if (NumberElements[X] == NumberElements[Y])
		for (i = 0; i < nPts; ++i)
			{
			pts[i].x = LV + (xscale * (data[X][i] - xmin));
			pts[i].y = BH - (yscale * (data[Y][i] - ymin));
			}
	else
	if (NumberElements[X] > NumberElements[Y])
		{
		int		in;
		float	ratio = (float)NumberElements[Y] / NumberElements[X];

		for (i = 0; i < nPts; ++i)
			{
			in = ratio * i;

			pts[i].x = LV + (xscale * (data[X][i] - xmin));
			pts[i].y = BH - (yscale * (data[Y][in] - ymin));
			}
		}
	else
	if (NumberElements[X] < NumberElements[Y])
		{
		int		in;
		float	ratio = (float)NumberElements[X] / NumberElements[Y];

		for (i = 0; i < nPts; ++i)
			{
			in = ratio * i;

			pts[i].x = LV + (xscale * (data[X][in] - xmin));
			pts[i].y = BH - (yscale * (data[Y][i] - ymin));
			}
		}



	/* Display lines			*/
	set_clipping(dpy, gc);

	cnt = reqSize = XMaxRequestSize(dpy);

	n = nPts / reqSize;

	if (nPts % reqSize != 0)
		++n;

	for (i = 0; i < n; ++i)
		{
		if (i == n-1)
			cnt = nPts % reqSize;

		if (ScatterPlot)
			XDrawPoints(dpy, win, gc, &pts[i * reqSize], cnt, CoordModeOrigin);
		else
			XDrawLines(dpy, win, gc, &pts[i * reqSize], cnt, CoordModeOrigin);
		}

	XSetClipMask(dpy, gc, None);

	free(pts);



	/* Display legend for each dataset
	 */
	{
	VARTBL			*vp;
	register int	ylegend, axis;

	for (i = 0; i < 2; ++i)
		{
		if (i == 0)
			{ vp = vpX; axis = X; }
		else
			{ vp = vpY; axis = Y; }

		ylegend = YLEGEND(axis);

		sprintf(buffer, "%s, %d s/sec", vp->name, vp->OutputRate);

		XDrawString(dpy, win, gc, XLEGEND_TEXT, ylegend+3,
						buffer, strlen(buffer));

		sprintf(buffer, "%10.2f%10.2f%10.2f%10.2f%10.2f",
						VarStats[axis].mean,
						VarStats[axis].sigma,
						VarStats[axis].variance,
						VarStats[axis].min,
						VarStats[axis].max);

		XDrawString(dpy, win, gc, XLEGEND_TEXT+150, ylegend+3,
						buffer, strlen(buffer));
		}
	}

	strcpy(buffer, "      mean     sigma       var       min       max");
	XDrawString(dpy, win, gc, XLEGEND_TEXT+150, YLEGEND(3)+12, buffer, strlen(buffer));


	XFlush(dpy);

}	/* END PLOTLINES */

/* -------------------------------------------------------------------- */
static void PlotWindBarbs(dpy, win, gc)
Display		*dpy;
Window		win;
GC			gc;
{
	int		i, j, x1, y1, x2, y2, nPts;
	float	xscale, yscale, ui_sum, vi_sum;

	xscale = (NR_TYPE)HD / (xmax - xmin);
	yscale = (NR_TYPE)VD / (ymax - ymin);

	nPts = MAX(NumberElements[X], NumberElements[Y]);


	for (i = 0; i < nPts; ++i)
		{

		if ((i % 600) - 300)
			continue;

		ui_sum = vi_sum = 0.0;

		for (j = i - 300; j < i + 300; ++j)
			{
			ui_sum += uic[i];
			vi_sum += vic[i];
			}

		ui_sum /= 600;
		vi_sum /= 600;

		x1 = LV + (xscale * (data[X][i] - xmin));
		y1 = BH - (yscale * (data[Y][i] - ymin));

		x2 = x1 + (xscale * (ui_sum / 30.0));
		y2 = y1 - (xscale * (vi_sum / 30.0));

		XDrawLine(dpy, win, gc, x1, y1, x2, y2);
		}

	y1 = y2 = YLEGEND(3);
	x1 = XLEGEND_TEXT - 10;
	x2 = x1 + (xscale * (10.0 / 30.0));
	XDrawLine(dpy, win, gc, x1, y1, x2, y2);

	strcpy(buffer, "10 m/s");
	XDrawString(dpy, win, gc, x2+10, YLEGEND(3)+5, buffer, strlen(buffer));

	strcpy(buffer, "Wind Barbs");
	XDrawString(dpy, win, gc, XLEGEND_TEXT, YLEGEND(4), buffer, strlen(buffer));

}	/* END PLOTWINDBARBS */

/* -------------------------------------------------------------------- */
static void do_ytics(dpy, win, gc, font_info)
Display		*dpy;
Window		win;
GC		gc;
XFontStruct	*font_info;
{
	int	i, xoffset, yoffset, len = strlen(xlabel);
	int	ticlen;
	double	y_diff, value;

	/* Draw Y-axis tic marks and #'s
	 */
	ticlen = Grid ? RV : LV+TICLEN;
	y_diff = ymax - ymin;

	for (i = 0; i <= numtics; ++i)
		{
		yoffset = BH - ((float)VD / numtics) * i;
		if (yoffset < TH)
			yoffset = TH;

		XDrawLine(dpy, win, gc, LV, yoffset, ticlen, yoffset);

		if (!Grid)
			XDrawLine(dpy, win, gc, RV-TICLEN, yoffset, RV, yoffset);

		value = ymin + (y_diff / numtics * i);

		MakeTicLabel(buffer, y_diff, value);
		len = strlen(buffer);

		xoffset = LV - Y_TIC_LABEL_OFFSET -
					XTextWidth(font_info, buffer, len);

		XDrawString(dpy, win, gc, xoffset, yoffset, buffer, len);
		}

}	/* END DO_YTICS */

/* -------------------------------------------------------------------- */
static void set_clipping(dpy, gc)
Display	*dpy;
GC	gc;
{
	XRectangle	clip_area[1];

	/* Set clipping so that graph data cannot exceed box boundries
	 */
	clip_area[0].x = LV;
	clip_area[0].y = TH;
	clip_area[0].width = HD;
	clip_area[0].height = VD;
	XSetClipRectangles(dpy, gc, 0, 0, clip_area, 1, Unsorted);

}	/* END SET_CLIPPING */

/* END XPLOT.C */
