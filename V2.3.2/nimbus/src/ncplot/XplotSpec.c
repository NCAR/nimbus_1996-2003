/*
-------------------------------------------------------------------------
OBJECT NAME:	XplotSpec.c

FULL NAME:		Plot Graph in X window

ENTRY POINTS:	PlotSpectrum()

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
#define HD      ((int)(83*5))	/* Dis between Left and Right Border	*/
#define VD      ((int)(83*5.5))	/* Dis between Top and Bottom Border	*/

#define LV		((int)(83*1.5))	/* 83 pixels times 2 inches		*/
#define RV		(LV+HD)
#define TH		((int)(83*1.25))
#define BH		(TH+VD)

#define TICLEN				10
#define TITLE_OFFSET		50
#define SUBTITLE_OFFSET		75
#define XLABEL_OFFSET		(BH+50)
#define YLABEL_OFFSET		(LV-75)
#define Y_TIC_LABEL_OFFSET	5
#define X_TIC_LABEL_OFFSET	20

/* Legend Values	*/
#define YLEGEND(s)      (588-(s*12))   /* Y offset from Graph Origin   */
#define XLEGEND_START   (12)
#define XLEGEND_END     (50)
#define XLEGEND_TEXT    (55)

static GC		gc;
static XGCValues	values;

static Display	*dpy;
static Window	win;

static Pixmap	in_pm, out_pm;
static XImage	*im_in, *im_out;

static int		numXtics, numYtics;	/* Number of decades		*/
static float	numXpix, numYpix;	/* Number pixels per decade	*/

static void	plotLines(), do_ytics(), set_clipping();

extern Widget	specCanvas;
extern double	*Pxx;
extern int		M, K;
extern int		specYmin, specYmax, specXmin, specXmax;


/* -------------------------------------------------------------------- */
/* ARGSUSED */
void PlotSpectrum(	Widget w,
					XtPointer client,
					XmDrawingAreaCallbackStruct	*call)
{
	static bool		first_time = TRUE;

	Arg				args[9];
	int				len, offset, depth;

	XFontStruct		*font_info;


	/* If there are more Expose Events on the queue, then ignore this
	 * one.
	 */
	if (call && ((XExposeEvent *)call->event)->count > 0)
		return;

	numXtics = abs(specXmax - specXmin);
	numYtics = abs(specYmax - specYmin);
	numXpix = (float)HD / numXtics;
	numYpix = (float)VD / numYtics;


	/* Set default Graphics Context stuff and get the GC
	 */
	if (first_time)
		{
		int				n;
		unsigned long	valuemask;

		dpy = XtDisplay(specCanvas);
		win = XtWindow(specCanvas);

		n = 0;
		XtSetArg(args[n], XtNforeground, &values.foreground); ++n;
		XtSetArg(args[n], XtNbackground, &values.background); ++n;
		XtSetArg(args[n], XtNdepth, &depth); ++n;
		XtGetValues(specCanvas, args, n);

		values.line_width = 1;
		valuemask = GCLineWidth | GCForeground | GCBackground;

		gc = XCreateGC(dpy, RootWindowOfScreen(XtScreen(specCanvas)),
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

		{
		char	*ylab = "P(f)";
		int		i, j;
		int		ascent, descent, limit, char_size, pix_len;


		len		= strlen(ylab);
		pix_len	= XTextWidth(font_info, ylab, len);
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
		XDrawString(dpy, in_pm, gc, 0, ascent, ylab, len);

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

		{
		char	*xlab = "Frequency [Hz]";
		int	offset, len = strlen(xlab);

		offset = LV + (HD>>1) - (XTextWidth(font_info, xlab, len)>>1);
		XDrawString(dpy, win, gc, offset, XLABEL_OFFSET, xlab, len);
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
	int		i, j;
	int		offset, ticlen;
	XPoint	*pts;
	int		yDecade, xDecade, nYdecades, nXdecades;
	double	yMantissa, xMantissa;
	double	x;
	VARTBL	*vp = Variable[SelectedVarIndex[0]];


	XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);


	/* Draw X-axis tic marks and #'s
	 */
	ticlen = Grid ? TH : BH-TICLEN;

	for (i = 0; i <= numXtics; ++i)
		{
		offset = LV + (numXpix * i);

		XDrawLine(dpy, win, gc, offset, BH, offset, ticlen);

		if (!Grid)
			XDrawLine(dpy, win, gc, offset, TH, offset, TH+TICLEN);

        MakeLogTicLabel(buffer, specXmin + i);

		offset -= (XTextWidth(font_info, buffer, strlen(buffer))>>1);
		XDrawString(dpy, win, gc, offset, BH+X_TIC_LABEL_OFFSET,
					buffer, strlen(buffer));

		if (i != numXtics)
			for (j = 1; j < 10; ++j)
				{
				offset = (LV + (numXpix * i)) + log10((double)j) * numXpix;

				XDrawLine(dpy, win, gc, offset, BH, offset, ticlen+5);
				XDrawLine(dpy, win, gc, offset, TH, offset, TH+TICLEN-5);
			}
		}


	/* Plot data
	 */
	pts = (XPoint *)GetMemory((M+1) * sizeof(XPoint));

	/* Calculate points for lines		*/
	for (i = 1; i <= M; ++i)
		{
		x = ((double)vp->OutputRate / (M << 1)) * i;
		xDecade = (int)log10(x);
		xMantissa = log10(x) - xDecade;

		for (j = 0; j < numXtics; ++j)
			if (specXmin + j == xDecade)
				nXdecades = j;

		yDecade = (int)log10(Pxx[i]);
		yMantissa = log10(Pxx[i]) - yDecade;

		for (j = 0; j <= numYtics; ++j)
			if (specYmin + j == yDecade)
				nYdecades = j;

		pts[i-1].x = LV + (nXdecades * numXpix) + (xMantissa * numXpix);
		pts[i-1].y = BH - (nYdecades * numYpix) - (yMantissa * numYpix);
		}

	/* Display lines			*/
	set_clipping(dpy, gc);

	XDrawLines(dpy, win, gc, pts, M, CoordModeOrigin);
	XSetClipMask(dpy, gc, None);

	free(pts);

	XFlush(dpy);


}	/* END PLOTLINES */

/* -------------------------------------------------------------------- */
static void do_ytics(dpy, win, gc, font_info)
Display		*dpy;
Window		win;
GC		gc;
XFontStruct	*font_info;
{
	int		i, j, xoffset, yoffset, len = strlen(xlabel);
	int		ticlen;

	/* Draw Y-axis tic marks and #'s
	 */
	ticlen = Grid ? RV : LV+TICLEN;

	for (i = 0; i <= numYtics; ++i)
		{
		yoffset = BH - (numYpix * i);
		if (yoffset < TH)
			yoffset = TH;

		XDrawLine(dpy, win, gc, LV, yoffset, ticlen, yoffset);

		if (!Grid)
			XDrawLine(dpy, win, gc, RV-TICLEN, yoffset, RV, yoffset);

		MakeLogTicLabel(buffer, specYmin + i);
		len = strlen(buffer);

		xoffset = LV - Y_TIC_LABEL_OFFSET -
					XTextWidth(font_info, buffer, len);

		XDrawString(dpy, win, gc, xoffset, yoffset, buffer, len);

		if (i != numYtics)
			for (j = 1; j < 10; ++j)
				{
				yoffset = BH - (numYpix * i) - (log10((double)j) * numYpix);

				XDrawLine(dpy, win, gc, LV, yoffset, ticlen-5, yoffset);
				XDrawLine(dpy, win, gc, RV-TICLEN+5, yoffset, RV, yoffset);
				}
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
