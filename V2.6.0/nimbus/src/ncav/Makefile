#
#    Makefile for Skeleton program
#
CC=			gcc
DESTDIR=
BINDIR=		/home/local/bin
ANONFTP=	/home/users/ftp/pub/src/bin.sol2
RELEASE=	/home/local/nimbus/src/ncav
MANDIR=		/home/local/man/man1
RESOURCE=   XmNCav
LIBS=		-lraf -lm -lnetcdf -lXm -lXt -lX11
LINTLIBS=	${LIBS}
INCLUDES=	-I${LOCAL}/include
CFLAGS=		-Wall -O2 ${INCLUDES}

SRCS=	ncav.c globals.c init.c timeseg.c sort.c search.c \
		netcdf.c cb_main.c cb_time.c cb_pause.c Xwin.c error.c

HDRS=	define.h

OBJS=	ncav.o globals.o init.o timeseg.o sort.o search.o \
		netcdf.o cb_main.o cb_time.o cb_pause.o Xwin.o error.o

PROG=	ncav

.c.o:
	${CC} ${CFLAGS} -c $*.c

${PROG}: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} ${LIBS} -o $@

install: ${PROG}
	cp ${PROG} ${DESTDIR}${BINDIR}
	cp ${PROG}.man ${DESTDIR}${MANDIR}/${PROG}.1
	cp ${RESOURCE} ${XAPPLRESDIR}
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
