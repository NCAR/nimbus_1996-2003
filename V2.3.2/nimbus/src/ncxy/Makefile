#
#    Makefile for ncxy
#
CC		= gcc
BIN		= /home/local/bin
ANONFTP	= /home/users/ftp/pub/src
RELEASE = /home/local/nimbus/src/ncxy
MAN1	= /home/local/man/man1
LIBS	= -lraf -lXm -lXt -lX11 -lnetcdf -lnsl -lm
LINTLIBS= -lm
INCLUDES= -I${LOCAL}/include
CFLAGS	= -O2 ${INCLUDES}

SRCS=	ncxy.c global.c init.c autoscale.c ps.c dataIO.c parmsIO.c Xwin.c \
		ccb.c Xplot.c error.c Xdialog.c misc.c sort.c search.c

OBJS=	ncxy.o global.o init.o autoscale.o ps.o dataIO.o parmsIO.o Xwin.o \
		ccb.o Xplot.o error.o Xdialog.o misc.o sort.o search.o

HDRS=	define.h extern.h
PROG=	ncxy

RESOURCE= XmNCxy

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
	cp ${PROG} $(ANONFTP)
	cp ${RESOURCE} ${XAPPLRESDIR}
	cp ${RESOURCE} ${ANONFTP}
	cp ${PROG}.man ${MAN1}/${PROG}.1

clean:
	rm -f core ${OBJS} ${PROG}

lint:
	lint ${INCLUDES} ${SRCS} ${LINTLIBS}

print:
	enscript -2Gr -b${PROG} ${HDRS} ${SRCS}

${OBJS}:	${HDRS}
dataIO.o:	${LOCAL}/include/netcdf.h
