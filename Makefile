CC = gcc
RM = del

all:
	$(CC) chip8.c -o chip8 -Wall -lmingw32 -lSDL2main -lSDL2
clean:
	$(RM) *.exe