DESTDIR=
LIBDIR=		$(NIMBUS)/lib

LIBNAME=	xlate

INCLUDES=	-I${NIMBUS}/include -I${JLOCAL}/include -I${LOCAL}/include
DEFINES=
CFLAGS=		${COPS} ${INCLUDES} ${DEFINES}

OBJS=	APN159.o APN232.o asas.o async.o avaps.o const.o dme.o dmt_hskp.o\
	etorq.o event.o fssp.o gpsHw.o gpsPps.o gpsTans.o hgm.o irsHw.o\
	irsL51.o loranc.o ophir3.o pms1_data.o pms1_hskp.o pms1_ps.o\
	pms2_data.o pms2_hskp.o psfd.o rdma.o serial.o time.o uvhyg.o\
	grey_hskp.o tdl.o float.o AceAsia.o

SRCS=	APN159.c APN232.c asas.c async.c avaps.c const.c dme.c dmt_hskp.o\
	etorq.c event.c fssp.c gpsHw.c gpsPps.c gpsTans.c hgm.c irsHw.c\
	irsL51.c loranc.c ophir3.c pms1_data.c pms1_hskp.c pms1_ps.c\
	pms2_data.c pms2_hskp.c psfd.c rdma.c serial.c time.c uvhyg.c\
	grey_hskp.c tdl.c float.o AceAsia.c

HDRS=	${JLOCAL}/include/header.h ${NIMBUS}/include/amlib.h \
	${NIMBUS}/include/nimbus.h ${JLOCAL}/include/constants.h

.SUFFIXES: .o .h .c

.c.o:
#	${CC} -pg ${CFLAGS} -c $*.c
#	mv $*.o profiled/$*.o
	${CC} ${CFLAGS} -c $*.c

all: ${LIBDIR}/lib${LIBNAME}.a ${LIBDIR}/lib${LIBNAME}_p.a

${OBJS}: ${HDRS}
hgm.o ins.o:	${NIMBUS}/include/gust.h

${LIBDIR}/lib${LIBNAME}.a: ${OBJS}
	rm -f ${LIBDIR}/lib${LIBNAME}.a
	ar cq ${LIBDIR}/lib${LIBNAME}.a ${OBJS}

${LIBDIR}/lib${LIBNAME}_p.a: ${OBJS}
#	rm -f ${LIBDIR}/lib${LIBNAME}_p.a
#	cd profiled; ar cq ${LIBDIR}/lib${LIBNAME}_p.a ${OBJS}

install: all
#	cp lib${LIBNAME}.a ${DESTDIR}/${LIBDIR}
#	cp lib${LIBNAME}_p.a ${DESTDIR}/${LIBDIR}

clean:
	rm -f *.o *.a *.ln profiled/*.o
