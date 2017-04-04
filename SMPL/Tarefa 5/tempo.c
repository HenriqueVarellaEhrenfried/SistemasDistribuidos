#include <stdio.h>
#include <stdlib.h>
#include "smpl.h"

// Eventos
#define TEST 1
#define FAULT 2
#define REPAIR 3

// Nodo
typedef struct tnodo
{
 int id;
 int * state;
}tnodo;
tnodo* nodo;

static int N, token, event, r, i, eventCounter;
static char fa_name[5];


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
  printf("\n\tTEMPO: %5.1f\n\tNo evento: %s\n", time(), place);
  for(i = 0; i < N; i++){
    printf("\tEvent Counter: %d | ", eventCounter);
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
int testarNodo(int token, int offset)
{
 int token2 = (token+offset)%N;
 int st = status(nodo[token2].id);
 char *c = (st==0?"SEM FALHA":"FALHO");
 if ((nodo[token].state[token2]%2) ^ (st)){
    nodo[token].state[token2]++;
 }

 printf("O nodo %d TESTOU o nodo %d como %s no tempo %5.1f\n", token, token2, c, time());
 updateState(token2, st);
 printArray(token);
//  printf("-------------------\n");
 return st;
}

// Programa Principal
int main(int argc, char * argv[])
{
 int j;
 if(argc !=2)
 {
  puts("Uso correto: tempo <num-nodos>");
  exit(1);
 }

 N = atoi(argv[1]);
 smpl(0, "Tarefa 0 SisDis");
 reset();
 stream(1);
 nodo = (tnodo*)malloc(sizeof(tnodo)*N);


 for(i = 0; i < N; i++)
 {
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
     schedule(TEST, 30.0, i);

 schedule(FAULT, 31.0, 1); // Nodo 1 falha no tempo 31
 schedule(REPAIR, 61.0, 1); // Nodo 1 recupera no tempo 61

 // Checagem de eventos
 while(time() < 100)
 {
  cause(&event, &token);
  switch(event)
  {
   case TEST:
     if (status(nodo[token].id) != 0) break;
    
     int offset = 1, st;

     // Testa todos os nodos até encontrar um sem falha.
     do
     {
      eventCounter++;
      st = testarNodo(token, offset++);
      printState("TEST");
     }
     while (st!=0 || offset==token-1);

     schedule(TEST, 30.0, token);
   break;

   case FAULT:
     r = request(nodo[token].id, token, 0);
     eventCounter++;
     if(r != 0)
     {
      puts("Nao consegui falhar nodo");
      exit(1);
     }
     printf("O nodo %d FALHOU no tempo %5.1f\n", token, time());
     printState("FAULT");
   break;

   case REPAIR:
     eventCounter++;
     release(nodo[token].id, token);
     printf("O nodo %d RECUPEROU no tempo %5.1f \n", token, time());
     printState("REPAIR");
     schedule(TEST, 30.0, token);
   break;
  }

  
 }

 return 0;
}