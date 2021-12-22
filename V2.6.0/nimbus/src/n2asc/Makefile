#
#    Makefile for n2asc program
#
CC=			gcc
DESTDIR=
BINDIR=		/home/local/bin
ANONFTP=	/home/users/ftp/pub/src/bin.sol2
RELEASE=	/home/local/nimbus/src/n2asc
MANDIR=		/home/local/man/man1
RESOURCE=	XmN2Ascii
LIBS=		-lnetcdf -lXm -lXt -lX11
LINTLIBS=	${LIBS}
INCLUDES=	-I${LOCAL}/include
CFLAGS=		-O2 ${INCLUDES}

SRCS=	n2asc.c globals.c init.c timeseg.c sort.c search.c lrloop.c\
		netcdf.c cb_main.c cb_time.c cb_pause.c Xwin.c error.c Xerror.c\
		Xfile.c Xquery.c Xwarn.c getmem.c

HDRS=	define.h

OBJS=	n2asc.o globals.o init.o timeseg.o sort.o search.o lrloop.o\
		netcdf.o cb_main.o cb_time.o cb_pause.o Xwin.o error.o Xerror.o\
		Xfile.o Xquery.o Xwarn.o getmem.o

PROG=	n2asc

.c.o:
	${CC} ${CFLAGS} -c $*.c

${PROG}: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} ${LIBS} -o $@

install: ${PROG}
	cp ${PROG} ${DESTDIR}${BINDIR}
	cp ${RESOURCE} ${XAPPLRESDIR}
	cp ${PROG}.man ${DESTDIR}${MANDIR}/${PROG}.1
	cp ${PROG} $(ANONFTP)
	cp ${RESOURCE} ${ANONFTP}
	cp ${PROG}.man ${ANONFTP}


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
