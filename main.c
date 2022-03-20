#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "Simulation.h"
#include "ThreadProcess.h"

int main(int argc, char *argv[])
{

  if (!(argc == 4 || argc == 5))
  {
    printf("Uso correcto:\n \
    ./main.ex [0: LRU | 1: SECOND CHANCE] \
    [Numero de RAM Frames] \
    [Entradas trace secuenciales por proceso (q)] \
    (Cantidad maxima de traces a leer) \n");
    return 1;
  }

  // Argumentos de Ejecucion
  int chosenAlgorithm = atoi(argv[1]);
  if (chosenAlgorithm != 0 && chosenAlgorithm != 1)
  {
    printf("Algoritmo invalido \n0 para LRU o 1 para Second Chance.\n");
    return -1;
  }
  int framesNumber = atoi(argv[2]);
  if (framesNumber < 2)
  {
    printf("Cantidad de frames debe ser mayor a 2\n");
    return -1;
  }
  int q = atoi(argv[3]);
  int maxTraceEntries = RAND_MAX;
  if (argc == 5)
    maxTraceEntries = atoi(argv[4]);

  SharedBlock *sharedMemory = calloc(1, sizeof(SharedBlock));

  // Inicializar valores de memoria compartida
  sharedMemory->tableSize = framesNumber > DEFAULT_TABLE_SIZE ? framesNumber : DEFAULT_TABLE_SIZE;
  sharedMemory->chosenAlgorithm = chosenAlgorithm;
  sharedMemory->q = q;
  sharedMemory->maxTraceEntries = maxTraceEntries;
  sharedMemory->iterations = 0;
  sharedMemory->numOfReads = sharedMemory->numOfWrites = 0;
  sharedMemory->pageFaults = sharedMemory->hits = sharedMemory->updates = 0;
  sharedMemory->gccTraceBits = sharedMemory->bzipTraceBits = 0;
  // Crear el queue (shared page table)
  sharedMemory->queue = createQueue(framesNumber);

  pthread_t t1, t2;

  // Crear threads
  if (pthread_create(&t1, NULL, ThreadProcess1, sharedMemory) != 0)
    return -1;
  if (pthread_create(&t2, NULL, ThreadProcess2, sharedMemory) != 0)
    return -1;

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  // ======= Imprimir resultados en consola =======
  printf("\n>>>>>Memory status<<<<<\n");
  printQueue(sharedMemory->queue);
  printf("\nQueue size is %d\n", sharedMemory->queue->occupiedPages);

  printf("\nTotal iterations: %d\n", sharedMemory->iterations);
  printf("Total P1 (gcc) iterations: %d\n", sharedMemory->gccTraceBits);
  printf("Total P2 (bzip) iterations: %d\n", sharedMemory->bzipTraceBits);
  printf("Total page faults: %d\n", sharedMemory->pageFaults);
  printf("Total hits: %d\n", sharedMemory->hits);
  printf("Total updates: %d\n", sharedMemory->updates);
  printf("Total reads: %d\n", sharedMemory->numOfReads);
  printf("Total writes: %d\n", sharedMemory->numOfWrites);

  free(sharedMemory);

  return 0;
}
