#
#    Makefile for mbus to DAP translators.
#
CC=			gcc
DESTDIR=
BINDIR=		/home/local/bin
RELEASE=	/home/local/nimbus/src/dap2cdf
LIBS=		-lVarDB -lnetcdf -lraf
LINTLIBS=	${LIBS}
INCLUDES=	-I$(LOCAL)/include
CFLAGS=		-O2 ${INCLUDES}

PROG=		dap2cdf

dap2cdf: dap2cdf.c
	${CC} ${CFLAGS} dap2cdf.c ${LIBS} -o dap2cdf

install: ${PROG}
	install -m 755 -s dap2cdf ${DESTDIR}${BINDIR}

release:
	tar cFFf - . | (cd ${RELEASE}; tar xfBp -)
	sed s/-O2/-O2/g Makefile > ${RELEASE}/Makefile
	rm -f ${RELEASE}/LOCK.make

clean:
	rm -f ${OBJS} ${PROG} core
