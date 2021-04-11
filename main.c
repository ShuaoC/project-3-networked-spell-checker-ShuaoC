#include "myproject.h"

int port;                                 //server port
char *dictionary=DEFAULT_DICTIONARY;        //Dictionary file name
char *output=DEFAULT_LOG;                      //Log file name
char **words;                             // dictionary content
char rString[STRING_SIZE];                           //receive string
int stringSize;                                    //receive string size
Queue *globalLog;    //global log
Queue *sket;    //global sket
pthread_t Plog;

//main thread
int main(int argc, char** argv)
{
    if (argc==1){
        port=DEFAULT_PORT;
    }
    else if (argc==2){
        port=DEFAULT_PORT;
        dictionary=argv[1];
    }
    else if (argc==3){
        dictionary=argv[1];
        char *newPort=argv[2];
        port = atoi(newPort);
        if (port<STRING_SIZE && port>49151){
            messages("Ports are in the range STRING_SIZE to 49151");
            return EXIT_FAILURE;
        }
    }
    else{
        messages("Parameter error! Please follow the format below:");
        messages("    ./myserver");
        messages("or  ./myserver [Dictionary File]");
        messages("or  ./myserver [Dictionary File] [Port]");
        return EXIT_FAILURE;
    }
    words = readDic(dictionary);
    printf("Dictionary File: [%s]\nService Port: [%d]\n", dictionary,port);
    if (service()!=EXIT_SUCCESS){
        messages("Service start error!");
        return EXIT_FAILURE;
    }
    // init sock
    struct sockaddr_in client;
    unsigned int clientLen = sizeof(client);
    int isket, clientsket;
    rString[0] = '\0';
    isket = listenerS(port);
    if(isket == -1){
    printf("Port [%d] conflict. Please try to use another port.\n", port);
    return EXIT_FAILURE;
    }
    printf("Port [%d], sket [%d] is listening\n", port, isket);
    while(1){
      if((clientsket = accept(isket, (struct sockaddr*)&client, &clientLen)) == -1){
        printf("Error connecting to client.\n");
        return -1;
      }
      //put a job queue
      JQueue(sket, clientsket);
    }
    return EXIT_SUCCESS;
}

int service()
{
    time_t system_time;
    char* system_time_string;
    time(&system_time);
    system_time_string = ctime(&system_time);
    //init Queue
    sket = cQueue();   //Job
    globalLog = cQueue();   //Log
    //create threadpool
    pThread();
    //print start success
    FILE *fp = fopen(DEFAULT_LOG, "w");
    printf("Service initialize success at %s",system_time_string);
    fprintf(fp,"Service initialize success at %s",system_time_string);
    fclose(fp);
    return EXIT_SUCCESS;
}

int stop()
{
    return EXIT_SUCCESS;
}

//load the dictionary
char **readDic(char *FILENAME)
{
    int i = 0;
    int lines = 0;
    int buff = 100;
    //open file
    FILE *fp = fopen(FILENAME, "r+");
    //storage the dictionary
    char *word = (char*)malloc(buff*sizeof(char));
    //get line by line
    while(fgets(word, buff, fp)!=NULL) {lines++;}
    rewind(fp);
    char **dict = (char**)malloc(lines*sizeof(char*));
    //read the content
    while (fgets(word, buff, fp)!=NULL){
        dict[i] = (char*)malloc(buff*sizeof(char));
        strcpy(dict[i], word);
        i++;
    }
    return dict;
}

//Rconnet function for all client connection
void Rconnet(int clientsket)
{
    char c1[STRING_SIZE];
    char c2[STRING_SIZE];
    char cTmp[STRING_SIZE];
    snprintf(c1, STRING_SIZE, "Client connected at sket[%d].",clientsket);
    messages(c1);
    PulogQueue(globalLog, c1);
    //initialize the logger thread
    pthread_create(&Plog, NULL, &FileOutput, NULL);
    send(clientsket, MSG_WELCOME, strlen(MSG_WELCOME), 0);
    //Begin sending and receiving messages.
    while(1){
        //hint for client
        send(clientsket, MSG_HINT, strlen(MSG_HINT), 0);
        stringSize = recv(clientsket, rString, STRING_SIZE, 0);
        //handle receive message
        if(stringSize <2){
            messages(MSG_ERROR);
            send(clientsket, MSG_ERROR, strlen(MSG_ERROR), 0);
            continue;
        }else if (stringSize == 2){
            messages(MSG_EMTY_ERROR);
            continue;
        }
        rString[stringSize-2] = '\0';
        stringSize-=2;
        //check if it is exit
        if (strlen(rString)>=4){
            if(strcmp(rString,"exit")==0){
                send(clientsket, MSG_EXIT, strlen(MSG_EXIT), 0);
                snprintf(c2, STRING_SIZE, "Client at sket[%d] disconnected.",clientsket);
                messages(c2);
                PulogQueue(globalLog, c2);
                sleep(1);
                close(clientsket);
                break;
            }
        }
        //check the word spell
        send(clientsket, MSG_Rconnet, strlen(MSG_Rconnet), 0);
        send(clientsket, rString, stringSize, 0);
        if (sc(words, rString)==EXIT_SUCCESS){
            snprintf(cTmp, STRING_SIZE,"Client at sket[%d]'s word [%s] is [OK].",clientsket,rString);
            send(clientsket, ". The spelling is OK.\n", 22, 0);
        }
        else{
            snprintf(cTmp, STRING_SIZE,"Client at sket[%d]'s word [%s] is [MISSPELLED].",clientsket,rString);
            send(clientsket, " . The spelling is MISSPELLED.\n", 31, 0);
        }
        messages(cTmp);
        PulogQueue(globalLog, cTmp);
    }
}

//write the log
void *FileOutput(void *arg)
{
    while(1){
        char* entry = popLog(globalLog);
        FILE *wfp;
        wfp = fopen(DEFAULT_LOG, "a");
        fprintf(wfp,"%s\n", entry);
        fclose(wfp);
    }
}

//creates thread pool
pthread_t *pThread(){
  //allocates memory for an array of NUM_WORKERS threads
  pthread_t *tRtn = (pthread_t*)malloc(NUM_WORKERS*sizeof(pthread_t));
  //fill the array for tracking purposes
  for (int i = 0; i < NUM_WORKERS; i++){
    pthread_create(&tRtn[i], NULL, &GetSkt, &i);
  }
  return tRtn;
}

//get a sket for a thread
void *GetSkt()
{
    while(1){
        int iSock = pop(sket);
        Rconnet(iSock);
    }
}

//check the word spelling in the dictionary
int sc(char **dict, char *word){
    int i = 0;
    while(dict[i]!=NULL){
        char tmp1[100];
        strcpy(tmp1,dict[i]);
        tmp1[strlen(dict[i])-2]='\0';
        if (strcmp(tmp1, word) == 0){
            return EXIT_SUCCESS;
        }
        i++;
    }
    return EXIT_FAILURE;
}

//print out message to screen
void messages(char *mymessage)
{
    printf("%s\n",mymessage);
}

//init the listener
int listenerS(int port)
{
    int listenfd, optval=1;
    struct sockaddr_in ServerAddr;
    // Create sket
    if ((listenfd = sket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }
     if (setsockopt(listenfd, SOL_sket, SO_REUSEADDR,
     (const void *)&optval , sizeof(int)) < 0){
        return -1;
     }
     bzero((char *) &ServerAddr, sizeof(ServerAddr));
     ServerAddr.sin_family = AF_INET;
     ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
     ServerAddr.sin_port = htons((unsigned short)port);
     if (bind(listenfd, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr)) < 0){
        return -1;
     }
     if (listen(listenfd, 2) < 0){
        return -1;
     }
     return listenfd;
}
