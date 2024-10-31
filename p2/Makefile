CC = gcc
CFLAGS = -c -Wall -Wshadow -Wvla -g
.RECIPEPREFIX = >

cyclictestURJC: cyclictestURJC.o
>$(CC) -lpthread -o cyclictestURJC cyclictestURJC.o

cyclictestURJC.o: cyclictestURJC.c
>$(CC) $(CFLAGS) -lpthread cyclictestURJC.c

clean:
>rm *.o cyclictestURJC
