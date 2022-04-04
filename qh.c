#include "spellchecker.h"

MyNode *cNode(int emID, char *cMsg)
{
    MyNode *temp = (MyNode*)malloc(sizeof(MyNode));
    temp->emID = emID;
    temp->cMsg = cMsg;
    temp->next = NULL;
    return temp;
}

Queue *cQueue()
{
    Queue *qe = (Queue*)malloc(sizeof(Queue));
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

void JQueue(Queue *qe, int emID)
{
    MyNode *temp = cNode(emID,"");
    if (pthread_mutex_lock(&qe->mutex)!=0){
        messages("Error: Queue lock.");
    }
    while(qe->counter == qe->capacity){
        if (pthread_cond_wait(&qe->isEmpty, &qe->mutex) != 0){
          messages("Queue is empty.");
        }
    }
    if (qe->size == MAX_QUEUES){
        messages("Error: Queue is full.");
    }
    if (qe->tail == NULL || qe->size == 0){
        qe->head = temp;
        qe->tail = temp;
    }else{
        qe->tail->next = temp;
        qe->tail = temp;
    }
    qe->size++;
    qe->counter++;
    if (pthread_cond_signal(&qe->isFull) != 0){
        messages("Error: Slot is full!");
    }
    if (pthread_mutex_unlock(&qe->mutex) != 0){
        messages("Unable to unlock mutex");
    }
}

int pop(Queue *qe)
{
    if (pthread_mutex_lock(&qe->mutex) != 0){
        messages("Error: Queue lock.");
    }
    while(qe->counter == 0){
        if (pthread_cond_wait(&qe->isFull, &qe->mutex)){
          messages("Slot is full.");
        }
    }
    MyNode *temp = qe->head;
    qe->head = qe->head->next;
    qe->size--;
    qe->counter--;
    if (pthread_cond_signal(&qe->isEmpty)!=0){
      messages("Queue is empty.");
    }
    if (pthread_mutex_unlock(&qe->mutex)!=0){
      messages("Unable to unlock mutex!");
    }
    return temp->emID;
}

void putQ(Queue *qe, char* cMsg)
{
    MyNode *temp = cNode(0,cMsg);
    if (pthread_mutex_lock(&qe->mutex)!=0){
        messages("Error: Queue lock.");
    }
    while(qe->counter == qe->capacity){
        if (pthread_cond_wait(&qe->isEmpty, &qe->mutex) != 0){
            messages("Queue is empty.");
        }
    }
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

char *popLog(Queue *qe)
{
    if (pthread_mutex_lock(&qe->mutex) != 0){
        messages("Error: Queue lock.");
    }
    while(qe->counter == 0){
        if (pthread_cond_wait(&qe->isFull, &qe->mutex)){
          messages("Slot is full.");
        }
    }
    MyNode *temp = qe->head;
    qe->head = qe->head->next;
    qe->size--;
    qe->counter--;
    if (pthread_cond_signal(&qe->isEmpty)!=0){
      messages("slot is empty!");
    }
    if (pthread_mutex_unlock(&qe->mutex)!=0){
      messages("Unable to unlock mutex!");
    } 
    return temp->cMsg;
}
