CC = gcc
CFLAGS = -c -Wall -Wshadow -Wvla -g
.RECIPEPREFIX = >

cyclictestURJC: cyclictestURJC.o
>$(CC) -o cyclictestURJC cyclictestURJC.o

cyclictestURJC.o: cyclictestURJC.c
>$(CC) $(CFLAGS) cyclictestURJC.c -lpthread

clean:
>rm *.o cyclictestURJC