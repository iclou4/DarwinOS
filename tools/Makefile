PROGS= clrBuilder copyClr

all: clean ${PROGS}
clean:
	rm -f ${PROGS} *.clr

copyClr: copyClr.m
	${CC} -o ${.TARGET} ${.ALLSRC} -framework AppKit -framework Foundation

clrBuilder: clrBuilder.m
	${CC} -o ${.TARGET} ${.ALLSRC} -framework AppKit -framework Foundation
