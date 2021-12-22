/*
-------------------------------------------------------------------------
OBJECT NAME:	Xplot.c

FULL NAME:		Plot Graph in X window

ENTRY POINTS:	PlotData()

STATIC FNS:		set_clipping()
				do_ytics()
				PlotLines()

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
#define HD      ((int)(83*8))	/* Dis between Left and Right Border	*/
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
#define XLEGEND_START   (12)
#define XLEGEND_END     (50)
#define XLEGEND_TEXT    (55)


static GC		gc;
static XGCValues	values;

static Display	*dpy;
static Window	win;

static Pixmap	in_pm, out_pm;
static XImage	*im_in, *im_out;
static XFontStruct		*font_info;

void	PlotLines(), do_ytics(), set_clipping();
float	GetData();


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
			HandleError("Can't load font", RETURN);
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
		HandleError("Xplot: can't load font", RETURN);
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
		HandleError("Xplot: can't load font", RETURN);
		return;
		}
	else
		{
		XSetFont(dpy, gc, font_info->fid);
		do_ytics(dpy, win, gc, font_info);
		}

	XFlush(dpy);

	PlotLines();

	XUnloadFont(dpy, font_info->fid);

}	/* END PLOT_DATA */

/* -------------------------------------------------------------------- */
void PlotLines()
{
	int			i, n, cnt;
	NR_TYPE		xscale, yscale, halfSecond;
	char		dash_list[4];
	long		foreGround;

	if (nVariables == 0)
		return;


	/* Draw X-axis tic marks and #'s
	 */
{
	register int	offset;
	int				ticlen, nsex, nh, nm, ns;

	ticlen = Grid ? TH : BH-TICLEN;

	for (i = 0; i <= numtics; ++i)
		{
		if ((offset = LV + ((float)HD / numtics) * i) > RV)
			offset = RV;

		XDrawLine(dpy, win, gc, offset, BH, offset, ticlen);

		if (i % 2)
			continue;

		nsex = i * NumberSeconds / numtics;

		nh = nsex / 3600;
		nm = (nsex - (nh * 3600)) / 60;
		ns = nsex - (nh * 3600) - (nm * 60);

/*		if ((ns += UserStartTime[2]) > 59) { ns -= 60; nm += 1; }
		if ((nm += UserStartTime[1]) > 59) { nm -= 60; nh += 1; }
		if ((nh += UserStartTime[0]) > 23) { nh -= 24; }
*/
		sprintf(buffer, "%02d:%02d:%02d", nh, nm, ns);

		offset -= (XTextWidth(font_info, buffer, strlen(buffer))>>1);
		XDrawString(dpy, win, gc, offset, BH+X_TIC_LABEL_OFFSET,
					buffer, strlen(buffer));
		}
}

	/* Plot data
	 */
	foreGround = gc->values.foreground;

	for (CurrentDataSet = 0; CurrentDataSet < NumberDataSets; ++CurrentDataSet)
		{
		XSetForeground(dpy, gc, gc->values.background);
		XDrawLines(dpy, win, gc, Variable[CurrentDataSet].pts, Variable[CurrentDataSet].nPoints, CoordModeOrigin);
		XSetForeground(dpy, gc, foreGround);

		xscale = (NR_TYPE)HD / Variable[CurrentDataSet].nPoints;
		yscale = (NR_TYPE)VD / (ymax - ymin);

		/* Center 1hz data.
		 */
		if (Variable[CurrentDataSet].nPoints == NumberSeconds)
			halfSecond = HD / NumberSeconds / 2;
		else
			halfSecond = 0.0;

		/* Calculate points for lines		*/
		for (i = 0; i < Variable[CurrentDataSet].nPoints; ++i)
			{
			Variable[CurrentDataSet].pts[i].x = LV + (xscale * i) + halfSecond;
			Variable[CurrentDataSet].pts[i].y = BH - (yscale * (GetData(CurrentDataSet, i) - ymin));
			}

		/* Display lines			*/
		set_clipping(dpy, gc);

		n = Variable[CurrentDataSet].nPoints / XMaxRequestSize(dpy);

		if (Variable[CurrentDataSet].nPoints % XMaxRequestSize(dpy) != 0)
			++n;

		cnt =  XMaxRequestSize(dpy);

		for (i = 0; i < n; ++i)
			{
			if (i == n-1)
				cnt = Variable[CurrentDataSet].nPoints % XMaxRequestSize(dpy);

			XDrawLines(dpy, win, gc, &Variable[CurrentDataSet].pts[i*XMaxRequestSize(dpy)], cnt, CoordModeOrigin);
			}

		XSetClipMask(dpy, gc, None);


		/* Display legend for each dataset
		 */
		{
		VARTBL			*vp = &Variable[CurrentDataSet];
		register int	ylegend = YLEGEND(CurrentDataSet);

		sprintf(buffer, "%s, %d s/sec", vp->name, vp->SampleRate);

		XDrawLine(dpy, win, gc, XLEGEND_START, ylegend,XLEGEND_END,ylegend);
		XDrawString(dpy, win, gc, XLEGEND_TEXT, ylegend+3,
						buffer, strlen(buffer));
		}

		/* Change Dash style for the next dataset	*/
		dash_list[0] = (CurrentDataSet+1)*6;
		dash_list[1] = (CurrentDataSet+1)*3;
		XSetLineAttributes(dpy, gc, 1, LineOnOffDash, CapButt, JoinMiter);
		XSetDashes(dpy, gc, 0, dash_list, 2);
		}

	XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
	XDrawRectangle(dpy, win, gc, LV, TH, HD, VD);
	XFlush(dpy);

}	/* END PLOTLINES */

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
