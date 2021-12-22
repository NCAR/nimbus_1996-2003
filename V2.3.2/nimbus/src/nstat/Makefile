#
#    Makefile for Skeleton program
#
CC=			gcc
DESTDIR=
BINDIR=		/home/local/bin
RELEASE=	/home/local/nimbus/src/nstat
MANDIR=		/home/local/man/man1
RESOURCE=	XmNstat
LIBS=		-lraf -lm -lnetcdf -lnsl -lXm -lXt -lX11
LINTLIBS=	${LIBS}
INCLUDES=	-I${LOCAL}/include
CFLAGS=		-O2 ${INCLUDES}

SRCS=	nstat.c globals.c init.c timeseg.c sort.c search.c lrloop.c\
		netcdf.c cb_main.c cb_time.c cb_pause.c Xwin.c error.c

HDRS=	define.h

OBJS=	nstat.o globals.o init.o timeseg.o sort.o search.o lrloop.o\
		netcdf.o cb_main.o cb_time.o cb_pause.o Xwin.o error.o

PROG=	nstat

.c.o:
	${CC} ${CFLAGS} -c $*.c

${PROG}: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} ${LIBS} -o $@

install: ${PROG}
	cp ${PROG} ${DESTDIR}${BINDIR}
	cp ${RESOURCE} ${XAPPLRESDIR}
	cp ${PROG}.1 ${DESTDIR}${MANDIR}

release:
	tar cFFf - . | (cd ${RELEASE}; tar xfBp -)
	sed s/-O2/-O2/g Makefile > ${RELEASE}/Makefile
	rm -f ${RELEASE}/LOCK.make

clean:
	rm -f ${OBJS} ${PROG}

lint:
	lint ${INCLUDES} ${SRCS} ${LINTLIBS}

print:
	enscript -2Gr -b${PROG} ${HDRS} ${SRCS}

${OBJS}:	define.h
netcdf.o:	${LOCAL}/include/netcdf.h
