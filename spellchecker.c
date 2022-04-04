#include "spellchecker.h"

int port;                                
char *dictionary=DICTIONARY;        
char *output=OUTPUTFILE;                     
char **words;                            
char rString[STRINGMAX];                         
int stringSize;                              
Queue *globalLog;  
Queue *sket;    
pthread_t Plog;

int main(int argc, char** argv)
{
    if (argc==1){
        port=PORTNUM;
    }
    else if (argc==2){
        port=PORTNUM;
        dictionary=argv[1];
    }
    else if (argc==3){
        dictionary=argv[1];
        char *newPort=argv[2];
        port = atoi(newPort);
        if (port<STRINGMAX && port>49151){
            messages("Ports are in between STRINGMAX and 49151");
            return EXIT_FAILURE;
        }
    }
    else{
        messages("Error. Pleases follow the format:");
        messages("    ./myserver");
        messages("or  ./myserver [Dictionary File]");
        messages("or  ./myserver [Dictionary File] [Port]");
        return EXIT_FAILURE;
    }
    words = readDic(dictionary);
    printf("Dictionary File: [%s]\nService Port: [%d]\n", dictionary,port);
    if (service()!=EXIT_SUCCESS){
        messages("Error: Service start");
        return EXIT_FAILURE;
    }
    struct sockaddr_in client;
    unsigned int clientLen = sizeof(client);
    int isket, clientsket;
    rString[0] = '\0';
    isket = listenerS(port);
    if(isket == -1){
    printf("Port [%d] conflict. Please use another port.\n", port);
    return EXIT_FAILURE;
    }
    printf("Port [%d], sket [%d] is listening\n", port, isket);
    while(1){
      if((clientsket = accept(isket, (struct sockaddr*)&client, &clientLen)) == -1){
        printf("Error: unable to connect to client.\n");
        return -1;
      }
      JQueue(sket, clientsket);
    }
    return EXIT_SUCCESS;
}//Main

int service()
{
    time_t system_time;
    char* system_time_string;
    time(&system_time);
    system_time_string = ctime(&system_time);
    sket = cQueue();  
    globalLog = cQueue(); 
    pThread();
    FILE *fp = fopen(OUTPUTFILE, "w");
    printf("Service initialize success at %s",system_time_string);
    fprintf(fp,"Service initialize success at %s",system_time_string);
    fclose(fp);
    return EXIT_SUCCESS;
}

char **readDic(char *FILENAME)
{
    int i = 0;
    int lines = 0;
    int buff = 100;
    FILE *fp = fopen(FILENAME, "r+");
    char *word = (char*)malloc(buff*sizeof(char));
    while(fgets(word, buff, fp)!=NULL) {lines++;}
    rewind(fp);
    char **dict = (char**)malloc(lines*sizeof(char*));
    while (fgets(word, buff, fp)!=NULL){
        dict[i] = (char*)malloc(buff*sizeof(char));
        strcpy(dict[i], word);
        i++;
    }
    return dict;
}

void Rconnet(int clientsket)
{
    char c1[STRINGMAX];
    char c2[STRINGMAX];
    char cTmp[STRINGMAX];
    snprintf(c1, STRINGMAX, "Client connected at sket[%d].",clientsket);
    messages(c1);
    putQ(globalLog, c1);
    pthread_create(&Plog, NULL, &FileOutput, NULL);
    send(clientsket, MSG_WELCOME, strlen(MSG_WELCOME), 0);
    while(1){
        send(clientsket, MSG_HINT, strlen(MSG_HINT), 0);
        stringSize = recv(clientsket, rString, STRINGMAX, 0);
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
        if (strlen(rString)>=4){
            if(strcmp(rString,"exit")==0){
                send(clientsket, MSG_EXIT, strlen(MSG_EXIT), 0);
                snprintf(c2, STRINGMAX, "Client at sket[%d] disconnected.",clientsket);
                messages(c2);
                putQ(globalLog, c2);
                sleep(1);
                close(clientsket);
                break;
            }
        }
        send(clientsket, MSG_Rconnet, strlen(MSG_Rconnet), 0);
        send(clientsket, rString, stringSize, 0);
        if (sc(words, rString)==EXIT_SUCCESS){
            snprintf(cTmp, STRINGMAX,"Client at sket[%d]'s word [%s] is [OK].",clientsket,rString);
            send(clientsket, ". The spelling is OK.\n", 22, 0);
        }
        else{
            snprintf(cTmp, STRINGMAX,"Client at sket[%d]'s word [%s] is [INCORRECT].",clientsket,rString);
            send(clientsket, " . The spelling is incorrect.\n", 31, 0);
        }
        messages(cTmp);
        putQ(globalLog, cTmp);
    }
}

void *FileOutput(void *arg)
{
    while(1){
        char* entry = popLog(globalLog);
        FILE *wfp;
        wfp = fopen(OUTPUTFILE, "a");
        fprintf(wfp,"%s\n", entry);
        fclose(wfp);
    }
}

pthread_t *pThread(){
  pthread_t *tRtn = (pthread_t*)malloc(NUM_WORKERS*sizeof(pthread_t));
  for (int i = 0; i < NUM_WORKERS; i++){
    pthread_create(&tRtn[i], NULL, &GetSkt, &i);
  }
  return tRtn;
}

void *GetSkt()
{
    while(1){
        int iSock = pop(sket);
        Rconnet(iSock);
    }
}

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

int listenerS(int port)
{
    int listenfd, optval=1;
    struct sockaddr_in ServerAddr;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }
     if (setsockopt(listenfd,SOL_SOCKET, SO_REUSEADDR,
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

int stop()
{
    return EXIT_SUCCESS;
}

void messages(char *mymessage)
{
    printf("%s\n",mymessage);
}
