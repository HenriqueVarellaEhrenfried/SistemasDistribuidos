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
#define MESSAGE_UNKNOWN -1
#define MESSAGE_TIME_UNKNOWN -1.0
#define MESSAGE_COUNTER_INIT 0
#define ALL_NODES_FROM_CLUSTER_WITH_ERROR -1
#define INITIAL_NODE_AND_CLUSTER -1
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

//Mensagem
typedef struct tmsg{
    int sender;          //Nodo remetente
    int destination;     //Nodo destino
    double timeSent;     //Tempo em que a mensagem foi enviada
    double timeReceived; //Tempo que o nodo recebeu a mensagem: -1 ou > 0
    int message;    //Pode ser qualquer coisa: int, string, float,... Neste exemplo vai ser um inteiro
    int newMessage; //Assume os valroes: TRUE or FALSE
    int received; //Assume os valroes: TRUE or FALSE
    int messageNumber; //Contador de mensagem
    int cluster; // Cluster atual em que a mensagem está
} tmsg;

//Eventos de Broadcast
typedef struct tbcast{
    double when;
    int node;
} tbcast;

events evnts;
tnodo* nodo;
tmsg mensagem;

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
int * send2; //Variável com os nodos os quais a mensagem já foi enviada
int messageCluster; //Variável que define o cluster para a mensagem braodcast
// int defaultNodeBroadcast = INITIAL_NODE_AND_CLUSTER; //Variável que determinar o nodo inicial do broadcast
// int defaultClusterBroadcast = INITIAL_NODE_AND_CLUSTER; // Variável com o cluster atual do nodo inicial do broadcast
int defaultNodeBroadcast; //Variável que determinar o nodo inicial do broadcast
int defaultClusterBroadcast; // Variável com o cluster atual do nodo inicial do broadcast
int * otherNodes2SendMessage; // Variável que contem os nodos que precisam ser enviados a mensagem. Funciona em dupla: arr[i%2==0]==Sender arr[i%2==1]==Cluster


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
//Função que verifia se um nodo (qNode) foi testado
int isInTested(int qNode){
    int i;
    for(i = 0; i < N; i++){
        if(qNode == tested[i]){
            return TRUE;
        } 
    }
    return FALSE;
}
//Função que limpa o vetor de testados
void cleanTested(){
    int i;
    for(i = 0; i < N; i++){
        tested[i]=-1;
    }
}
//Função que adiciona um nodo no vetor de testados
void addInTested(int n){
    int i;
    for(i = 0; i < N; i++){
        if(i == n){
            tested[i] = n;
            break;
        }
    }
}
//Função que verifica se um nodo (qNode) recebeu uma mensagem
int wasSend(int qNode){
    int i;
    for(i = 0; i < N; i++){
        if(qNode == send2[i]){
            return TRUE;
        } 
    }
    return FALSE;
}
//Função que limpa o vetor de nodos que receberam mensagem
void cleanSend2(){
    int i;
    for(i = 0; i < N; i++){
        send2[i]=-1;
    }
}
//Função auxiliar para adicionar um nodo no vetor de nodos que receberam mensagens
void addInSend2(int n){
    int i;
    for(i = 0; i < N; i++){
        if(i == n){
            send2[i] = n;
            break;
        }
    }
}
//Função que verifica se todos os nodos sem falha receberam a mensagem
int allNotFailedReceivedMsg(){
    int i, received, totalNotFailed;
    for(i = 0, received = 0, totalNotFailed = 0; i < N; i++ ){
        if(i == defaultNodeBroadcast){
            continue;
        }
        if (!status(nodo[i].id)){
           totalNotFailed++;
           if(wasSend(i)){
               received++;
           }
        }
    }
    if(totalNotFailed == received)
        return TRUE;
    else
        return FALSE;
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
// Função que retorna quantos nodos deverão ser testados
int numNodosTest(int testes_token[N-1]){
    int i, sum;
    for (i = 0, sum = 0; i < N-1; i++){
        if(testes_token[i] != -1){
            sum++;
        }
    }
    return sum;
}
//Função para imprimir um vetor de tamanho 'size'
printvec(int size, int*array){
    int i;
    printf("[ ");
    for(i = 0; i < size; i++){
        printf("%d ",array[i]);
    }
    printf("]\n");
}
//Função para calcular quantos dígitos um número tem
int numberOfDigits(int n){
    if (n == 0)
        return 0;
    return floor( log10( abs( n ) ) ) + 1;
}
// Função para imprimir a rodada de testes atual
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
//Função para recuperor o último índice de um vetor
int lastIndex(int * array){
    int i;
    int size = 2*N;
    for (i = 0; i < size; i++){
        if (array[i]== -1){
            return i-1;
        }
    }
    return i;
}
//Função para remover elementos do vetor otherNodesArray
void removeElements(int * array, int size){
    //Size desta função é size-1 
    int i;
    for (i = 2; i <= size; i++){
        array[i-2] = array[i];
    }
}
//Função para incializar a variável mensagem, que comtém uma mensagem
void initMessage(double time){
    mensagem.sender = MESSAGE_UNKNOWN;
    mensagem.destination = MESSAGE_UNKNOWN;
    mensagem.timeSent = MESSAGE_TIME_UNKNOWN;
    mensagem.timeReceived = MESSAGE_TIME_UNKNOWN;
    mensagem.message = MESSAGE_UNKNOWN;
    mensagem.newMessage = FALSE;
    mensagem.received = FALSE;
    mensagem.cluster = MESSAGE_UNKNOWN;
    mensagem.messageNumber = MESSAGE_COUNTER_INIT;
}
//Função para imprimir a variável mensagem
void printMessage(){
    printf("\tNodo que enviou a mensagem >> %d\n",mensagem.sender);
    printf("\tNodo destino da mensagem >> %d\n",mensagem.destination);
    printf("\tA mensagem foi enviada no tempo >> %5.1lf\n",mensagem.timeSent);
    printf("\tA mensagem foi recebida no tempo >> %5.1lf\n",mensagem.timeReceived);
    printf("\tO conteúdo da mensagem  é >> %d\n",mensagem.message);
    printf("\tA mensagem é nova? >> %s\n",mensagem.newMessage == TRUE ? "Sim" : "Não");
    printf("\tA mensagem foi recebida? >> %s\n",mensagem.received == TRUE ? "Sim" : "Não");
    printf("\tO cluster atual é >> %d\n",mensagem.cluster);
    printf("\tO contador de mensagens está em >> %d\n",mensagem.messageNumber);
    puts("/////////////////////////////////////////////////////\n");
}
//Função para imprimir a variável mensagem com simplificações
void printMessageSimpler(){
    printf("\tNodo que enviou a mensagem >> %d\n",mensagem.sender);
    printf("\tNodo destino da mensagem >> %d\n",mensagem.destination);
    printf("\tA mensagem foi enviada no tempo >> %5.1lf\n",mensagem.timeSent);
    printf("\tA mensagem foi recebida no tempo >> %5.1lf\n",mensagem.timeReceived);
    printf("\tO conteúdo da mensagem  é >> %d\n",mensagem.message);
    printf("\tO cluster atual é >> %d\n",mensagem.cluster);
    printf("\tO contador de mensagens está em >> %d\n",mensagem.messageNumber);
    puts("*****************************************************\n");
}
//Função para criar nova mensagem
void newMessage(int sender, int cluster, int destination, double actualTime, int message){
    mensagem.sender = sender;
    mensagem.cluster = cluster;
    mensagem.destination = destination;
    mensagem.timeSent = actualTime;
    mensagem.message = message; 
    mensagem.newMessage = TRUE;
    mensagem.received = FALSE;
    mensagem.messageNumber++;
    mensagem.timeReceived = MESSAGE_TIME_UNKNOWN;
}
//Função que simula a recebimento de uma mensagem
void receiveMessage(){
    if(mensagem.newMessage && !mensagem.received){
        mensagem.timeReceived = time();
        mensagem.received = TRUE;
        mensagem.newMessage = FALSE;
        addInSend2(mensagem.destination);
        puts("\n*****************************************************");
        printf("\t\tNODO %d RECEBEU MENSAGEM DO NODO %d\n\n", mensagem.destination, mensagem.sender);
        printMessageSimpler();
    }
}
//Função que simula o envio de uma mensagem
void sendMessage(int sender, int cluster, int destination, double timeNow, int message){
    newMessage(sender, cluster, destination, timeNow, message);
    puts("\n======================================================");
    printf("\t\tNODO %d ENVIOU MENSAGEM PARA O NODO %d",sender, destination);
    puts("\n======================================================\n");
    // puts("\n/////////////////////////////////////////////////////\n");
    // printMessage();
}
//Função para gerenciar o envio e recebimento de mensagem
void messageHandler(tcis table_cis[N][N_CLUSTERS], int sender, double timeNow, int message){
    int destination; 
    if (defaultClusterBroadcast == INITIAL_NODE_AND_CLUSTER){
        defaultClusterBroadcast = 1;
    }
    receiveMessage();
    if(!allNotFailedReceivedMsg()){
        messageCluster = mensagem.cluster-1;
        if(messageCluster > 0){
            destination = messageDestination(sender, messageCluster, table_cis);
            if(destination != ALL_NODES_FROM_CLUSTER_WITH_ERROR){
                sendMessage(sender, messageCluster, destination, timeNow, message);
                schedule(BROADCAST, 1.0, destination);
                messageCluster--;
                while(messageCluster > 0){
                    otherNodes2SendMessage[lastIndex(otherNodes2SendMessage)+1] = sender;
                    otherNodes2SendMessage[lastIndex(otherNodes2SendMessage)+1] = messageCluster;
                    messageCluster--;
                }
            }
            else{
                schedule(BROADCAST, 1.0, defaultNodeBroadcast);
                defaultClusterBroadcast++;
            }
        }
        else{
            messageCluster = defaultClusterBroadcast;
            destination =  messageDestination(defaultNodeBroadcast, messageCluster, table_cis);
            if(defaultClusterBroadcast <= N_CLUSTERS){  
                if(destination != ALL_NODES_FROM_CLUSTER_WITH_ERROR){
                    sendMessage(defaultNodeBroadcast, defaultClusterBroadcast, destination, timeNow, message);
                    schedule(BROADCAST, 1.0, destination);
                    defaultClusterBroadcast++;
                }
                else{
                    defaultClusterBroadcast++;
                    schedule(BROADCAST, 1.0, defaultNodeBroadcast);
                }
            }
            else{
                if(lastIndex(otherNodes2SendMessage) >= -1){
                    messageCluster = otherNodes2SendMessage[1];
                    destination =  messageDestination(otherNodes2SendMessage[0], messageCluster, table_cis);
                    if(destination != ALL_NODES_FROM_CLUSTER_WITH_ERROR){
                        sendMessage(otherNodes2SendMessage[0], messageCluster, destination, timeNow, message);
                        schedule(BROADCAST, 1.0, destination);                        
                    }
                    removeElements(otherNodes2SendMessage, lastIndex(otherNodes2SendMessage)+2); 
                }
            }
            
        }
    }
    else{
        cleanSend2();
        printf("\n>> Mensagem com origem no nodo %d foi enviada com sucesso para todos os outros nodos sem falha\n\n",defaultNodeBroadcast);
    }
}
//Função para calcular o destino da mensagem
int messageDestination(int sender, int cluster ,tcis table_cis[N][N_CLUSTERS]){
    int i, clusterSize, found, possibleNode2Send;
    found = FALSE;
    clusterSize = pow(2, cluster-1);
    for(i = 0; i < clusterSize; i++){
        possibleNode2Send = table_cis[sender][cluster-1].cis[i];
        if (!status(nodo[possibleNode2Send].id)){
            found = TRUE;
            break;
        }
    }
    // printf("\n\nFALSE: %d  |  TRUE: %d\n",FALSE, TRUE);
    // printf("\n\nSENDER: %d  |  CLUSTER: %d  |  FOUND: %d  |  POSSIBLE NODE: %d \n\n", sender, cluster, found, possibleNode2Send);
    if(found){
        return possibleNode2Send;
    }
    else{
        return ALL_NODES_FROM_CLUSTER_WITH_ERROR;
    }
}
//Função para iniciar o vetor otherNodesArray, que é responsável por guardar os próximos envios de mensagens 
void initOtherNodesArray(){
    int i;
    int size = 2*N;
    for(i = 0; i < size; i++){
        otherNodes2SendMessage[i] = -1;
    }
}
//Função que verifica se o evento de broadcast atual é um novo broadcast ou não
int isNewBroadcastEvent(tbcast * arr,  int size, double timeNow){
    //Return -1 if isn't and the index if there is
    int i;
    for(i = 0; i < size; i++){
        if(arr[i].when == timeNow){
            return i;
        }
    }
    return -1;
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
    int countBcast = 0;
    int indexBcast;
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
    smpl(0, "Trabalho pratico 3 - Sistemas Distribuidos");
    reset();
    stream(1);
    nodo = (tnodo*)malloc(sizeof(tnodo)*N);    
    tested = (int*)malloc(sizeof(int)*N); 
    send2 = (int*)malloc(sizeof(int)*N); 
    otherNodes2SendMessage = (int*)malloc(sizeof(int)*N*2); 
    cleanSend2();
    newEvent(EVENT_TIME_UNKNOWN,EVENT_NUMBER_UNKNOWN,EVENT_UNKNOWN,EVENT_NODE_UNKNOWN);
    
    initOtherNodesArray();

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
        eventOcc = tokens[i][0] == 'F' ? FAULT : (eventOcc = tokens[i][0] == 'R' ? REPAIR : BROADCAST);
        if(eventOcc==BROADCAST)
            countBcast++;
        //Imprime dados sobre o evento que será agendado
        printf("\nAgendando evento:\n\tEvento: %s\n\tTempo: %5.1lf\n\tNodo > %d\n",eventOcc==FAULT ? "Falha" : (eventOcc==REPAIR ? "Recuperação" : "Broadcast"), strtod(tokens[i+1],NULL),atoi(tokens[i+2]));
        
        schedule(eventOcc,strtod(tokens[i+1],NULL),atoi(tokens[i+2]));
        countEventsScheduled++;
    }
    //Imprime Estatísticas sobre a simulçao (Número de eventos, tempo de warm up e tempo de simulção)
    printf("\n\nNúmero de eventos agendados: %d\n",countEventsScheduled);
    printf("\n\nN = %d\nTempo para Warm Up = %d.0\nTempo de simulação = %5.1lf\n\n\n",N , warmUpTime, simulationTime);
    tbcast eventsBcast[countBcast];
    int x;
    for(i=2, x=0; tokens[i]!=NULL; i+=3) {
        eventOcc = tokens[i][0] == 'F' ? FAULT : (eventOcc = tokens[i][0] == 'R' ? REPAIR : BROADCAST);
        if(eventOcc==BROADCAST){
            eventsBcast[x].when = strtod(tokens[i+1],NULL);
            eventsBcast[x].node = atoi(tokens[i+2]);
            x++;
        }      
    }

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
            // roundTest++;
            roundTest = (int)floor(time()/TEST_INTERVAL);
            if(time() < timeNow+TEST_INTERVAL){
                shouldPrintRoundTest = FALSE;
            }
            else{
                shouldPrintRoundTest = TRUE;                
                timeNow = time();
            }
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
            indexBcast = isNewBroadcastEvent(eventsBcast, countBcast, time());
            if(indexBcast != -1){
                printf("\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
                printf("|  Início de uma nova transmissão de mensagem por braodcast |\n");
                printf("|\t\t\t\t\tNodo transmissor é o: %d                 |",eventsBcast[indexBcast].node);
                printf("\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
                defaultClusterBroadcast = 1;
                defaultNodeBroadcast = eventsBcast[indexBcast].node;
                //Como se entrou neste if é um broadcast novo, então pode ser configurado qual mensagem será transmitida,
                //neste caso, o tempo em que a mensagem começou a ser enviada
                mensagem.message = (int)time();
            }
            messageHandler(table_cis, token, time(), mensagem.message);
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