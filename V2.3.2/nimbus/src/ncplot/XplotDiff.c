/*
-------------------------------------------------------------------------
OBJECT NAME:	XplotDiff.c

FULL NAME:		Plot Graph in X window

ENTRY POINTS:	PlotDifference()

STATIC FNS:		set_clipping()
				do_ytics()
				plotLines()

DESCRIPTION:	This is the Expose event procedure to regenerate the
				whole image.

INPUT:			none

OUTPUT:			none

AUTHOR:			websterc@ncar
-------------------------------------------------------------------------
*/

#include "define.h"
#include <math.h>
#include <X11/Xutil.h>
#include <Xm/DrawingA.h>

/* NOTE: In an X window the origin is the upper left corner with X
 *	going accross, and Y going down.  I have also noticed that
 *	there are 83 pixels/inch across and 82 pixels/inch down.
 */
#define HD      ((int)(83*8))	/* Dis between Left and Right Border	*/
#define VD      ((int)(83*2.5))	/* Dis between Top and Bottom Border	*/

#define LV		((int)(83*1.0))	/* 83 pixels times 2 inches		*/
#define RV		(LV+HD)
#define TH		((int)(83*0.33))
#define BH		(TH+VD)

#define TICLEN				10
#define TITLE_OFFSET		50
#define SUBTITLE_OFFSET		75
#define XLABEL_OFFSET		(BH+50)
#define YLABEL_OFFSET		(LV-50)
#define Y_TIC_LABEL_OFFSET	5
#define X_TIC_LABEL_OFFSET	20

/* Legend Values	*/
#define YLEGEND(s)      (290-(s*12))   /* Y offset from Graph Origin   */
#define XLEGEND_START   (12)
#define XLEGEND_END     (50)
#define XLEGEND_TEXT    (55)

static GC		gc;
static XGCValues	values;

static Display	*dpy;
static Window	win;

static Pixmap	in_pm, out_pm;
static XImage	*im_in, *im_out;

static int		numYtics;	/* Number of decades		*/

static void	plotLines(), do_ytics(), set_clipping();

extern Widget	diffCanvas;
extern float	*diffData[], diffYmin, diffYmax;
extern STAT		DiffStats;


/* -------------------------------------------------------------------- */
/* ARGSUSED */
void PlotDifference(Widget w,
					XtPointer client,
					XmDrawingAreaCallbackStruct *call)
{
	static bool		first_time = TRUE;

	Arg				args[9];
	int				len, offset, depth;

	XFontStruct		*font_info;


	/* If there are more Expose Events on the queue, then ignore this
	 * one.
	 */
	if (DiffWinOpen == FALSE ||
		(call && ((XExposeEvent *)call->event)->count > 0))
		return;

	numYtics = 6;	/* Should be even	*/


	/* Set default Graphics Context stuff and get the GC
	 */
	if (first_time)
		{
		int				n;
		unsigned long	valuemask;

		dpy = XtDisplay(diffCanvas);
		win = XtWindow(diffCanvas);

		n = 0;
		XtSetArg(args[n], XtNforeground, &values.foreground); ++n;
		XtSetArg(args[n], XtNbackground, &values.background); ++n;
		XtSetArg(args[n], XtNdepth, &depth); ++n;
		XtGetValues(diffCanvas, args, n);

		values.line_width = 1;
		valuemask = GCLineWidth | GCForeground | GCBackground;

		gc = XCreateGC(dpy, RootWindowOfScreen(XtScreen(diffCanvas)),
							valuemask, &values);
		}


	/* Set clipping to whole window */
	XSetClipMask(dpy, gc, None);

	XClearWindow(dpy, win);
	XDrawRectangle(dpy, win, gc, LV, TH, HD, VD);
	XFlush(dpy);


	if ((font_info = XLoadQueryFont(dpy, iv.subtitleFont)) == NULL)
		{
		HandleError("Xplot: can't load font", Interactive, RETURN);
		return;
		}
	else
		{
		int		i, j;
		int		ascent, descent, limit, char_size, pix_len;

		XSetFont(dpy, gc, font_info->fid);

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

		len		= strlen(diffYlabel);
		pix_len	= XTextWidth(font_info, diffYlabel, len);
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
		XDrawString(dpy, in_pm, gc, 0, ascent, diffYlabel, len);

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

	if (data_file)
		plotLines(dpy, win, gc, font_info);

	XUnloadFont(dpy, font_info->fid);

}	/* END PLOT_DATA */

/* -------------------------------------------------------------------- */
static void plotLines(dpy, win, gc, font_info)
Display		*dpy;
Window		win;
GC			gc;
XFontStruct	*font_info;
{
	int		i, cnt;
	int		offset, ticlen;
	XPoint	*pts;
	NR_TYPE	xscale, yscale, halfSecond;


	XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);


	/* Draw X-axis tic marks and #'s
	 */
	ticlen = Grid ? TH : BH-TICLEN;

	for (i = 0; i <= numtics; ++i)
		{
		if ((offset = LV + ((float)HD / numtics) * i) > RV)
			offset = RV;

		XDrawLine(dpy, win, gc, offset, BH, offset, ticlen);

		if (!Grid)
			XDrawLine(dpy, win, gc, offset, TH, offset, TH+TICLEN);

		if (i % 2)
			continue;

		MakeTimeTicLabel(buffer, i);

		offset -= (XTextWidth(font_info, buffer, strlen(buffer))>>1);
		XDrawString(dpy, win, gc, offset, BH+X_TIC_LABEL_OFFSET,
											buffer, strlen(buffer));
		}


	/* Plot data
	 */
	CurrentDataSet = 0;
	pts = (XPoint *)GetMemory(NumberElements[0] * sizeof(XPoint));

	xscale = (NR_TYPE)HD / NumberElements[CurrentDataSet];
	yscale = (NR_TYPE)VD / (diffYmax - diffYmin);

	if (NumberElements[CurrentDataSet] == NumberSeconds)
		halfSecond = HD / NumberSeconds / 2;
	else
		halfSecond = 0.0;

	/* Calculate points for lines		*/
	for (i = 0; i < NumberElements[CurrentDataSet]; ++i)
		{
		while ((int)data[CurrentDataSet][i] == MISSING_VALUE)
			++i;

		for (cnt = 0; (int)data[CurrentDataSet][i] != MISSING_VALUE &&
				cnt < XMaxRequestSize(dpy) &&
				i < NumberElements[CurrentDataSet]; ++cnt, ++i)
			{
			pts[cnt].x = LV + (xscale * i) + halfSecond;
			pts[cnt].y = BH - (yscale * (diffData[0][i] - diffYmin));
			}

		/* Display lines			*/
		set_clipping(dpy, gc);

		XDrawLines(dpy, win, gc, pts, cnt, CoordModeOrigin);
		}

	XSetClipMask(dpy, gc, None);

	free(pts);

	sprintf(buffer, "%10.2f%10.2f%10.2f%10.2f%10.2f",
			DiffStats.mean, DiffStats.sigma, DiffStats.variance,
			DiffStats.min, DiffStats.max);

	XDrawString(dpy, win, gc, XLEGEND_TEXT+175, YLEGEND(0)+3,
			buffer, strlen(buffer));

	strcpy(buffer, "      mean     sigma       var       min       max");
	XDrawString(dpy, win, gc, XLEGEND_TEXT+175, YLEGEND(2)+12,
			buffer, strlen(buffer));

	XFlush(dpy);

}	/* END PLOTLINES */

/* -------------------------------------------------------------------- */
static void do_ytics(dpy, win, gc, font_info)
Display		*dpy;
Window		win;
GC		gc;
XFontStruct	*font_info;
{
	int		i, xoffset, yoffset, len = strlen(xlabel);
	int		ticlen;
	double	y_diff, value;

	/* Draw Y-axis tic marks and #'s
	 */
	ticlen = Grid ? RV : LV+TICLEN;
	y_diff = diffYmax - diffYmin;

	for (i = 0; i <= numYtics; ++i)
		{
		yoffset = BH - ((float)VD / numYtics) * i;
		if (yoffset < TH)
			yoffset = TH;

		XDrawLine(dpy, win, gc, LV, yoffset, ticlen, yoffset);

		if (!Grid)
			XDrawLine(dpy, win, gc, RV-TICLEN, yoffset, RV, yoffset);

		value = diffYmin + (y_diff / numYtics * i);

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

/* END XPLOTDIFF.C */
