/*
-------------------------------------------------------------------------
OBJECT NAME:	Xplot.c

FULL NAME:	Plot Graph in X window

ENTRY POINTS:	PlotData()

STATIC FNS:	set_clipping()
		do_ytics()
		PlotLines()

DESCRIPTION:	This is the Expose event procedure to regenerate the
		whole image.

INPUT:		none

OUTPUT:		none

AUTHOR:		websterc@ncar
-------------------------------------------------------------------------
*/

#include "define.h"
#include "header.h"
#include <X11/Xutil.h>
#include <Xm/DrawingA.h>

/* Y offset from Graph Origin   */
#define YLEGEND(s)      (windowHeight-10-(s*12))

static int	HD, VD, LV, RV, TH, BH, TICLEN, TITLE_OFFSET, SUBTITLE_OFFSET,
		XLABEL_OFFSET, YLABEL_OFFSET, Y_TIC_LABEL_OFFSET,
		X_TIC_LABEL_OFFSET, XLEGEND_START, XLEGEND_END, XLEGEND_TEXT;

static short	windowWidth, windowHeight;

static GC		gc;
static XGCValues	values;

static Display	*dpy;
static Window	win;

static Pixmap		in_pm, out_pm;
static XImage		*im_out;
static XFontStruct	*fontInfo[4];

static void	do_ytics(), set_clipping();

void	PlotLines(), SetYlabelPixmap();
float	GetData();


/* -------------------------------------------------------------------- */
void ResizePlotWindow(Widget w, XtPointer client, XtPointer call)
{
  int     n, depth;
  Arg     args[5];
 
  n = 0;
  XtSetArg(args[n], XmNwidth, &windowWidth); ++n;
  XtSetArg(args[n], XmNheight, &windowHeight); ++n;
  XtSetArg(args[n], XmNdepth, &depth); ++n;
  XtGetValues(canvas, args, n);

  XFreePixmap(dpy, win);
  win = XCreatePixmap(dpy, XtWindow(canvas), windowWidth, windowHeight,depth);

 
  /* NOTE: In an X window the origin is the upper left corner with X
   *  going accross, and Y going down.  I have also noticed that
   *  there are 83 pixels/inch across and 82 pixels/inch down.
   */
  HD = windowWidth * 0.78;
  VD = windowHeight * 0.676;
  LV = windowWidth * 0.15;
  TH = windowHeight * 0.15;
 
  RV = LV + HD;
  BH = TH + VD;
 
  TICLEN = HD > 400 ? 10 : 5;
  TITLE_OFFSET    = TH * 0.5;
  SUBTITLE_OFFSET = TH * 0.75;
  XLABEL_OFFSET   = BH + 50;
  YLABEL_OFFSET   = LV - 75;
  Y_TIC_LABEL_OFFSET = 5;
  X_TIC_LABEL_OFFSET = 20;
 
  XLEGEND_START   = 12;
  XLEGEND_END     = 50;
  XLEGEND_TEXT    = 55;
 
}	/* END RESETWINDOWSIZE */

/* -------------------------------------------------------------------- */
void PlotData(w, client, call)
Widget				w;
XtPointer			client;
XmDrawingAreaCallbackStruct	*call;
{
  Arg	args[9];
  int	len, offset;

  static bool	firstTime = TRUE;


  /* If there are more Expose Events on the queue, then ignore this
   * one.
   */
  if (call && call->event && ((XExposeEvent *)call->event)->count > 0)
    return;


  /* Set default Graphics Context stuff and get the GC
   */
  if (firstTime)
    {
    int			n, width = 300, height, depth;
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

    if ((fontInfo[0] = XLoadQueryFont(dpy, iv.titleFont)) == NULL ||
	(fontInfo[1] = XLoadQueryFont(dpy, iv.subtitleFont)) == NULL ||
	(fontInfo[2] = XLoadQueryFont(dpy, iv.ticFont)) == NULL)
      HandleError("Xplot.c: can't load font", FALSE, EXIT);

    height = fontInfo[1]->max_bounds.ascent + fontInfo[1]->max_bounds.descent;

    in_pm  = XCreatePixmap(dpy, win, width, height, depth);
    out_pm = XCreatePixmap(dpy, win, height, width, depth);

    win = XCreatePixmap(dpy, win, 10, 10, depth);
    ResizePlotWindow(NULL, NULL, NULL);
    ToggleUnits(NULL, (XtPointer)VOLTS, NULL);

    firstTime = FALSE;
    }


  /* Set clipping to whole window */
  XSetClipMask(dpy, gc, None);

  XSetForeground(dpy, gc, values.background);
  XFillRectangle(dpy, win, gc, 0, 0, windowWidth, windowHeight);
  XSetForeground(dpy, gc, values.foreground);

  XDrawRectangle(dpy, win, gc, LV, TH, HD, VD);

  /* Display title, then do subtitle and ylabel.
   */
  if ((len = strlen(title)))
    {
    XSetFont(dpy, gc, fontInfo[0]->fid);
    offset = LV + (HD>>1) - (XTextWidth(fontInfo[0], title, len)>>1);
    XDrawString(dpy, win, gc, offset, TITLE_OFFSET, title, len);
    }

  XSetFont(dpy, gc, fontInfo[1]->fid);

  if ((len = strlen(subtitle)))
    {
    offset = LV + (HD>>1) - (XTextWidth(fontInfo[1], subtitle, len)>>1);
    XDrawString(dpy, win, gc, offset, SUBTITLE_OFFSET, subtitle, len);
    }

  if ((len = strlen(ylabel)))
    {
    int		ascent, descent, charSize, pixLen;

    ascent	= fontInfo[1]->max_bounds.ascent;
    descent	= fontInfo[1]->max_bounds.descent;
    pixLen	= XTextWidth(fontInfo[1], ylabel, len);
    charSize	= ascent + descent;

    /* Write out the rotated text
     */
    offset = TH + (VD >> 1) - (pixLen >> 1);

    XPutImage(dpy, win, gc, im_out, 0, 0, YLABEL_OFFSET - ascent,
			offset, charSize, pixLen);
    }

  if (strlen(xlabel))
    {
    int	offset, len = strlen(xlabel);

    offset = LV + (HD>>1) - (XTextWidth(fontInfo[1], xlabel, len)>>1);
    XDrawString(dpy, win, gc, offset, XLABEL_OFFSET, xlabel, len);
    }


  XSetFont(dpy, gc, fontInfo[2]->fid);
  do_ytics(dpy, win, gc, fontInfo[2]);

  PlotLines();

  XCopyArea(dpy, win, XtWindow(canvas), gc, 0, 0, windowWidth, windowHeight, 0, 0);

}	/* END PLOTDATA */

/* -------------------------------------------------------------------- */
void PlotLines()
{
  int		i, n, cnt;
  NR_TYPE	xscale, yscale, halfSecond;
  char		dash_list[4];
  long		foreGround;

  if (nVariables == 0)
    return;


  /* Draw X-axis tic marks and #'s
   */
{
  register int	offset;
  int		ticlen, nsex, nh, nm, ns;

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

/*    if ((ns += UserStartTime[2]) > 59) { ns -= 60; nm += 1; }
    if ((nm += UserStartTime[1]) > 59) { nm -= 60; nh += 1; }
    if ((nh += UserStartTime[0]) > 23) { nh -= 24; }
*/
    sprintf(buffer, "%02d:%02d:%02d", nh, nm, ns);

    offset -= (XTextWidth(fontInfo[2], buffer, strlen(buffer))>>1);
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
    VARTBL	*vp = &Variable[CurrentDataSet];
    int		ylegend = YLEGEND(CurrentDataSet);

    sprintf(buffer, "%s, %d s/sec", vp->name, vp->SampleRate);

    XDrawLine(dpy, win, gc, XLEGEND_START, ylegend,XLEGEND_END,ylegend);
    XDrawString(dpy, win, gc, XLEGEND_TEXT, ylegend+3, buffer, strlen(buffer));
    }

    /* Change Dash style for the next dataset	*/
    dash_list[0] = (CurrentDataSet+1) * 6;
    dash_list[1] = (CurrentDataSet+1) * 3;
    XSetLineAttributes(dpy, gc, 1, LineOnOffDash, CapButt, JoinMiter);
    XSetDashes(dpy, gc, 0, dash_list, 2);
    }

  XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
  XDrawRectangle(dpy, win, gc, LV, TH, HD, VD);

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

/* -------------------------------------------------------------------- */
SetYLabelPixmap()
{
	register	i, j;
	int			len, ascent, descent, charSize, pixLen;
	XImage		*im_in;

	len = strlen(ylabel);
	ascent	= fontInfo[1]->max_bounds.ascent;
	descent	= fontInfo[1]->max_bounds.descent;
	charSize = ascent + descent;
	pixLen = XTextWidth(fontInfo[1], ylabel, len);

	XSetForeground(dpy, gc, values.background);
	XFillRectangle(dpy, in_pm, gc, 0, 0, pixLen, charSize);
	XSetForeground(dpy, gc, values.foreground);
	XSetFont(dpy, gc, fontInfo[1]->fid);

	if (im_out)
		XDestroyImage(im_out);

	/* Fix this weird bug where one character doesn't show up.
	 */
	if (len == 1)
		{
		len = 3;
		ylabel[1] = ylabel[0]; ylabel[0] = ylabel[2] = ' '; ylabel[3] = '\0';
		pixLen = XTextWidth(fontInfo[1], ylabel, len);
		}

	XDrawString(dpy, in_pm, gc, 0, ascent, ylabel, len);

	im_in = XGetImage(dpy, in_pm, 0, 0, pixLen, charSize, -1, XYPixmap);
	im_out = XGetImage(dpy, out_pm, 0, 0, charSize, pixLen, -1, XYPixmap);

	/* Rotate pixmap
	 */
	for (i = 0; i < charSize; i++)
	for (j = 0; j < pixLen; j++)
			XPutPixel(im_out, i, pixLen-j-1, XGetPixel(im_in, j, i));

	XDestroyImage(im_in);

}	/* END SETYLABELPIXMAP */

/* END XPLOT.C */
