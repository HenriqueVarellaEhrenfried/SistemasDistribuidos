/*
+------------------------------------------------------------------+
|Programa de Pós-graduação da Universidade Federal do Paraná - UFPR|
| Primeiro trabalho prático da disciplina de Sistemas Distribuídos |
|              Estudante: Henrique Varella Ehrenfried              |
|                  Professor: Elias P. Duarte Jr.                  |
|                              2017-1                              |
+------------------------------------------------------------------+
*/

//Bibliotecas
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

//Constates para criar Header na saída
#define HEADER_BAR       "+------------------------------------------------------------------+"
#define HEADER_PROGRAM   "|Programa de Pós-graduação da Universidade Federal do Paraná - UFPR|"
#define HEADER_PROJECT   "| Primeiro trabalho prático da disciplina de Sistemas Distribuídos |"
#define HEADER_STUDENT   "|              Estudante: Henrique Varella Ehrenfried              |"
#define HEADER_PROFESSOR "|                  Professor: Elias P. Duarte Jr.                  |"
#define HEADER_SEMESTER  "|                              2017-1                              |"


// Nodo
typedef struct tnodo
{
    int id;
    int * state;
} tnodo;

//Eventos
typedef struct events {
    double time;
    int eventNumber;
    int event;
    int nodeNumber;
    int * found;
    int detected;
    int nodeDetected;
    double timeFirstDetect;
    int numberOfTestsWhenOccured;
} events;

events evnts;
tnodo* nodo;

//Variáveis do SMPL
static int N, token, event, r, i;
static char fa_name[5];

int eventCounter; //Variável que contém quantidade de eventos que ocorreram no sistema
int  testCounter; //Variável com a quantidade de testes executados no sistema
char ** tokens; //Vetor com as strings tokenizadas
int numNotFailed = 0, auxFail = 0, numFailed = 0; //Variáveis para evento

//Função para inicializar a variável de tipo events
void newEvent(double time, int eventNumber, int event, int nodeNumber) {
    evnts.found = (int*)malloc(N*sizeof(int));
    evnts.time = time;
    evnts.eventNumber = eventNumber;
    evnts.event = event;
    evnts.nodeNumber = nodeNumber;
    evnts.detected = 0;
    evnts.timeFirstDetect = EVENT_FIRST_NODE_TIME_DETECTED;
    evnts.nodeDetected = EVENT_FIRST_NODE_DETECT;
    evnts.numberOfTestsWhenOccured = 0;
}
//Função usada para atualizar um evento com os dados do novo evento
void updateEvent(double time, int eventNumber, int event, int nodeNumber) {
    int i;
    evnts.time = time;
    evnts.detected = 0;
    evnts.eventNumber = eventNumber;
    evnts.event = event;
    evnts.nodeNumber = nodeNumber;
    for(i = 0; i < N; i++) {
        evnts.found[i]=0;
    }
    evnts.timeFirstDetect = EVENT_FIRST_NODE_TIME_DETECTED;
    evnts.nodeDetected = EVENT_FIRST_NODE_DETECT;
    evnts.numberOfTestsWhenOccured = testCounter;
}
//Função usada para imprimir os dados de um evento quando ele acontece e quando ele é diagnosticado
void printEvent(events e) {
    int i;
    printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("\t\t\t STATUS DO EVENTO\n");
    printf("Tempo em que o evento ocorreu >> %5.1lf\n", e.time);
    printf("Tempo atual >> %5.1lf\n", time());
    printf("Quando o primeiro nodo detectou >> %5.1lf\n", e.timeFirstDetect);
    printf("Primeiro nodo que detectou >> %d\n", e.nodeDetected);
    printf("Número do evento >> %d\n", e.eventNumber);
    printf("Evento >> %s\n", e.event==FAULT?"Falha":"Recuperação");
    printf("Nodo em que aconteceu o evento >> %d\n", e.nodeNumber);
    printf("Foi detectado? >> %s\n", e.detected==1?"Sim":"Não");
    printf("Número de testes executados até a detecção do evento >> %d\n", testCounter - evnts.numberOfTestsWhenOccured);
    // printf("numNotFailed >> %d\n",numNotFailed);
    // printf("numFailed >> %d\n",numFailed);
    printf("Nodos que detectaram >> [");
    for(i = 0; i < N; i++) {
        printf(" %d ",e.found[i]);
    }
    puts("]");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
}
//Função usada para calcular a latência de um evento
int getLatency(double time, events e) {
    int i, j, sum;
    int latency;
    int states[N];

    if(!e.detected) {
        for(i = 0, sum = 0; i < N; i++) {
            if(((nodo[i].state[e.nodeNumber]%2 == 0) && (e.event==REPAIR)) || ((nodo[i].state[e.nodeNumber]%2 == 1) && (e.event==FAULT))) {
                sum += 1;
                e.found[i] = 1;
            }
        }
        if((time > N*TEST_INTERVAL)){
            for(i = 0, numFailed = 0; i < N; i++){
                for(j = 0, auxFail = 0; j < N; j++){
                    auxFail+=nodo[i].state[j];
                }
                if(auxFail == (-1*N)){
                    numFailed++;
                }
            }
            if((e.event==REPAIR) && ((floor((time - e.timeFirstDetect)/TEST_INTERVAL) + 1) <= 1)){
                if(numFailed > 0){
                    numNotFailed = numFailed-1;
                }
                else{
                    numNotFailed = 0;
                }
            }
        }
        if(numNotFailed==1 && numFailed == 0){
            numNotFailed = 0;
        }
        if(( ((sum >= N-numFailed) && (e.event==FAULT))  || ((sum >= (N-numNotFailed)) && (e.event==REPAIR)) )) {
            if(sum == 1){
                evnts.timeFirstDetect=e.timeFirstDetect=time;
                evnts.nodeDetected=e.nodeDetected=token;
            }
            latency =  floor((time - e.timeFirstDetect)/TEST_INTERVAL) + 1;
            printf("\n\tA latência para detectar o evento %d (que começou no tempo %5.1lf) é de %d rodada(s) de teste(s)  [Tempo atual: %5.1lf] \n", e.eventNumber, e.time, latency, time);
        }
        else {
            latency = LATENCY_UNKNOWN;
        }
    }
    return(latency);
}
//Função para imprimir o vetor STATE
void printArray(int token) {
    int i = 0;
    printf("Nodo %d >> [ ", token);
    for(i = 0; i < N; i++) {
        printf("%d ", nodo[token].state[i]);
    }
    puts("] ");
}
//Função que imprime dados sobre os cada operação agendada (Testes, Falhas e Recuperações).
//Além dissso esta função é responsável por imprimir o vetor STATE de cada nodo, o contador
//de eventos e o contador de testes
void printState(char * place) {
    //Começa aqui o print com tabs
    printf("\n\tTempo atual: %5.1f\n\tAção Executada: %s\n", time(), place);
    printf("\tContador de eventos: %d\n ", eventCounter);
    printf("\tContador de testes: %d\n\n ", testCounter);
    printf("\n\tVetores STATE: \n");
    for(i = 0; i < N; i++) {
        printf("\t");
        printArray(((N-token)+(token+i))%N);
    }
    puts("\n---------------------------\n");
}
//Função para atualizar o vetor STATE de um nodo
void updateState(int token2, int st) {
    int i = 0, newInfoIndex = 0, totalInfo;
    int newInfo[N];
    for(i = 0, newInfoIndex = 0; i < N; i++) {
        if (nodo[token].state[i] < nodo[token2].state[i] && st == 0){
            nodo[token].state[i] = nodo[token2].state[i];
            newInfo[newInfoIndex] = i;
            newInfoIndex++; 
        }
    }
    if (newInfoIndex !=0){
        printf("O nodo %d recebeu informação sobre o(s) nodo(s): [", token);
        totalInfo = newInfoIndex;
        for(newInfoIndex = 0; newInfoIndex < totalInfo; newInfoIndex++){
            printf(" %d",newInfo[newInfoIndex]);
        }
        printf(" ]\n\n");
    }
}
//Função que testa um nodo a partir do nodo atual
int testarNodo(int token, int offset) {
    testCounter++;
    int token2 = (token+offset)%N;
    int st = status(nodo[token2].id);
    char *c = (st==0?"SEM FALHA":"FALHO");
    if ((nodo[token].state[token2]%2) ^ (st)) {
        nodo[token].state[token2]++;
    }
    printf("O nodo %d TESTOU o nodo %d como %s no tempo %5.1f\n", token, token2, c, time());
    updateState(token2, st);
    printf("\n\tEstado atual do vetor STATE do ");
    printArray(token);
    return st;
}
//Função que conta número de caracteres '\n' e ' ' para definir quantos indices deve ser considerados
//na função split
int words(const char sentence[ ]) {
    int counted = 0; // Resultado

    // Estado:
    const char* it = sentence;
    int inword = 0;

    do switch(*it) {
        case '\0':
        case ' ':
        case '\n':
            if (inword) {
                inword = 0;
                counted++;
            }
            break;
        case '\r':
            break;
        default:
            inword = 1;
        }
    while(*it++);
    return counted;
}
//Função para quebrar string em um delimitador e gerar tokens que definirão o tempo de simulação,
//número de nodos e agendamento de eventos
void split(char * string, char * delimiters) {
    int arraySize = words(string)+1;
    char * token = strtok (string, delimiters);
    free(tokens);
    tokens = (char**)malloc(arraySize*sizeof(char*));
    tokens[0] = token;
    i = 1;
    while (tokens[i-1] != NULL)  {
        tokens[i] = strtok (NULL, delimiters); // Primeiro atributo do SCHEDULE
        i++;
    }
}
//Função para leitura de dados a partir da da entrada padrão
char* readinput() {
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
    return input;
}
// Programa Principal
int main(int argc, char * argv[]){

    //Imprime header
    printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n",HEADER_BAR, HEADER_PROGRAM,HEADER_PROJECT,HEADER_STUDENT,HEADER_PROFESSOR,HEADER_SEMESTER,HEADER_BAR);

    char * configuracao; //Variável para leitura das configurações da simulação
    int lat; //Variável para identificar se um evento foi diagnosticado
    int j, lastEventCounter = 0; //Variáveis auxiliares

    //Verifica número de argumentos
    if(argc > 1) {
        puts("\n\nUso correto: tempo < arquivo.conf\n");
        exit(1);
    }

    //Lê da entrada os parâmetro para a simulação (Tempo, número de nodos e eventos)
    split(readinput(), " \n");

    //Extrai tempo de simulção e N da entrada
    double simulationTime = strtod(tokens[0], NULL);
    N = atoi(tokens[1]);

    //Define tempo de warm up
    int warmUpTime = N*(int)TEST_INTERVAL;


    //Inicializa simulção inicializando as variáveis necessárias
    smpl(0, "Trabalho pratico 1 - Sistemas Distribuidos");
    reset();
    stream(1);
    nodo = (tnodo*)malloc(sizeof(tnodo)*N);
    newEvent(EVENT_TIME_UNKNOWN,EVENT_NUMBER_UNKNOWN,EVENT_UNKNOWN,EVENT_NODE_UNKNOWN);

    for(i = 0; i < N; i++) {
        memset(fa_name,'\0',5);
        sprintf(fa_name, "%d", i);
        nodo[i].id = facility(fa_name, 1);
        nodo[i].state = (int*)malloc(sizeof(int)*N);
        for(j = 0; j < N; j++) {
            nodo[i].state[j] = -1;
        }
    }
    eventCounter = 0;

    // Agenda os eventos de teste para todos os nodos
    for(i = 0; i < N; i++)
        schedule(TEST, TEST_INTERVAL, i);

    //Agenda eventos lidos da entrada
    int eventOcc;
    int countEventsScheduled = 0;
    for(i=2; tokens[i]!=NULL; i+=3) {
        eventOcc = tokens[i][0] == 'F' ? FAULT : REPAIR;
        //Imprime dados sobre o evento que será agendado
        printf("\nAgendando evento:\n\tEvento: %s\n\tTempo: %5.1lf\n\tNodo > %d\n",eventOcc==FAULT?"Falha":"Recuperação", strtod(tokens[i+1],NULL),atoi(tokens[i+2]));
        schedule(eventOcc,strtod(tokens[i+1],NULL),atoi(tokens[i+2]));
        countEventsScheduled++;
    }
    //Imprime Estatísticas sobre a simulçao (Número de eventos, tempo de warm up e tempo de simulção)
    printf("\n\nNúmero de eventos agendados: %d\n",countEventsScheduled);
    printf("\n\nN = %d\nTempo para Warm Up = %d.0\nTempo de simulação = %5.1lf\n\n\n",N , warmUpTime, simulationTime);

    int printedEndOfWarmUp = 0;
    // Checagem de eventos
    //Faz a simulação acontecer
    printf("************************** COMECOU O WARMUP **************************\n\n");
    while(time() < simulationTime) {
        cause(&event, &token);
        if((time()>(double)warmUpTime) && !printedEndOfWarmUp) {
            printf("************************** TERMINOU O WARMUP**************************\n\n");
            printedEndOfWarmUp++;
        }
        switch(event) {
        case TEST:
            if (status(nodo[token].id) != 0) break;
            int offset = 1, st;
            // Testa todos os nodos até encontrar um sem falha.
            do{
                st = testarNodo(token, offset++);
                printState("TEST");
            }while ((st!=0) && (((token+offset)%N)!= token));

            schedule(TEST, TEST_INTERVAL, token);
            break;

        case FAULT:
            r = request(nodo[token].id, token, 0);
            eventCounter++;
            for(i = 0; i < N; i++){
                nodo[token].state[i] = -1;
            }
            updateEvent(time(), eventCounter,FAULT,token);
            if(r != 0){
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

        lat = getLatency(time(),evnts); //Verifica a latência do evento
        if(lat != LATENCY_UNKNOWN){ // Se não for indefinida, quer dizer que o evento foi identificado
            evnts.detected = 1; //Atualiza o evento
            if(lastEventCounter < evnts.eventNumber) { //Imprime dados do evento se já não o fez
                lastEventCounter=evnts.eventNumber;
                printEvent(evnts);
            }
        }

        //Verifica qual nodo identifico o evento por primeiro, e atualiza as estatísticas do evento
        for(i = 0; i < N; i++){
            if((evnts.found[i]!=0) && (evnts.timeFirstDetect == EVENT_FIRST_NODE_TIME_DETECTED)) {
                evnts.timeFirstDetect = time();
                evnts.nodeDetected = i;
            }
        }
    }
    return 0;
}