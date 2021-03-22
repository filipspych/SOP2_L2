CC=gcc
CFLAGS= -std=gnu99 -Wall -g 
OUTPUT=a
OUTPUT2=b
SOURCE=gen.c
SOURCE2=proc.c


all: 
	${CC} ${CFLAGS} -o ${OUTPUT} ${SOURCE} -lrt
	${CC} ${CFLAGS} -o ${OUTPUT2} ${SOURCE2} -lrt
