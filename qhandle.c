#include "myproject.h"

// create a new node
MyNode *cNode(int emID, char *cMsg)
{
    MyNode *temp = (MyNode*)malloc(sizeof(MyNode));
    temp->emID = emID;
    temp->cMsg = cMsg;
    temp->next = NULL;
    return temp;
}

// create a queue
Queue *cQueue()
{
    Queue *qe = (Queue*)malloc(sizeof(Queue));
    //init
    qe->capacity = MAX_QUEUES;
    qe->counter = 0;
    pthread_mutex_init(&qe->mutex, NULL);
    pthread_cond_init(&qe->isEmpty, NULL);
    pthread_cond_init(&qe->isFull, NULL);
    qe->head = NULL;
    qe->tail = NULL;
    qe->size = 0;
    return qe;
}

//Job Queue Put handling
void JQueue(Queue *qe, int emID)
{
    //creates a new node for the qe
    MyNode *temp = cNode(emID,"");
    if (pthread_mutex_lock(&qe->mutex)!=0){
        messages("Queue lock error.");
    }
    //check if wait a empty queue
    while(qe->counter == qe->capacity){
        if (pthread_cond_wait(&qe->isEmpty, &qe->mutex) != 0){
          messages("Queue is empty.");
        }
    }
    //Is max capacity on queue
    if (qe->size == MAX_QUEUES){
        messages("Queue is full.");
    }
    //check starting at size 0 or queue is empty
    if (qe->tail == NULL || qe->size == 0){
        qe->head = temp;
        qe->tail = temp;
    }else{
        qe->tail->next = temp;
        qe->tail = temp;
    }
    //increase size and counter
    qe->size++;
    qe->counter++;
    //Full slot error
    if (pthread_cond_signal(&qe->isFull) != 0){
        messages("Slot is full!");
    }
    //check if it can unlock
    if (pthread_mutex_unlock(&qe->mutex) != 0){
        messages("Could not unlock mutex");
    }
}

//Job Queue Pop handling
int pop(Queue *qe)
{
    if (pthread_mutex_lock(&qe->mutex) != 0){
        messages("Queue lock error.");
    }
    while(qe->counter == 0){
        if (pthread_cond_wait(&qe->isFull, &qe->mutex)){
          messages("Slot is full.");
        }
    }
    //remove a node
    MyNode *temp = qe->head;
    qe->head = qe->head->next;
    qe->size--;
    qe->counter--;
    //attempt to signal producers
    if (pthread_cond_signal(&qe->isEmpty)!=0){
      messages("Queue is empty.");
    }
    //unlock the thread
    if (pthread_mutex_unlock(&qe->mutex)!=0){
      messages("Cannot unlock mutex!");
    }
    return temp->emID;
}

//Log Queue Put handling
void putQ(Queue *qe, char* cMsg)
{
    //creates a new node for the queue
    MyNode *temp = cNode(0,cMsg);
    //lock queue
    if (pthread_mutex_lock(&qe->mutex)!=0){
        messages("Queue lock error.");
    }
    while(qe->counter == qe->capacity){
        if (pthread_cond_wait(&qe->isEmpty, &qe->mutex) != 0){
            messages("Queue is empty.");
        }
    }
    //Queue is full
    if (qe->size == MAX_QUEUES){
        messages("Queue is full.");
    }
    if (qe->tail == NULL || qe->size == 0){
        qe->head = temp;
        qe->tail = temp;
    }
    else{
        qe->tail->next = temp;
        qe->tail = temp;
    }
    qe->size++;
    qe->counter++;
    if (pthread_cond_signal(&qe->isFull) != 0){
        messages("Slot is full!");
    }
    if (pthread_mutex_unlock(&qe->mutex) != 0){
        messages("Could not unlock mutex");
    }
}

//Log Queue Pop handling
char *popLog(Queue *qe)
{
    if (pthread_mutex_lock(&qe->mutex) != 0){
        messages("Queue lock error.");
    }
    while(qe->counter == 0){
        if (pthread_cond_wait(&qe->isFull, &qe->mutex)){
          messages("Slot is full.");
        }
    }
    //remove a node
    MyNode *temp = qe->head;
    qe->head = qe->head->next;
    qe->size--;
    qe->counter--;
    if (pthread_cond_signal(&qe->isEmpty)!=0){
      messages("slot is empty!");
    }
    if (pthread_mutex_unlock(&qe->mutex)!=0){
      messages("Cannot unlock mutex!");
    } 
    return temp->cMsg;
}
