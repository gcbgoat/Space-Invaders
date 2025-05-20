CC = gcc
CFLAGS = -Wall -Wextra -g
SRCDIR = jogo
INCDIR = include
OBJS = $(SRCDIR)/main.o $(SRCDIR)/keyboard.o $(SRCDIR)/screen.o $(SRCDIR)/timer.o

all: space_invaders

space_invaders: $(OBJS)
	$(CC) $(CFLAGS) -o space_invaders $(OBJS)

$(SRCDIR)/main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $(SRCDIR)/main.c -o $(SRCDIR)/main.o

$(SRCDIR)/keyboard.o: $(SRCDIR)/keyboard.c $(INCDIR)/keyboard.h
	$(CC) $(CFLAGS) -I$(INCDIR) -c $(SRCDIR)/keyboard.c -o $(SRCDIR)/keyboard.o

$(SRCDIR)/screen.o: $(SRCDIR)/screen.c $(INCDIR)/screen.h
	$(CC) $(CFLAGS) -I$(INCDIR) -c $(SRCDIR)/screen.c -o $(SRCDIR)/screen.o

$(SRCDIR)/timer.o: $(SRCDIR)/timer.c $(INCDIR)/timer.h
	$(CC) $(CFLAGS) -I$(INCDIR) -c $(SRCDIR)/timer.c -o $(SRCDIR)/timer.o

clean:
	rm -f $(SRCDIR)/*.o space_invaders
