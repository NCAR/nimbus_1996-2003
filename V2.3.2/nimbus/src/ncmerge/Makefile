#
#    Makefile for netCDF merge program
#
CC=			gcc
DESTDIR=
BINDIR=		/home/local/bin
RELEASE=	/home/local/nimbus/src/ncmerge
MANDIR=		/home/local/man/man1
LIBS=		-lnetcdf -lnsl
LINTLIBS=	${LIBS}
INCLUDES=	-I/home/local/include
CFLAGS=		-O ${INCLUDES}

SRCS=	ncmerge.c
HDRS=	
OBJS=	ncmerge.o
PROG=	ncmerge

.c.o:
	${CC} ${CFLAGS} -c $*.c

${PROG}: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} ${LIBS} -o $@

install: ${PROG}
	cp ${PROG} ${DESTDIR}${BINDIR}
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
