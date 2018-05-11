#include <stdlib.h>


typedef struct queueNode_t {
    struct queueNode_t *next;
    void *elem;
} queueNode;

typedef struct queue_t {
    struct queueNode_t *head;
    struct queueNode_t *tail;
} queue;

/**
 * @return NULL en cas d'échec, une queue sinon
 *
 * intialise queue en allouant la mémoire
 */
queue *initQueue() {
    struct queue_t *queue = (struct queue_t *) malloc(sizeof(struct queue_t));
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

/**
 * @param queue: une file non NULL de type struct queue_t
 *
 * supprime la file et tous ses éléments de la mémoire
 */
void freeQueue(queue *queue) {
    if (queue == NULL) return;
    queueNode *curr = queue->head;
    while (curr != NULL) {
        queueNode *next = curr->next;
        free(curr->elem);
        free(curr);
        curr = next;
    }
    free(queue);
}

/**
 * @param queue: une file de type struct queue_t
 * @return la taille de queue
 */
int sizeQueue(queue *queue) {
    queueNode *curr = queue->head;
    int size = 0;
    while (curr != NULL) {
        size++;
        curr = curr->next;
    }
    return size;
}

/**
 * @param queue: une file de type struct queue_t
 * @return 1 si queue est vide, 0 sinon
 */
int isQueueEmpty(queue *queue) {
    return queue->head == NULL;
}

/**
 * @param elem: une chaîne de charactère
 * @param queue: une file de type struct queue_t
 * @return -1 en cas d'échec, 0 sinon
 *
 * crée un nouveau struct node et l'ajoute à la fin de queue
 */
int enqueue(void *elem, queue *queue) {
    queueNode *newNode = (struct queueNode_t *) malloc(sizeof(struct queueNode_t));
    if (newNode == NULL) {
        return -1;
    }
    newNode->elem = elem;
    newNode->next = NULL;
    if (queue->head == NULL) {
        queue->head = newNode;
        queue->tail = newNode;
    } else {
        queue->tail->next = newNode;
        queue->tail = newNode;
    }
    return 0;
}

/**
 * @param queue: une file de type struct queue_t
 * @return la chaîne de charactère du premier élément de la file
 *
 * supprime et retourne le premier élément de la queue
 */
void *dequeue(queue *queue) {
    if (queue->head == NULL) return NULL;
    void *elem = queue->head->elem;
    queueNode *next = queue->head->next;
    free(queue->head);
    queue->head = next;
    if (queue->head == NULL) queue->tail = NULL;
    return elem;
}

/**
 * @param queue: une file de type struct queue_t
 * @return la chaîne de charactère du premier élément de la file
 *
 * retourne le premier élément de la queue
 */
void *peek(queue *queue) {
    if (queue->head == NULL) return NULL;
    return queue->head->elem;
}

/*
 *@param queue: une file de struct queue 
 *@return : 1 si le nom est dedans ,0 sinon
 *
 *retourne 1 si le nom est dans la liste
 */
int checkName(queue *queue,char* toCheck){
   if(queue == NULL || queue -> head == NULL){
      return 0 ;
   }
   char* currentName = malloc(sizeof(char)*64) ;
   struct fractal* fract = (queue -> head) -> elem ;
   currentName = fractal_get_name(fract) ;
   queueNode* node = queue -> head ;
   while(node != NULL){
      if(strcmp(currentName,toCheck)){
         free(currentName) ;
         return 1 ;
      }
   node = node -> next ;
   fract = node -> elem ;
   currentName = fractal_get_name(fract) ;
   }
   free(currentName) ;
   return 0 ;
}
