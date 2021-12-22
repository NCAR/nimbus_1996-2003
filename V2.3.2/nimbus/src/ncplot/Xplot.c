/*
-------------------------------------------------------------------------
OBJECT NAME:	Xplot.c

FULL NAME:		Plot Graph in X window

ENTRY POINTS:	PlotData()
				Zoom()

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
#include <X11/Xutil.h>
#include <Xm/DrawingA.h>

/* NOTE: In an X window the origin is the upper left corner with X
 *	going accross, and Y going down.  I have also noticed that
 *	there are 83 pixels/inch across and 82 pixels/inch down.
 */
#define HD		((int)(83*8))		/* Dis between Left and Right Border */
#define VD		((int)(83*5.5))	/* Dis between Top and Bottom Border */

#define LV		((int)(83*1.5))		/* 83 pixels times 2 inches		*/
#define RV		(LV+HD)
#define TH		((int)(83*1.25))
#define BH		(TH+VD)

#define TICLEN			10
#define TITLE_OFFSET	50
#define SUBTITLE_OFFSET	75
#define XLABEL_OFFSET	(BH+40)
#define YLABEL_OFFSET	(LV-75)
#define Y_TIC_LABEL_OFFSET 5
#define X_TIC_LABEL_OFFSET 20

/* Legend Values	*/
#define YLEGEND(s)      (665-(s*12))   /* Y offset from Graph Origin   */
#define XLEGEND_START   (12)
#define XLEGEND_END     (50)
#define XLEGEND_TEXT    (55)


static GC		gc;
static XGCValues	values;

static Display	*dpy;
static Window	win;

static Pixmap	in_pm, out_pm;
static XImage	*im_in, *im_out;

static void	plotLines(), do_ytics(), set_clipping();


/* -------------------------------------------------------------------- */
void PlotData(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *call)
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

	if (DataChanged)
		{
		XClearWindow(dpy, win);

		if (AsciiWinOpen) {
			SetASCIIdata(NULL, NULL, NULL);
			}

		if (DiffWinOpen) {
			ComputeDiff(NULL, NULL, NULL);
			PlotDifference(NULL, NULL, NULL);
			}

		DataChanged = FALSE;
		}
	else
		if (YaxisRescaled)
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
			int		i, j;
			int		ascent, descent, limit, char_size,
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
	int			i, cnt;
	int			offset, ticlen;
	NR_TYPE		xscale, yscale, halfSecond;
	XPoint		*pts;
	char		dash_list[4];

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
	for (CurrentDataSet = 0; CurrentDataSet < NumberDataSets; ++CurrentDataSet)
		{
		xscale = (NR_TYPE)HD / NumberElements[CurrentDataSet];
		yscale = (NR_TYPE)VD / (ymax - ymin);

		pts =
		(XPoint *)GetMemory(NumberElements[CurrentDataSet] * sizeof(XPoint));

		/* Center 1hz data.
		 */
		if (NumberElements[CurrentDataSet] == NumberSeconds)
			halfSecond = HD / NumberSeconds / 2;
		else
			halfSecond = 0.0;

		/* Calculate points for lines		*/
		for (i = 0; i < NumberElements[CurrentDataSet]; ++i)
			{
			while ((int)data[CurrentDataSet][i] == MISSING_VALUE &&
				i < NumberElements[CurrentDataSet])
				++i;

			for (cnt = 0; (int)data[CurrentDataSet][i] != MISSING_VALUE &&
						cnt < XMaxRequestSize(dpy) &&
						i < NumberElements[CurrentDataSet]; ++cnt, ++i)
				{
				pts[cnt].x = LV + (xscale * i) + halfSecond;
				pts[cnt].y = BH - (yscale * (data[CurrentDataSet][i] - ymin));
				}

			/* Display lines			*/
			set_clipping(dpy, gc);
			XDrawLines(dpy, win, gc, pts, cnt, CoordModeOrigin);
			}

		XSetClipMask(dpy, gc, None);

		free(pts);


		/* Display legend for each dataset
		 */
		{
		VARTBL	*vp = Variable[SelectedVarIndex[CurrentDataSet]];
		int		ylegend = YLEGEND(CurrentDataSet);

		sprintf(buffer, "%s, %d s/sec", vp->name, vp->OutputRate);

		XDrawLine(dpy, win, gc, XLEGEND_START, ylegend,XLEGEND_END,ylegend);
		XDrawString(dpy, win, gc, XLEGEND_TEXT, ylegend+3,
						buffer, strlen(buffer));

		sprintf(buffer, "%10.2f%10.2f%10.2f%10.2f%10.2f   %s",
						VarStats[CurrentDataSet].mean,
						VarStats[CurrentDataSet].sigma,
						VarStats[CurrentDataSet].variance,
						VarStats[CurrentDataSet].min,
						VarStats[CurrentDataSet].max,
						VarStats[CurrentDataSet].units);

		XDrawString(dpy, win, gc, XLEGEND_TEXT+150, ylegend+3,
						buffer, strlen(buffer));
		}

		/* Change Dash style for the next dataset	*/
		dash_list[0] = (CurrentDataSet+1)*6;
		dash_list[1] = (CurrentDataSet+1)*3;
		XSetLineAttributes(dpy, gc, 1, LineOnOffDash, CapButt, JoinMiter);
		XSetDashes(dpy, gc, 0, dash_list, 2);
		}

	if (NumberDataSets > 0)
		{
		strcpy(buffer, "      mean     sigma       var       min       max   units");
		XDrawString(dpy, win, gc, XLEGEND_TEXT+150,
					YLEGEND((NumberDataSets+1))+12, buffer, strlen(buffer));
		}

	XFlush(dpy);

	XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);

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

/* -------------------------------------------------------------------- */
/* Following 2 functions are for the drag zoom.	*/

static int		startX = 0, startY = 0;
static int		endX = 0, endY = 0;

static void DoTheBox(Widget w, XtPointer client, XMotionEvent *evt, Boolean cont2disp)
{
	if (evt == NULL)
		return;

	if (endX != 0 && endY != 0)
		XDrawRectangle(dpy, win, gc,	startX, startY,
										endX - startX, endY - startY);

	endX = evt->x;
	endY = evt->y;

	XDrawRectangle(dpy, win, gc, startX, startY, endX - startX, endY - startY);

}

/* -------------------------------------------------------------------- */
void Zoom(Widget w, XtPointer client, XmDrawingAreaCallbackStruct *evt)
{
	XButtonEvent	*xb = (XButtonEvent *)evt->event;
	static bool		cancel = FALSE;

	if (xb->button != Button1)
		return;

	if (xb->state == 0)
		startX = startY = endX = endY = 0;

	if (xb->state == 0x100)
		{
		XSetFunction(dpy, gc, GXcopy);
		XtRemoveEventHandler(canvas, Button1MotionMask, False, DoTheBox, NULL);
		}

	if (xb->x < LV || xb->x > RV || xb->y < TH || xb->y > BH)
		{
		if (xb->state == 0)
			cancel = TRUE;

		return;
		}

	if (xb->state != 0)
		if (cancel || endX < startX || endY < startY)
			return;


	if (xb->state == 0)
		{
		cancel = FALSE;

		startX = xb->x;
		startY = xb->y;

		XSetFunction(dpy, gc, GXxor);
		XtAddEventHandler(canvas, Button1MotionMask, False, DoTheBox, NULL);
		}
	else if (xb->state == 0x100)
		{
		int		stpe, etpe;
		float	pixPerSec;

		if (abs(startX - xb->x) < 10 || abs(startY - xb->y) < 10)
			return;

		endX = xb->x;
		endY = xb->y;

		pixPerSec = (float)HD / NumberSeconds;
		stpe = (startX - LV) / pixPerSec;
		etpe = (RV - endX) / pixPerSec;

		if (NumberSeconds > 900)	/* 15 minutes.	*/
			{
			stpe /= 60.0; stpe *= 60.0;
			etpe += 60.0; etpe /= 60.0; etpe *= 60.0;
			}

		ReduceData(stpe, NumberSeconds - stpe - etpe);

		biggest_y = ymax - ((ymax-ymin) * ((float)(startY - TH) / VD));
		smallest_y = ymin + ((ymax-ymin) * ((float)(BH - endY) / VD));

		DoAutoScale(NULL, NULL, NULL);
		}
	else
		printf("Zoom: new state = 0x%x\n", xb->state);

}

/* END XPLOT.C */
