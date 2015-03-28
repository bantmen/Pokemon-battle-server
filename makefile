PORT=30102
CFLAGS= -DPORT=\$(PORT) -g -Wall

all: battle
	
battle: battleserver.o game.o
	gcc ${CFLAGS}  -o $@ $^

%.o: %.c game.h
	gcc ${CFLAGS}  -c $<
