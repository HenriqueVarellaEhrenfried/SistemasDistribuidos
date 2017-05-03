/*
+------------------------------------------------------------------+
|Programa de Pós-graduação da Universidade Federal do Paraná - UFPR|
| Primeiro trabalho prático da disciplina de Sistemas Distribuídos |
|              Estudante: Henrique Varella Ehrenfried              |
|                  Professor: Elias P. Duarte Jr.                  |
|                              2017-1                              |
+------------------------------------------------------------------+
*/
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
}tnodo;
tnodo* nodo;

static int N, token, event, r, i;
static char fa_name[5];

// Função que testa um nodo a partir do token do nodo atual
int testarNodo(int token, int offset)
{
 int token2 = (token+offset)%N;
 int st = status(nodo[token2].id);

 char *c = (st==0?"SEM FALHA":"FALHO");

 printf("O nodo %d TESTOU o nodo %d como %s no tempo %5.1f\n", token, token2, c, time());
 return st;
}

// Programa Principal
int main(int argc, char * argv[])
{
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
 }

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
      st = testarNodo(token, offset++);
     }
     while (st!=0 && (((token+offset)%N)!= token));

     schedule(TEST, 30.0, token);
   break;

   case FAULT:
     r = request(nodo[token].id, token, 0);
     if(r != 0)
     {
      puts("Nao consegui falhar nodo");
      exit(1);
     }
     printf("O nodo %d FALHOU no tempo %5.1f\n", token, time());
   break;

   case REPAIR:
     release(nodo[token].id, token);
     printf("O nodo %d RECUPEROU no tempo %5.1f \n", token, time());
     schedule(TEST, 30.0, token);
   break;
  }
 }

 return 0;
}