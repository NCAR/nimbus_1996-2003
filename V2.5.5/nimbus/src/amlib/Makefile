DESTDIR=

SUBDIR= std user xlate
CC= "CC=gcc"
CFLAGS= "COPS=-O2"

all: ${SUBDIR}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS} ${CC} ${CFLAGS}
#	cd $@; pmake -k ${MFLAGS} ${CC} ${CFLAGS}

install clean lint print:
	for i in ${SUBDIR}; do						\
	    (cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} $@); done
#	    (cd $$i; pmake -k ${MFLAGS} DESTDIR=${DESTDIR} $@); done

FRC:
