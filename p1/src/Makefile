CC = gcc
CFLAGS = -c -Wall -Wshadow -Wvla -g
.RECIPEPREFIX = >

practica1: practica1.o
>$(CC) -o practica1 practica1.o

practica1.o: practica1.c
>$(CC) $(CFLAGS) practica1.c -lpthread

clean:
>rm *.o practica1