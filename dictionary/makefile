TARGETS=${patsubst %.c,%,${wildcard ./*.c}}

default:	${TARGETS}

%:	%.c
	${CC} -o $@ $< -L../lib/bt/lib -lbt -I../lib/bt/inc ../soundEx/soundex.o

clean:
	rm -f ${TARGETS}