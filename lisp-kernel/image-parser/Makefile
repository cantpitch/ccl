
OBJS = image-parser.o
CDEFINES = -D_XOPEN_SOURCE -D_UNICODE

.c.o:
	$(CC) -c $< ${CDEFINES} -g -o $@

all: image-parser

image-parser: ${OBJS}
	$(CC) -lsystem -g -o $@ ${OBJS}

clean: 
	rm ${OBJS}
	rm image-parser