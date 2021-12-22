DESTDIR=
LIBDIR=		${NIMBUS}/lib
LINTDIR=	/usr/lib/lint
MANDIR=		/usr/man/man3
INCLUDEDIR=	/usr/include

LIBNAME=	user

INCLUDES=	-I${NIMBUS}/include -I${LOCAL}/include
DEFINES=
CFLAGS=		${COPS} ${INCLUDES} ${DEFINES}

OBJS=	bits.o chemcal.o circles.o co2c.o conccn.o const.o cvi.o efield.o\
	gegg.o geopot.o licor.o ndvi.o ophir3.o prcltrk1.o prcltrk2.o raz.o\
	rlwp.o sail.o spefm.o xpsfdc.o CAINE2.o

SRCS=	bits.c chemcal.c circles.c co2c.c conccn.c const.c cvi.c efield.c\
	gegg.c geopot.c licor.c ndvi.c ophir3.c prcltrk1.c prcltrk2.c raz.c\
	rlwp.c sail.c spefm.c xpsfdc.c CAINE2.c

HDRS=	${NIMBUS}/include/nimbus.h ${NIMBUS}/include/amlib.h \
	${LOCAL}/include/constants.h ${LOCAL}/include/header.h

.SUFFIXES: .o .h .c

.c.o:
#	${CC} -pg ${CFLAGS} -c $*.c
#	mv $*.o profiled/$*.o
	${CC} ${CFLAGS} -c $*.c

all: ${LIBDIR}/lib${LIBNAME}.a ${LIBDIR}/lib${LIBNAME}_p.a

${OBJS}: ${HDRS}

${LIBDIR}/lib${LIBNAME}.a: ${OBJS}
	rm -f ${LIBDIR}/lib${LIBNAME}.a
	ar cq ${LIBDIR}/lib${LIBNAME}.a ${OBJS}

${LIBDIR}/lib${LIBNAME}_p.a: ${OBJS}
#	rm -f ${LIBDIR}/lib${LIBNAME}_p.a
#	cd profiled; ar cq ${LIBDIR}/lib${LIBNAME}_p.a ${OBJS}

lint:
	lint -C${LIBNAME} ${INCLUDES} *.c

install: all
#	cp lib${LIBNAME}.a ${DESTDIR}/${LIBDIR}
#	cp lib${LIBNAME}_p.a ${DESTDIR}/${LIBDIR}

clean:
	rm -f *.o *.a *.ln profiled/*.o
