#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#define DEFAULT_PORT 1100
#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_LOG "output.txt"
#define NUM_WORKERS 2
#define MAX_QUEUES 20
#define STRING_SIZE 1024
#define MSG_WELCOME "***Welcome to use network spell checking system.***\n***You can enter a word or [exit] for exit system.***\n"
#define MSG_HINT "Enter a word :>"
#define MSG_Rconnet "You sent "
#define MSG_ERROR "Client sent a illegal string."
#define MSG_EMTY_ERROR "Client just sent a enter."
#define MSG_EXIT "Thank you for your using!\n"

//Node structure
typedef struct MyNode{
    int emID;
    char *cMsg;
    struct MyNode *next;     //MyNode next pointer
}MyNode;

//Queue structure
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

MyNode *cNode(int emID, char *cMsg);
Queue *cQueue();
void messages(char *mymessage);
void *FileOutput(void *arg);
int listenerS(int);
int service();
int stop();
pthread_t *pThread();
void *GetSkt();
void JQueue(Queue *qe, int emID);
int pop(Queue *qe);
void putQ(Queue *qe, char *cMsg);
char* popLog(Queue *qe);
void Rconnet(int clientsket);
char **readDic(char *FILENAME);
int sc(char **dict, char *word);
