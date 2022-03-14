#ifndef __SIMULATION__
#define __SIMULATION__

#define DEFAULT_TABLE_SIZE 1000
#define PAGE_SIZE 4096
#define FRAME_SIZE 4096

// struct usada en Hashtable, cola y lista
typedef struct Page
{
  unsigned int trace;
  unsigned int pageNumber;
  unsigned int offset;
  char traceType;
  unsigned int proccessId;
  // isInCache es 0 cuando la pagina no esta en el cache / 1 cuando esta en cache
  int isInCache;
  int secondChance;
} Page;

// Linked list
typedef struct LinkedList
{
  Page *page;
  struct LinkedList *next;
} LinkedList;

// Hash Table
typedef struct HashTable
{

  // array de pointers para mappear nodos
  Page **pageArray;

  // array de buckets para resolver colisiones
  LinkedList **overflowBucket;
  int size;
  int count;
  int totalPages;
} HashTable;

// Queue FIFO
typedef struct Queue
{
  unsigned occupiedPages;
  unsigned maxPages;
  int front, rear;
  Page *pageArray;
} Queue;

// Funciones de Pagina
Page *createPage(unsigned int trace, char type, unsigned int pid);
void deletePage(Page *nodeToDelete);
void printPage(HashTable *table, Page *page);

// Funciones de List
LinkedList *createList();
LinkedList *insertToList(LinkedList *list, Page *newNode);
void deleteList(LinkedList *list);
LinkedList **createOverflowBucket(HashTable *table);
void deleteOverflowBucket(HashTable *table);

// Funciones de la Hashtable
unsigned long hashFunction(unsigned int trace, unsigned int size);
HashTable *createHashTable(int size);
void deleteHashTable(HashTable *table);
void handleCollision(HashTable *table, unsigned long index, Page *collidedNode);
void insertToHashTable(HashTable *table, Page *page);
Page *searchHashTable(HashTable *table, unsigned int trace, int pid);

// Funciones del Queue
Queue *createQueue(int numberOfPages);
int isQueueFull(Queue *queue);
int isQueueEmpty(Queue *queue);
void insertToQueue(Queue *queue, Page *page);
int searchPageInQueue(Queue *queue, Page *page);
int lruReferToPageInQueue(Queue *queue, HashTable *table, Page *page);
int secondChanceReferToPageInQueue(Queue *queue, HashTable *table, Page *page);
void printQueue(Queue *queue);

#endif