#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "smpl.h"

// Eventos
#define TEST 1
#define FAULT 2
#define REPAIR 3

//Constantes
#define TEST_INTERVAL 30.0
#define LATENCY_UNKNOWN -1
#define EVENT_TIME_UNKNOWN 0.0
#define EVENT_NUMBER_UNKNOWN 0
#define EVENT_UNKNOWN 0
#define EVENT_NODE_UNKNOWN 0
#define EVENT_FIRST_NODE_DETECT -1
#define EVENT_FIRST_NODE_TIME_DETECTED -1


// Nodo
typedef struct tnodo
{
 int id;
 int * state;
}tnodo;

//Eventos
typedef struct events{
  double time;
  int eventNumber;
  int event;
  int nodeNumber;
  int * found;
  int detected;
  int nodeDetected;
  double timeFirstDetect;
}events;

events evnts;
tnodo* nodo;

static int N, token, event, r, i;
static char fa_name[5];
int size_events = 0, eventCounter, testCounter;
char ** tokens; //Vetor com as strings tokenizadas


void newEvent(double time, int eventNumber, int event, int nodeNumber){
  evnts.found = (int*)malloc(N*sizeof(int));
  evnts.time = time;
  evnts.eventNumber = eventNumber;
  evnts.event = event;
  evnts.nodeNumber = nodeNumber;
  evnts.detected = 0;
  evnts.timeFirstDetect = EVENT_FIRST_NODE_TIME_DETECTED;
  evnts.nodeDetected = EVENT_FIRST_NODE_DETECT;
}

void updateEvent(double time, int eventNumber, int event, int nodeNumber){
  int i;
  evnts.time = time;
  evnts.detected = 0;
  evnts.eventNumber = eventNumber;
  evnts.event = event;
  evnts.nodeNumber = nodeNumber;
  for(i = 0; i < N; i++){
    evnts.found[i]=0;
  }
  evnts.timeFirstDetect = EVENT_FIRST_NODE_TIME_DETECTED;
  evnts.nodeDetected = EVENT_FIRST_NODE_DETECT;
}

void printEvent(events e){
  int i;
  printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  printf("\t\t\t STATUS DO EVENTO\n");
  printf("Tempo >> %5.1lf\n", e.time);
  printf("Tempo atual >> %5.1lf\n", time());
  printf("Quando o primeiro nodo detectou >> %5.1lf\n", e.timeFirstDetect);  
  printf("Primeiro nodo que detectou >> %d\n", e.nodeDetected);
  printf("Numero do evento >> %d\n", e.eventNumber);
  printf("Evento >> %s\n", e.event==FAULT?"Falha":"Recuperação");
  printf("Nodo em que aconteceu o evento >> %d\n", e.nodeNumber);
  printf("Foi detectado? >> %s\n", e.detected==1?"Sim":"Não");
  printf("Nodos que detectaram >> [");
  for(i = 0; i < N; i++){
    printf(" %d ",e.found[i]);
  }
  puts("]");
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
}

int getLatency(double time, events e){
  int i, j, sum;
  int latency;
  int states[N];

  if(!e.detected){
    for(i = 0, sum = 0; i < N; i++){
      if(((nodo[i].state[e.nodeNumber]%2 == 0) && (e.event==REPAIR)) || ((nodo[i].state[e.nodeNumber]%2 == 1) && (e.event==FAULT))){
        sum += 1;
        e.found[i] = 1;
      }
    }    
    if(( ((sum >= N-1) && (e.event==FAULT))  || ((sum >= N) && (e.event==REPAIR)) )){
      latency =  floor((time - e.timeFirstDetect)/TEST_INTERVAL) + 1;
      printf("\n\tA latência para detectar o evento %d (que começou no tempo %5.1lf) é de %d rodada(s) de teste(s)  [Tempo atual: %5.1lf] \n\n\n", e.eventNumber, e.time, latency, time);
    }
    else{
      latency = LATENCY_UNKNOWN;
    }
    // printEvent(evnts);
    // printf("/////SUM >> %d\n/////nodeNumber >> %d\n/////LATENCY >> %d\n", sum, e.nodeNumber,latency);
  }
  return(latency);
}

void printArray(int token){
  int i = 0;
  printf("Nodo %d >> [ ", token);
  for(i = 0; i < N; i++){
    printf("%d ", nodo[token].state[i]);
  }
  puts("] ");
}
void printState(char * place){
  //Começa aqui o print com tabs
  printf("\n\tTEMPO: %5.1f\n\tAcao Executada: %s\n", time(), place);
  for(i = 0; i < N; i++){
    printf("\tEvent Counter: %d | ", eventCounter);
    printf("Test Counter: %d | ", testCounter);
    printArray(((N-token)+(token+i))%N);
  }
  puts("---------------------------");
}

void updateState(int token2, int st){
  int i = 0;
  for(i = 0; i < N; i++){
    if (nodo[token].state[i] < nodo[token2].state[i] && st == 0)
      nodo[token].state[i] = nodo[token2].state[i];
  }
}


// Função que testa um nodo a partir do token do nodo atual
int testarNodo(int token, int offset){
 testCounter++;
 int token2 = (token+offset)%N;
 int st = status(nodo[token2].id);
 char *c = (st==0?"SEM FALHA":"FALHO");
 if ((nodo[token].state[token2]%2) ^ (st)){
    nodo[token].state[token2]++;
 }
 
 printf("O nodo %d TESTOU o nodo %d como %s no tempo %5.1f\n", token, token2, c, time());
 updateState(token2, st);
 printArray(token);
 return st;
}
int words(const char sentence[ ]){
    int counted = 0; // result

    // state:
    const char* it = sentence;
    int inword = 0;

    do switch(*it) {
        case '\0': 
        case ' ':
        case '\n': 
            if (inword) { inword = 0; counted++; }
            break;
        case '\r':
            break;
        default: inword = 1;
    } while(*it++);

    return counted;
}
void split(char * string, char * delimiters){
  int arraySize = words(string)+1;
  char * token = strtok (string, delimiters); 
  free(tokens);
  tokens = (char**)malloc(arraySize*sizeof(char*));
  printf("ARRAY SIZE >> %d\n", arraySize);
  printf("STRING:\n\n\t%s",string);
  tokens[0] = token;
  i = 1;
  printf("\t%s | ID = %d\n",tokens[0],0);
  while (tokens[i-1] != NULL)  {
    tokens[i] = strtok (NULL, delimiters); // Primeiro atributo do SCHEDULE 
    printf("\t%s | ID = %d\n",tokens[i],i);
    i++;
  }
  // printf("TOTAL = %d\n",atoi(token)*atoi(numberOfNodes)*3+2);
  printf("TOTAL = %d\n",arraySize)+1;
  // return tokens;
}
char* readinput(){
  #define CHUNK 200
   char* input = NULL;
   char tempbuf[CHUNK];
   size_t inputlen = 0, templen = 0;
   do {
       fgets(tempbuf, CHUNK, stdin);
       templen = strlen(tempbuf);
       inputlen += templen;
       input = realloc(input, inputlen+1);
       strcat(input, tempbuf);
    } while (templen==CHUNK-1 && tempbuf[CHUNK-2]!='\n');
    printf("THE INPUT IS : %s \n\n",input);
    return input;
}


// Programa Principal
int main(int argc, char * argv[])
{
 char * configuracao;
 float lat;
 int j, lastEventCounter = 0;
 char str[] = "200\n3\nTEST 30.0 1\nFAIL 31.0 2";
 
 if(argc !=2){
  puts("Uso correto: tempo <num-nodos>");
  exit(1);
 }
 split(readinput(), " \n"); 
 double simulationTime = strtod(tokens[0], NULL);
 N = atoi(tokens[1]);
 int warmUpTime = N*(int)TEST_INTERVAL;
//  N = atoi(argv[1]);
 smpl(0, "Tarefa 0 SisDis");
 reset();
 stream(1);
 nodo = (tnodo*)malloc(sizeof(tnodo)*N); 
 newEvent(EVENT_TIME_UNKNOWN,EVENT_NUMBER_UNKNOWN,EVENT_UNKNOWN,EVENT_NODE_UNKNOWN);
//   for(i = 0; tokens[i+1] != NULL; i++){
//    printf("\t\t%s", tokens[i]);
//  }
 for(i = 0; i < N; i++){
  memset(fa_name,'\0',5);
  sprintf(fa_name, "%d", i);
  nodo[i].id = facility(fa_name, 1);
  nodo[i].state = (int*)malloc(sizeof(int)*N);
  for(j = 0; j < N; j++){
    nodo[i].state[j] = -1;
  }
 }
 eventCounter = 0;

 // Escalonamento de eventos
 for(i = 0; i < N; i++)
     schedule(TEST, TEST_INTERVAL, i);

//  schedule(FAULT, 91.0, 1); // Nodo 1 falha no tempo 31
//  schedule(REPAIR, 151.0, 1); // Nodo 1 recupera no tempo 61

//Escalona eventos lidos
int eventOcc;
for(i=2; tokens[i]!=NULL; i+=3){
  eventOcc = tokens[i][0] == 'F' ? FAULT : REPAIR;
  printf("\tEscalonando evento:\n\t\tEVENT OCC > %d\n\t\tTIME > %5.1lf\n\t\tNODE > %d\n---\n",eventOcc, strtod(tokens[i+1],NULL),atoi(tokens[i+2]));
  schedule(eventOcc,strtod(tokens[i+1],NULL),atoi(tokens[i+2]));
}
 printf("Tempo de simulacao = %5.1lf\nN = %d\nTempo para Warm Up = %d\n\n", simulationTime, N, warmUpTime);
 int printedEndOfWarmUp = 0;
 // Checagem de eventos
 //TODO : Permitir que possa-se definir no código os eventos assim como na tarefa 0 e Dinamicamente, como está este código
 printf("************************** COMECOU O WARMUP **************************\n");
 while(time() < simulationTime){
  cause(&event, &token);
  if((time()>(double)warmUpTime) && !printedEndOfWarmUp){
    printf("************************** TERMINOU O WARMUP**************************\n");
    printedEndOfWarmUp++;
  }
  switch(event)
  {
   case TEST:
     if (status(nodo[token].id) != 0) break;
     int offset = 1, st;
     // Testa todos os nodos até encontrar um sem falha.
     do
     {
      // eventCounter++;
      st = testarNodo(token, offset++);
      printState("TEST");
     }
     while (st!=0 || offset==token-1);

     schedule(TEST, TEST_INTERVAL, token);
   break;

   case FAULT:
     r = request(nodo[token].id, token, 0);
     eventCounter++;
     for(i = 0; i < N; i++){
      nodo[token].state[i] = -1;
     }
     updateEvent(time(), eventCounter,FAULT,token);
     if(r != 0)
     {
      puts("Nao consegui falhar nodo");
      exit(1);
     }
     printf("O nodo %d FALHOU no tempo %5.1f\n", token, time());
     printState("FAULT");
     printEvent(evnts);
   break;

   case REPAIR:
     eventCounter++;
     updateEvent(time(), eventCounter,REPAIR,token);
     release(nodo[token].id, token);
     printf("O nodo %d RECUPEROU no tempo %5.1f \n", token, time());     
     printState("REPAIR");     
     printEvent(evnts);
     schedule(TEST, TEST_INTERVAL, token);
   break;
  } 
  lat = getLatency(time(),evnts);
  if(lat != -1){
    evnts.detected = 1;
    if(lastEventCounter < evnts.eventNumber){
      lastEventCounter=evnts.eventNumber;
      printEvent(evnts);
    }
  }
   
  
  for(i = 0; i < N; i++){
    // printf("I = %d | evnts.found[i] = %d | evnts.timeFirstDetect = %5.1lf\n", i, evnts.found[i],evnts.timeFirstDetect);
    if((evnts.found[i]!=0) && (evnts.timeFirstDetect == EVENT_FIRST_NODE_TIME_DETECTED)){
      evnts.timeFirstDetect = time();
      evnts.nodeDetected = i;
    }
  }
  // printEvent(evnts);
 }
 


 puts(" ");
 return 0;
}
