#include <stdio.h>
#include <stdlib.h>
#include "smpl.h"

/*Neste programa cada nodo informa seu tempo*/
/*Eventos*/

#define TEST 1
#define FAULT 2
#define REPAIR 3

/*Descrito do nodo*/
typedef struct tnodo{
    int id; //Identificador do nodo/facility
}tnodo;

tnodo * nodo;

void main(int argc, char * argv[]){
    static int N, //Numero de nodos
               token,
               event, r, i;
    static char fa_name[5];
    if(argc !=2){
        puts("Uso correto: tempo <num-nodos>");
        exit(1);
    }
    N = atoi(argv[1]);
    smpl(0,"Tarefa 0 SisDis");
    reset();
    stream(1);
    nodo = (tnodo*)malloc(sizeof(tnodo)*N);
    for(i = 0; i < N; i++){
        memset(fa_name,'\0',5);
        sprintf(fa_name, '%d', i);
        nodo[i].id = facility(fa_name, 1);
    }
    /*Escalonamento de eventos*/

    for(i = 0; i < N; i++)
        schedule(TEST, 30.0, i);
    
    schedule(FAULT, 31.0, 1); //Nodo 1 falha no tempo 31
    schedule(REPAIR, 61.0, 1); //Nodo 1 recupera no tempo 61

    while(time() < 100){
        cause(&event, &token);
        switch(event){
            case TEST:
                if (status(nodo[token].id) != 0) break;
                printf("O nodo %d testa no tempo %5.1f\n", token, time());
                schedule(TEST, 30.0, token); /*Escalona próximo teste*/
                break;
            case FAULT:
                r = request(nodo[token].id, token, 0);
                if(r != 0){
                    puts("Nao consegui falhar nodo");
                    exit(1);
                }
                printf("O nodo %d falhou no tempo %5.1f\n", token, time());
                break;
            case REPAIR:
                release(nodo[token].id, token);
                printf("O nodo %d recuperou no tempo %5.1f \n", token, time());
                schedule(TEST, 30.0, token);
                break;
        }
    }
}