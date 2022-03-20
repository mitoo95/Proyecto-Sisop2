C = gcc
CFLAGS = -g -Wall
LDLIBS = -lpthread -lrt

OBJS = Simulation.o ThreadProcess.o
TARGET = main.ex

all: $(TARGET)

%.ex: %.o $(OBJS)
	$(C) $(CFLAGS) $< $(OBJS) -o $@ $(LDLIBS)

main: main.c
	$(C) $(CFLAGS) -c main.c
	
Simulation: Simulation.c
	$(C) $(CFLAGS) -c Simulation.c

ThreadProcess: ThreadProcess.c
	$(C) $(CFLAGS) -c ThreadProcess.c

clean:
	rm -f *.o $(TARGET)
