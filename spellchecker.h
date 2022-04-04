#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>



#define PORTNUM 1100
#define DICTIONARY "dictionary.txt"
#define OUTPUTFILE "output.txt"
#define NUM_WORKERS 2
#define MAX_QUEUES 20
#define STRINGMAX 1024
#define MSG_WELCOME "***Welcome to use network spell checking system.***\n***You can enter a word or [exit] for exit system.***\n"
#define MSG_HINT "Enter a word :>"
#define MSG_Rconnet "You sent "
#define MSG_ERROR "Client sent a illegal string."
#define MSG_EMTY_ERROR "Client just sent a enter."
#define MSG_EXIT "Thank you for your using!\n"

typedef struct MyNode{
    int emID;
    char *cMsg;
    struct MyNode *next;
}MyNode;

typedef struct Queue{
  MyNode *head;
  MyNode *tail;
  int size;
  int counter;
  int capacity;
  pthread_mutex_t mutex;
  pthread_cond_t isEmpty;
  pthread_cond_t isFull;
}Queue;

pthread_t *pThread();
void *GetSkt();
void JQueue(Queue *qe, int emID);
int pop(Queue *qe);
void putQ(Queue *qe, char *cMsg);
char* popLog(Queue *qe);
void Rconnet(int clientsket);
char **readDic(char *FILENAME);
int sc(char **dict, char *word);
MyNode *cNode(int emID, char *cMsg);
Queue *cQueue();
void messages(char *mymessage);
void *FileOutput(void *arg);
int listenerS(int);
int service();
int stop();
