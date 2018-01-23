CFLAGS = -Wall -O3
CC = gcc
OPTIONS = $(OPT)

LDFLAGS = -lm 
CFILES = testRBT.c redblack.c
OFILES = $(CFILES:%.c=%.o)

compile: $(OFILES)

	$(CC) $(CFLAGS) $(OPTIONS) -o testRBT $(OFILES) $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) $(OPTIONS) -c $< $(LDFLAGS)

clean:
	rm -f *.o testRBT
