#
#    Makefile for ncplot
#
CC	= gcc
BIN	= ${LOCAL}/bin
RELEASE = ${LOCAL}/nimbus/src/rtplot
NOBJ	= ${NIMBUS}/src/filter/obj
MAN1	= ${LOCAL}/man/man1
LIBS	= -lraf -lhdr_api -lXm -lXt -lX11 -lsocket -lnsl -lxlate -lm
LINTLIBS= -lm
INCLUDES= -I${LOCAL}/include -I${NIMBUS}/include -I${WINDS}/include
CFLAGS	= -O2 ${INCLUDES}

SRCS=	rtplot.c global.c init.c autoscale.c dataIO.c Xwin.c hdr_decode.c\
	ccb.c Xplot.c error.c misc.c memalloc.c amlibfn.c rtloop.c rec_decode.c

OBJS=	rtplot.o global.o init.o autoscale.o dataIO.o Xwin.o hdr_decode.o\
	ccb.o Xplot.o error.o misc.o memalloc.o amlibfn.o rtloop.o rec_decode.o

OOBJ=	${NOBJ}/calco.o ${NOBJ}/sort.o ${NOBJ}/textfile.o ${NOBJ}/popen.o ${NOBJ}/search.o

HDRS=	define.h extern.h
PROG=	rtplot

RESOURCE= XmNCrt

.c.o:
	${CC} ${CFLAGS} -c $*.c

${PROG}: ${OBJS}
	${CC} ${CFLAGS} -L../../lib ${OBJS} ${OOBJ} ${LIBS} -o $@

release:
	tar cFFf - . | (cd ${RELEASE}; tar xfBp -)
	sed s/-O2/-O2/g Makefile > ${RELEASE}/Makefile
	rm -f ${RELEASE}/LOCK.make

install: ${PROG}
	cp ${PROG} ${BIN}
	cp ${PROG}.1 ${MAN1}
	cp ${RESOURCE} ${XAPPLRESDIR}

clean:
	rm -f core ${OBJS} ${PROG}

lint:
	lint ${INCLUDES} ${SRCS} ${LINTLIBS}

print:
	enscript -2Gr -b${PROG} ${HDRS} ${SRCS}

${OBJS}:	${HDRS}
amlibfn.o:	${NIMBUS}/include/xlatefns.h
