#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Simulation.h"

// ============= Page =============

Page *createPage(unsigned int trace, char type, unsigned int pid)
{

    Page *node = (Page *)malloc(sizeof(Page));

    node->trace = trace;
    node->pageNumber = trace / PAGE_SIZE;
    node->offset = trace % PAGE_SIZE;
    node->traceType = type;
    node->proccessId = pid;
    node->isInCache = 0;

    return node;
}

void deletePage(Page *nodeToDelete)
{
    free(nodeToDelete);
}

void printPage(HashTable *table, Page *page)
{

    if (page == NULL)
        return;

    if ((page = searchHashTable(table, page->pageNumber, page->proccessId)) == NULL)
    {
        printf("%d does not exist\n", page->trace);
        return;
    }
    else
    {
        printf("Trace:%8x | Page Number:%6d | Offset:%4d | Type:%2c",
               (page->trace * PAGE_SIZE + page->offset), page->pageNumber,
               page->offset, page->traceType);
    }
}

void deleteOverflowBucket(HashTable *table)
{
    LinkedList **bucket = table->overflowBucket;
    for (int i = 0; i < table->size; i++)
        deleteList(bucket[i]);
    free(bucket);
}

LinkedList *createList()
{
    LinkedList *list = (LinkedList *)calloc(1, sizeof(LinkedList));
    return list;
}

LinkedList *insertToList(LinkedList *list, Page *newNode)
{

    if (!list)
    {
        LinkedList *head = createList();
        head->page = newNode;
        head->next = NULL;
        list = head;
        return list;
    }
    else if (list->next == NULL)
    {
        LinkedList *node = createList();
        node->page = newNode;
        node->next = NULL;
        list->next = node;
        return list;
    }

    LinkedList *temp = list;
    while (temp->next)
    {
        temp = temp->next;
    }

    LinkedList *node = createList();
    node->page = newNode;
    node->next = NULL;
    temp->next = node;

    return list;
}

void deleteList(LinkedList *list)
{

    LinkedList *temp = list;
    if (!list)
        return;

    while (list)
    {
        temp = list;
        list = list->next;
        free(temp->page);
        free(temp);
    }
}

LinkedList **createOverflowBucket(HashTable *table)
{

    LinkedList **bucket = (LinkedList **)calloc(table->size, sizeof(LinkedList *));
    for (int i = 0; i < table->size; i++)
        bucket[i] = NULL;
    return bucket;
}

// ============= HashTable =============

unsigned long hashFunction(unsigned int trace, unsigned int size)
{
    unsigned long i = 0;
    i = ((trace >> 16) ^ trace) * 0x45d9f3b;
    i = ((i >> 16) ^ i) * 0x45d9f3b;
    i = ((i >> 16) ^ i);
    return i % size;
}

HashTable *createHashTable(int size)
{

    HashTable *table = (HashTable *)malloc(sizeof(HashTable));
    table->size = size;
    table->count = 0;
    table->totalPages = 0;
    table->pageArray = (Page **)calloc(table->size, sizeof(Page *));
    for (int i = 0; i < table->size; i++)
        table->pageArray[i] = NULL;
    table->overflowBucket = createOverflowBucket(table);

    return table;
}

void deleteHashTable(HashTable *table)
{

    for (int i = 0; i < table->size; i++)
    {
        Page *item = table->pageArray[i];
        if (item != NULL)
            deletePage(item);
    }

    deleteOverflowBucket(table);
    free(table->pageArray);
    free(table);
}

void handleCollision(HashTable *table, unsigned long index, Page *collidedNode)
{

    LinkedList *head = table->overflowBucket[index];

    if (head == NULL)
    {
        // Primer item de la lista
        head = createList();
        head->page = collidedNode;
        table->overflowBucket[index] = head;
        table->totalPages++;
        return;
    }
    else
    {
        // Insertar a la lista
        table->overflowBucket[index] = insertToList(head, collidedNode);
        table->totalPages++;
        return;
    }
}

void insertToHashTable(HashTable *table, Page *page)
{

    // En caso de que el nodo ya este en la hash table
    if (searchHashTable(table, page->pageNumber, page->proccessId) != NULL)
        return;

    // Crear item
    Page *nodeToInsert = createPage(page->trace, page->traceType, page->proccessId);

    // Generar indice
    int index = hashFunction(nodeToInsert->pageNumber, table->size);

    Page *current_item = table->pageArray[index];

    if (current_item == NULL)
    {
        if (table->count == table->size)
        {
            // Hash Table llena
            printf("Insert Error: Hash Table is full\n");
            // Eliminar item
            deletePage(nodeToInsert);
            return;
        }
        // Insertar
        table->pageArray[index] = nodeToInsert;
        table->count++;
        table->totalPages++;
    }
    else
    {
        // Caso 1: Solo es necesario actualizar el nodo
        if (current_item->trace == nodeToInsert->trace &&
            current_item->proccessId == nodeToInsert->proccessId)
        {
            if (current_item->traceType == 'R' && nodeToInsert->traceType == 'W')
                current_item->traceType = 'W';
            return;
        }
        else
        {
            // Caso 2: Colision
            handleCollision(table, index, nodeToInsert);
            return;
        }
    }
}

// Busca el oage number en la hash table
// y retorna NULL si no existe
Page *searchHashTable(HashTable *table, unsigned int pageNumber, int pid)
{

    int index = hashFunction(pageNumber, table->size);
    Page *requestedNode = table->pageArray[index];
    LinkedList *head = table->overflowBucket[index];

    // Buscamos una pagina con el mismo numero de pagina y process id
    while (requestedNode != NULL)
    {
        if (requestedNode->pageNumber == pageNumber &&
            requestedNode->proccessId == pid)
            return requestedNode;
        if (head == NULL)
            return NULL;
        requestedNode = head->page;
        head = head->next;
    }

    return NULL;
}

void printHashTable(HashTable *table)
{
    printf("\n==============\n");
    for (int i = 0; i < table->size; i++)
    {
        if (table->pageArray[i])
        {
            printf("||Index %4d|| ", i);
            printPage(table, table->pageArray[i]);
            if (table->overflowBucket[i])
            {
                LinkedList *head = table->overflowBucket[i];
                while (head)
                {
                    printf("  ==>  ");
                    printPage(table, head->page);
                    head = head->next;
                }
            }
            printf("\n");
        }
    }
    printf("==============\n\n");
}

// ============= HashTable =============

// =============   Queue   =============

Queue *createQueue(int numberOfPages)
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));

    queue->occupiedPages = 0;
    queue->maxPages = numberOfPages;
    queue->front = queue->rear = -1;
    queue->pageArray = malloc(numberOfPages * sizeof(Page));

    // Inicializar cola
    for (int i = 0; i < queue->maxPages; i++)
    {
        queue->pageArray[i].trace = 0;
        queue->pageArray[i].pageNumber = 0;
        queue->pageArray[i].offset = 0;
        queue->pageArray[i].traceType = 'R';
        queue->pageArray[i].proccessId = 0;
        queue->pageArray[i].isInCache = 1;
    }

    return queue;
}

int isQueueEmpty(Queue *queue)
{
    return queue->occupiedPages == 0;
}

int isQueueFull(Queue *queue)
{
    return queue->occupiedPages == queue->maxPages;
}

void insertToQueue(Queue *queue, Page *page)
{

    // Primera incersion
    if (queue->front == -1)
        queue->front = 0;

    if (isQueueFull(queue))
        return;
    else
    {

        queue->rear++;
        // Insertar nueva pagina
        queue->pageArray[queue->rear].trace = page->trace;
        queue->pageArray[queue->rear].pageNumber = page->pageNumber;
        queue->pageArray[queue->rear].offset = page->offset;
        queue->pageArray[queue->rear].traceType = page->traceType;
        queue->pageArray[queue->rear].proccessId = page->proccessId;
        queue->pageArray[queue->rear].isInCache = 1;
        page->isInCache = 1;

        queue->occupiedPages++;
        return;
    }
}

// Retorna el indice de una pagina en la cola (cache)
// o -1 en caso de que falle
int searchPageInQueue(Queue *queue, Page *page)
{

    if (page == NULL)
        return -1;

    for (int index = 0; index < queue->rear; index++)
        if (page->trace == queue->pageArray[index].trace &&
            page->proccessId == queue->pageArray[index].proccessId)
            return index;

    return -1;
}

/*
 * Comprueba si una página está en caché y, si no lo está, la trae.
 * Si la memoria está llena, decide qué página eliminar en función de LRU.
 * Devuelve 1 si la página estaba en caché, 0 si tuvimos una falla en la página
 * o 2 en caso de actualización.
 */
int lruReferToPageInQueue(Queue *queue, HashTable *table, Page *page)
{
    Page *pageToRemove;
    Page requestedPage;
    unsigned int pageNumber = 0;
    int pid = 0;
    int index = 0;
    int update = 0;

    // Comprueba si la página está en la tabla hash
    Page *temp = searchHashTable(table, page->pageNumber, page->proccessId);
    // Encuentra la página usada menos recientemente en la cola (nodo frontal)
    if (isQueueFull(queue))
    {
        pageNumber = queue->pageArray[index].pageNumber;
        pid = queue->pageArray[index].proccessId;
        pageToRemove = searchHashTable(table, pageNumber, pid);
    }

    if (temp == NULL)
    {

        if (pageToRemove != NULL)
            pageToRemove->isInCache = 0;
        insertToHashTable(table, page);
        if (isQueueFull(queue))
        {
            for (int i = 0; i < queue->rear; i++)
            {
                queue->pageArray[i].trace = queue->pageArray[i + 1].trace;
                queue->pageArray[i].pageNumber = queue->pageArray[i + 1].pageNumber;
                queue->pageArray[i].offset = queue->pageArray[i + 1].offset;
                queue->pageArray[i].traceType = queue->pageArray[i + 1].traceType;
                queue->pageArray[i].proccessId = queue->pageArray[i + 1].proccessId;
            }
            queue->rear--;
            queue->occupiedPages--;
        }
        insertToQueue(queue, page);

        return 0;
    }

    if (!temp->isInCache)
    {

        if (pageToRemove != NULL)
            pageToRemove->isInCache = 0;
        if (isQueueFull(queue))
        {
            for (int i = 0; i < queue->rear; i++)
            {
                queue->pageArray[i].trace = queue->pageArray[i + 1].trace;
                queue->pageArray[i].pageNumber = queue->pageArray[i + 1].pageNumber;
                queue->pageArray[i].offset = queue->pageArray[i + 1].offset;
                queue->pageArray[i].traceType = queue->pageArray[i + 1].traceType;
                queue->pageArray[i].proccessId = queue->pageArray[i + 1].proccessId;
            }
            queue->rear--;
            queue->occupiedPages--;
        }
        insertToQueue(queue, temp);

        return 0;
    }
    else
    {

        requestedPage.trace = temp->trace;
        requestedPage.pageNumber = temp->pageNumber;
        requestedPage.offset = temp->offset;
        requestedPage.traceType = temp->traceType;
        requestedPage.proccessId = temp->proccessId;

        index = searchPageInQueue(queue, temp);
        if (queue->pageArray[index].traceType == 'R' && page->traceType == 'W')
            update = 1;

        for (int i = index; i < queue->rear; i++)
        {
            queue->pageArray[i].trace = queue->pageArray[i + 1].trace;
            queue->pageArray[i].pageNumber = queue->pageArray[i + 1].pageNumber;
            queue->pageArray[i].offset = queue->pageArray[i + 1].offset;
            queue->pageArray[i].traceType = queue->pageArray[i + 1].traceType;
            queue->pageArray[i].proccessId = queue->pageArray[i + 1].proccessId;
        }
        queue->pageArray[queue->rear].trace = requestedPage.trace;
        queue->pageArray[queue->rear].pageNumber = requestedPage.pageNumber;
        queue->pageArray[queue->rear].offset = requestedPage.offset;
        queue->pageArray[queue->rear].traceType = requestedPage.traceType;
        queue->pageArray[queue->rear].proccessId = requestedPage.proccessId;

        if (update)
            return 2;

        return 1;
    }

    return 0;
}

int secondChanceReferToPageInQueue(Queue *queue, HashTable *table, Page *page)
{

    Page *pageToRemove;
    Page requestedPage;
    unsigned int pageNumber = 0;
    int pid = 0;
    int update = 0;
    int index = queue->front;

    Page *temp = searchHashTable(table, page->pageNumber, page->proccessId);
    if (isQueueFull(queue))
    {
        pageNumber = queue->pageArray[queue->front].pageNumber;
        pid = queue->pageArray[queue->front].proccessId;
        pageToRemove = searchHashTable(table, pageNumber, pid);
        if (queue->pageArray[index].secondChance == 1)
        {
            pageToRemove->secondChance = 0;
            do
            {
                index++;
                pageToRemove = searchHashTable(table, queue->pageArray[index].pageNumber, queue->pageArray[index].proccessId);
            } while (pageToRemove->secondChance);
        }
    }

    if (temp == NULL)
    {
        if (pageToRemove != NULL)
            pageToRemove->isInCache = 0;
        insertToHashTable(table, page);
        if (isQueueFull(queue))
        {
            for (int i = index; i < queue->rear; i++)
            {
                queue->pageArray[i].trace = queue->pageArray[i + 1].trace;
                queue->pageArray[i].pageNumber = queue->pageArray[i + 1].pageNumber;
                queue->pageArray[i].offset = queue->pageArray[i + 1].offset;
                queue->pageArray[i].traceType = queue->pageArray[i + 1].traceType;
                queue->pageArray[i].proccessId = queue->pageArray[i + 1].proccessId;
            }
            queue->rear--;
            queue->occupiedPages--;
        }
        insertToQueue(queue, page);

        return 0;
    }

    if (!temp->isInCache)
    {

        if (pageToRemove != NULL)
            pageToRemove->isInCache = 0;
        if (isQueueFull(queue))
        {
            for (int i = index; i < queue->rear; i++)
            {
                queue->pageArray[i].trace = queue->pageArray[i + 1].trace;
                queue->pageArray[i].pageNumber = queue->pageArray[i + 1].pageNumber;
                queue->pageArray[i].offset = queue->pageArray[i + 1].offset;
                queue->pageArray[i].traceType = queue->pageArray[i + 1].traceType;
                queue->pageArray[i].proccessId = queue->pageArray[i + 1].proccessId;
            }
            queue->rear--;
            queue->occupiedPages--;
        }
        insertToQueue(queue, temp);

        return 0;
    }
    else
    {

        requestedPage.trace = temp->trace;
        requestedPage.pageNumber = temp->pageNumber;
        requestedPage.offset = temp->offset;
        requestedPage.traceType = temp->traceType;
        requestedPage.proccessId = temp->proccessId;

        index = searchPageInQueue(queue, temp);
        if (queue->pageArray[index].traceType == 'R' && page->traceType == 'W')
            update = 1;

        while (index < queue->rear)
        {
            queue->pageArray[index].trace = queue->pageArray[index + 1].trace;
            queue->pageArray[index].pageNumber = queue->pageArray[index + 1].pageNumber;
            queue->pageArray[index].offset = queue->pageArray[index + 1].offset;
            queue->pageArray[index].traceType = queue->pageArray[index + 1].traceType;
            queue->pageArray[index].proccessId = queue->pageArray[index + 1].proccessId;
            index++;
        }
        queue->pageArray[queue->rear].trace = requestedPage.trace;
        queue->pageArray[queue->rear].pageNumber = requestedPage.pageNumber;
        queue->pageArray[queue->rear].offset = requestedPage.offset;
        queue->pageArray[queue->rear].traceType = requestedPage.traceType;
        queue->pageArray[queue->rear].proccessId = requestedPage.proccessId;
        queue->pageArray[queue->rear].secondChance = 1;

        temp->secondChance = 1;

        if (update)
            return 2;

        return 1;
    }

    return 0;
}

void printQueue(Queue *queue)
{

    if (queue->front == queue->rear)
        return;

    printf("=============\n");
    for (int i = 0; i < queue->maxPages; i++)
    {
        printf("Trace: %8x | Page Number: %6d | Offset: %4d | Type: %2c \n",
               queue->pageArray[i].trace * PAGE_SIZE + queue->pageArray[i].offset, queue->pageArray[i].pageNumber,
               queue->pageArray[i].offset, queue->pageArray[i].traceType);
    }
    printf("=============\n");
}
