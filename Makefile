CC = gcc
RM = del

all:
	$(CC) chip8.c -Wall -lmingw32 -lSDL2main -lSDL2
clean:
	$(RM) *.exe