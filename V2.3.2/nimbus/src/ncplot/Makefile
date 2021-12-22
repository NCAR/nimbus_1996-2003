#
#    Makefile for ncplot
#
CC		= gcc
BIN		= /home/local/bin
RELEASE = /home/local/nimbus/src/ncplot
MAN1	= /home/local/man/man1
LIBS	= -lraf -lXm -lXt -lX11 -lnetcdf -lnsl -lm
LINTLIBS= -lm
INCLUDES= -I${LOCAL}/include
CFLAGS	= -O2 ${INCLUDES}

SRCS=	ncplot.c global.c init.c autoscale.c ps.c dataIO.c parmsIO.c Xwin.c \
		ccb.c Xplot.c error.c ed_plot.c ticlabel.c sort.c search.c spec.c \
		detrend.c window.c spctrm.c regret.c XplotSpec.c ed_spec.c ascii.c \
		diff.c XplotDiff.c fft.c page.c

OBJS=	ncplot.o global.o init.o autoscale.o ps.o dataIO.o parmsIO.o Xwin.o \
		ccb.o Xplot.o error.o ed_plot.o ticlabel.o sort.o search.o spec.o \
		detrend.o window.o spctrm.o regret.o XplotSpec.o ed_spec.o ascii.o \
		diff.o XplotDiff.o fft.o page.o

HDRS=	define.h extern.h
PROG=	ncplot

RESOURCE= XmNCplot

.c.o:
	${CC} ${CFLAGS} -c $*.c

${PROG}: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} ${LIBS} -o $@

release:
	tar cFFf - . | (cd ${RELEASE}; tar xfBp -)
	sed s/-O2/-O2/g Makefile > ${RELEASE}/Makefile
	rm -f ${RELEASE}/LOCK.make

install: ${PROG}
	cp ${PROG} $(BIN)
	cp ${RESOURCE} ${XAPPLRESDIR}
	cp ${PROG}.man ${MAN1}/${PROG}.1

clean:
	rm -f core ${OBJS} ${PROG}

lint:
	lint ${INCLUDES} ${SRCS} ${LINTLIBS}

print:
	enscript -2Gr -b${PROG} ${HDRS} ${SRCS}

${OBJS}:	${HDRS}
dataIO.o:   ${LOCAL}/include/netcdf.h
