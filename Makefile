CC=gcc
CFLAGS= -std=gnu99 -Wall -g 
OUTPUT=a
SOURCE=main.c 


all: 
	${CC} ${CFLAGS} -o ${OUTPUT} ${SOURCE} -lrt -lpthread
