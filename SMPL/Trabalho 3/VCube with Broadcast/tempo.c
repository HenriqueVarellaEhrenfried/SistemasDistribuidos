/*
+------------------------------------------------------------------+
|Programa de Pós-graduação da Universidade Federal do Paraná - UFPR|
| Terceiro trabalho prático da disciplina de Sistemas Distribuídos |
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
#define BROADCAST 4

//Constantes
#define TEST_INTERVAL 30.0
#define LATENCY_UNKNOWN -1
#define EVENT_TIME_UNKNOWN 0.0
#define EVENT_NUMBER_UNKNOWN 0
#define EVENT_UNKNOWN 0
#define EVENT_NODE_UNKNOWN 0
#define EVENT_FIRST_NODE_DETECT -1
#define EVENT_FIRST_NODE_TIME_DETECTED -1
#define TRUE 1
#define FALSE !TRUE

//Constates para criar Header na saída
#define HEADER_BAR       "+------------------------------------------------------------------+"
#define HEADER_PROGRAM   "|Programa de Pós-graduação da Universidade Federal do Paraná - UFPR|"
#define HEADER_PROJECT   "| Terceiro trabalho prático da disciplina de Sistemas Distribuídos |"
#define HEADER_STUDENT   "|              Estudante: Henrique Varella Ehrenfried              |"
#define HEADER_PROFESSOR "|                  Professor: Elias P. Duarte Jr.                  |"
#define HEADER_SEMESTER  "|                              2017-1                              |"


/*-------INÍCIO CISJ-------*/
#define POW_2(num) (1<<(num))
#define VALID_J(j, s) ((POW_2(s-1)) >= j)
/* |-- node_set.h */
typedef struct node_set {
	int* nodes;
	ssize_t size;
	ssize_t offset;
} node_set;

static node_set* set_new(ssize_t size){
	node_set* new;

	new = (node_set*)malloc(sizeof(node_set));
	new->nodes = (int*)malloc(sizeof(int)*size);
	new->size = size;
	new->offset = 0;
	return new;
}

static void set_insert(node_set* nodes, int node){
	if (nodes == NULL) return;
	nodes->nodes[nodes->offset++] = node;
}

static void set_merge(node_set* dest, node_set* source){
	if (dest == NULL || source == NULL) return;
	memcpy(&(dest->nodes[dest->offset]), source->nodes, sizeof(int)*source->size);
	dest->offset += source->size;
}

static void set_free(node_set* nodes){
	free(nodes->nodes);
	free(nodes);
}
/* node_set.h --| */

node_set* cis(int i, int s){
	node_set* nodes, *other_nodes;
	int xor = i ^ POW_2(s-1);
	int j;

	/* starting node list */
	nodes = set_new(POW_2(s-1));

	/* inserting the first value (i XOR 2^^(s-1)) */
	set_insert(nodes, xor);

	/* recursion */
	for (j=1; j<=s-1; j++) {
		other_nodes = cis(xor, j);
		set_merge(nodes, other_nodes);
		set_free(other_nodes);
	}
	return nodes;
}

/*-------FIM CISJ-------*/



// Nodo
typedef struct tnodo{
    int id;
    int * state;
    double * timestamp;
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

//Cis
typedef struct tcis{
    int cluster_id;
    int* cis;
} tcis;

events evnts;
tnodo* nodo;


//Variáveis do SMPL
static int N, half_N, token, event, r, i;
static char fa_name[5];

int eventCounter; //Variável que contém quantidade de eventos que ocorreram no sistema
int  testCounter; //Variável com a quantidade de testes executados no sistema
char ** tokens; //Vetor com as strings tokenizadas
int numNotFailed = 0, auxFail = 0, numFailed = 0; //Variáveis para evento
static int N_CLUSTERS; //Variável que contém o número de clusters necessários para o programa 
int roundTest = 0; //Variável com o número da rodada de teste para facilitar a escolha do cluster que será testado
int * tested; //Variável com os nodos testados

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
        if((time > N_CLUSTERS*TEST_INTERVAL)){
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
    printf("\tContador de testes: %d\n ", testCounter);
    printf("\n\tVetores STATE: \n");
    for(i = 0; i < N; i++) {
        printf("\t");
        printArray(((N-token)+(token+i))%N);
    }
    puts("\n---------------------------\n");
}

int isInTested(int qNode){
    int i;
    for(i = 0; i < N; i++){
        if(qNode == tested[i]){
            return TRUE;
        } 
    }
    return FALSE;
}

void cleanTested(){
    int i;
    for(i = 0; i < N; i++){
        tested[i]=-1;
    }
}

void addInTested(int n){
    int i;
    for(i = 0; i < N; i++){
        if(i == n){
            tested[i] = n;
            break;
        }
    }
}

//Função para atualizar o vetor STATE de um nodo
void updateState(int token2, int st, tcis table_cis[N][N_CLUSTERS]) {
    int i, newInfoIndex, totalInfo, clusterNodes;
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
            // if(evnts.found[newInfo[newInfoIndex]]==0 && !evnts.detected)
            //     evnts.found[token] = 1;
            printf(" %d",newInfo[newInfoIndex]);
        }
        printf(" ]\n\n");
    }
    
}
//Função que testa um nodo a partir do nodo atual
int testarNodo(int token, int token2, tcis table_cis[N][N_CLUSTERS]) {
    testCounter++;
    
    int st = status(nodo[token2].id);
    char *c = (st==0?"SEM FALHA":"FALHO");
    addInTested(token2);
    if ((nodo[token].state[token2]%2) ^ (st)) {
        nodo[token].state[token2]++;
    }
    printf("O nodo %d TESTOU o nodo %d como %s no tempo %5.1f\n", token, token2, c, time());
    updateState(token2, st, table_cis);
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
//Função para criação da tabela Cis 
void createTableCis(tcis table_cis[N][N_CLUSTERS]){
    int i,j,k,num_nodes;
    node_set* nodes;
    for(i=0;i<N;i++){
        for(j=0;j<N_CLUSTERS;j++){
            num_nodes = pow(2,j);
            table_cis[i][j].cluster_id = j+1; 
            table_cis[i][j].cis = (int*)malloc(sizeof(int)*num_nodes);
            nodes = cis(i, j+1);
            for(k = 0; k < num_nodes; k++){
                table_cis[i][j].cis[k] = nodes->nodes[k];
            }
        }
    }
}
//Função que imprime a tabela Cis : NOTA - Esta função está otimizada para no máximo 3 clusters,
//mesmo funcionando para menos clusters
void printTableCis(tcis table_cis[N][N_CLUSTERS]){
    int i,j,k,x;
    puts("Tabela Cis");
    for(i = 0; i < N; i++){
        printf("+---------");
    }
    printf("+---------+\n");
    printf("|    S    ");
    for(i = 0; i < N; i++){
        printf("|    %d    ",i);
    }
    printf("|\n");
    for(i = 0; i < N; i++){
        printf("+---------");
    }
    printf("+---------+\n");
    int spaces;
    for(j = 0; j < N_CLUSTERS; j++){
        for(i = 0; i < N; i++){
            if(i==0){
                printf("|    %d    ",j+1);
            }
            int num_nodes_to_print = pow(2,table_cis[i][j].cluster_id-1);
            
            switch(num_nodes_to_print){
                case 1:
                    spaces = 4;
                    break;
                case 2: 
                    spaces = 3;
                    break;
                case 4:
                    spaces = 1;
                    break;
            }
            printf("|");
            for(x = 0; x < spaces; x++)
                    printf(" ");
            for(k = 0; k < num_nodes_to_print; k++){
                if(k+1 == num_nodes_to_print)
                    printf("%d",table_cis[i][j].cis[k]);
                else 
                    printf("%d ",table_cis[i][j].cis[k]);
            }
            for(x = 0; x < spaces; x++)
                    printf(" ");
        }
            
            printf("|\n");
    }
    for(i = 0; i < N; i++){
        printf("+---------");
    }
    printf("+---------+\n");
}
//Função que calcula os nodos que o o nodo token vai testar
int * calculateTests(int currentCluster, tcis table_cis[N][N_CLUSTERS]){
    int i, j, k;
    int * toTest;
    int numNodes = pow(2,currentCluster-1);
    
    toTest = (int*)malloc(sizeof(int)*(N-1));

    for(i = 0; i < N; i++){
        toTest[i] = -1;
    }

    for(i = 0, k = 0; i < N; i++){
        for(j = 0; j < numNodes; j++){
            // printf("TOKEN: %d  |  I: %d  |  J: %d  |  CIS: %d  |  STATUS: %d  |  CLUSTER: %d\n", token, i, j, table_cis[i][currentCluster-1].cis[j], status(nodo[table_cis[i][currentCluster-1].cis[j]].id), currentCluster-1);
           if(status(nodo[table_cis[i][currentCluster-1].cis[j]].id) == 0){ 
                if(table_cis[i][currentCluster-1].cis[j]==token){
                    toTest[k] = i;
                    k++;
                }
                else{
                    break;
                }
           }
        }
    }
    printf("\tO nodo: %d deve testar o(s) nodo(s): ", token);
    printvec(k, toTest);
    return toTest;
}
int numNodosTest(int testes_token[N-1]){
    int i, sum;
    for (i = 0, sum = 0; i < N-1; i++){
        if(testes_token[i] != -1){
            sum++;
        }
    }
    return sum;
}
printvec(int size, int*array){
    int i;
    printf("[ ");
    for(i = 0; i < size; i++){
        printf("%d ",array[i]);
    }
    printf("]\n");
}
int numberOfDigits(int n){
    if (n == 0)
        return 0;
    return floor( log10( abs( n ) ) ) + 1;
}
void printRoundTest(){
    int auxPrint;
    printf("\t\t\t+");
    for(auxPrint = 0; auxPrint < numberOfDigits(roundTest)-1; auxPrint++)
        printf("-");
    puts("------------------------+");
    printf("\t\t\t|Rodada de teste atual: %d|\n",roundTest);
    printf("\t\t\t+");
    for(auxPrint = 0; auxPrint < numberOfDigits(roundTest)-1; auxPrint++)
        printf("-");
    puts("------------------------+");
}
void sendMessengeBroadcast(){
    
}
// Programa Principal
int main(int argc, char * argv[]){
    //Imprime header
    printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n",HEADER_BAR, HEADER_PROGRAM,HEADER_PROJECT,HEADER_STUDENT,HEADER_PROFESSOR,HEADER_SEMESTER,HEADER_BAR);

    char * configuracao; //Variável para leitura das configurações da simulação
    int lat; //Variável para identificar se um evento foi diagnosticado
    int j, lastEventCounter = 0; //Variáveis auxiliares
    int * testes_token;
    int nodos_a_testar;
    //Verifica número de argumentos

    if(argc > 1) {
        puts("\n\nUso correto: tempo < arquivo.conf\n");
        exit(1);
    }

    // Lê da entrada os parâmetro para a simulação (Tempo, número de nodos e eventos)
    split(readinput(), " \n");

    //Extrai tempo de simulção e N da entrada
    double simulationTime = strtod(tokens[0], NULL);
    N = atoi(tokens[1]);

    half_N = (N/2); // Define a metade dos nodos
    N_CLUSTERS = (int)ceil(log2(N)); // Define a quantidade de clusters necessários
    //Define tabela Cis
    tcis table_cis[N][N_CLUSTERS];
    int k;
    createTableCis(table_cis);
    printTableCis(table_cis);

    //Define tempo de warm up neste caso foi definido que é Log2(N)+1 para garantir que todos os elementos
    //dos vetores STATE de todos os nodos sejam zero. Entretanto o valor correto é Log2(N). Na simulção para
    //garantir a corretude do algoritmo foi preciso adicionar 1 para que o vetor STATE do nodo zero na posição
    //zero seja zero no final do warmup, caso fosse utilizado o valor correto de Log2(N) o resultado da primeira 
    //posição do vetor STATE do nodo zero ficava como -1.
    int warmUpTime = (N_CLUSTERS+1)*(int)(TEST_INTERVAL);
    
    //Inicializa simulção inicializando as variáveis necessárias
    smpl(0, "Trabalho pratico 2 - Sistemas Distribuidos");
    reset();
    stream(1);
    nodo = (tnodo*)malloc(sizeof(tnodo)*N);    
    tested = (int*)malloc(sizeof(int)*N);    
    newEvent(EVENT_TIME_UNKNOWN,EVENT_NUMBER_UNKNOWN,EVENT_UNKNOWN,EVENT_NODE_UNKNOWN);

    for(i = 0; i < N; i++) {
        memset(fa_name,'\0',5);
        sprintf(fa_name, "%d", i);
        nodo[i].id = facility(fa_name, 1);
        nodo[i].state = (int*)malloc(sizeof(int)*N);
        nodo[i].timestamp = (double*)malloc(sizeof(double)*N);
        for(j = 0; j < N; j++) {
            nodo[i].state[j] = -1;
        }
    }

    eventCounter = 0;

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

    int printedEndOfWarmUp = 0, token2;

    // Checagem de eventos
    //Faz a simulação acontecer
    printf("************************** COMECOU O WARMUP **************************\n\n");
    float timeNow = time();
    int shouldPrintRoundTest = FALSE;
    cleanTested();
    while(time() < simulationTime) {
        cause(&event, &token);
        if (timeNow != time()){
            roundTest++;
            shouldPrintRoundTest = TRUE;
            timeNow = time();
        }
        else{
            shouldPrintRoundTest = FALSE;
        }
        if((time()>(double)warmUpTime) && !printedEndOfWarmUp) {
            printf("************************** TERMINOU O WARMUP**************************\n\n");
            printedEndOfWarmUp++;
        }
        if(shouldPrintRoundTest)
            printRoundTest();
        switch(event) {
        case TEST:
            if (status(nodo[token].id) != 0) break;
            int offset = 1, st, numTestes = 0, currentCluster;
            currentCluster = roundTest%N_CLUSTERS == 0 ? N_CLUSTERS : roundTest%N_CLUSTERS;

            testes_token = calculateTests(currentCluster, table_cis);
            nodos_a_testar = numNodosTest(testes_token);
            int testando = 0;
            // Testa todos os nodos até encontrar um sem falha.
            do{
                token2 = testes_token[testando];
                st = testarNodo(token, token2, table_cis);
                numTestes++;
                testando++;
                printState("TEST");
            }while (testando < nodos_a_testar);
            schedule(TEST, TEST_INTERVAL, token);
            cleanTested();
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
        
         case BROADCAST:
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