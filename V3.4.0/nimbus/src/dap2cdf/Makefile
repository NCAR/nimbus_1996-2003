#
#    Makefile for DAP to netCDF translators.
#
CC=			gcc
DESTDIR=
BINDIR=		/jnet/linux/bin
RELEASE=	/jnet/linux/nimbus/src/dap2cdf
LIBS=		-lVarDB -lnetcdf -lnsl -lraf
LINTLIBS=	${LIBS}
INCLUDES=	-I$(JLOCAL)/include
CFLAGS=		-O2 ${INCLUDES}

PROG=		dap2cdf

${PROG}: ${PROG}.c
	${CC} ${CFLAGS} ${PROG}.c ${LIBS} -o ${PROG}

install: ${PROG}
	cp ${PROG} ${DESTDIR}${BINDIR}

release:
	tar cFFf - . | (cd ${RELEASE}; tar xfBp -)
	sed s/-O2/-O2/g Makefile > ${RELEASE}/Makefile
	rm -f ${RELEASE}/LOCK.make

clean:
	rm -f ${OBJS} ${PROG} core
