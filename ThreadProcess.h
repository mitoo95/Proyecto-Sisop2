#ifndef __THREADPROCESS__
#define __THREADPROCESS__

#include "Simulation.h"

#define PID_1 1
#define PID_2 2

typedef struct
{
  unsigned int tableSize;
  int chosenAlgorithm, q, maxTraceEntries;
  int iterations, numOfReads, numOfWrites;
  int pageFaults, hits, updates;
  int gccTraceBits, bzipTraceBits;
  Queue *queue;
} SharedBlock;

void *ThreadProcess1(void *arg);
void *ThreadProcess2(void *arg);

#endif
