
PROG =	sort
SRCS =	bwstring.c coll.c file.c mem.c radixsort.c sort.c vsort.c

sort.1: sort.1.in
	sed ${MAN_SUB} ${.ALLSRC} >${.TARGET}

CLEANFILES += sort.1

CFLAGS += -DSORT_THREADS -pthread
MAN_SUB += -e 's|%%THREADS%%||g'

.include <bsd.prog.mk>
