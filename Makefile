CC = gcc
RM = del

all:
	$(CC) chip8.c -lmingw32 -lSDL2main -lSDL2
clean:
	$(RM) *.exe