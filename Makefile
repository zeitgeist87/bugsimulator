CC      = gcc
#CFLAGS  = -Wall -ggdb -O0
CFLAGS  = -Wall -O2
LDFLAGS = 

OBJ = simulator.o

all: simulator

simulator: $(OBJ)
	$(CC) $(CFLAGS) -o simulator $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm simulator *.o -f
