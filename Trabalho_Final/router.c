//Universidade Federal da Fronteira Sul - UFFS
//Estudante: Renan Luiz Babinski
//Disciplina: Redes de Computadores
//Professor: MARCO AURÉLIO SPOHN
//Trabalho: SIMULAÇÃO DO PROTOCOLO DE ROTEAMENTO RIP COM ALGORITMO DE BELLMAN-FORD DISTRIBUÍDO


// LIBRARY
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

//  MACROS

#define NUMBER_OF_ROUTERS 6     //NÚMERO DE ROTEADORES DA TOPOLOGIA
#define CONTROLE 'c'            //TIPO DE MENSAGEM (CABEÇALHO)
#define DADOS 'd'               //TIPO DE MENSAGEM (CABEÇALHO)
#define EXIT 0                  //MENU EXIT
#define INFINITE 100            //DEFINE QUE VALOR SERÁ CONDIDERADO COMO INFINITO
#define DESTINO 0               //ACESSO TABELA DE ROTEAMENTO
#define CUSTO 1                 //ACESSO TABELA DE ROTEAMENTO
#define NEXT_HOP 2              //ACESSO TABELA DE ROTEAMENTO
#define BUFLEN 512              //Max length of buffer
#define BEACON 10               //Periodo em que o vetor distância será enviado (SEGUNDOS)

//  DEBUG   ->>>   1 PARA _ON_      0 PARA _OFF_

#define THREAD_UDP_SERVER 1     //INFORMAÇÕES PARA DEBUG DA THREAD QUE RECEBE OS PACOTES DE DADOS "d" e CONTROLE "c" 
#define THREAD_DV_UPDATE  1     //INFORMAÇÕES PARA DEBUG DA THREAD QUE ATUALIZA OS VETORES DISTÂNCIA, IMPRIME OS VETORES RECEBIDOS DOS VIZINHOS
#define ENVIO_BEACON      1     //ATIVA OU DESATIVA O ENVIO DO VETOR DISTÂNCIA PERIÓDICO
#define THREAD_TIMER      1     //ATIVA OU DESATIVA O TIMER(THREAD) DO ROTEADOR (SE O TIMER ESTIVER OFF O BEACON NÃO SERÁ ENVIADO!)
#define MSG_ROTEAMENTO    1     //EXIBE NA TELA OS PACOTES QUE ESTÃO SENDO ROTEADOS
#define THREAD_DV_SENDER  1     //MOSTRA PARA ONDE OS VETORES DISTÂNCIA ESTÃO SENDO ENCAMINHADOS

// FLAGS

int resend_dv = 0;              //indica quando a thread dv_sender deverá enviar o vetor distância

// TIMER

int time_s = 0;                 //VÁRIAVEL DE TEMPO GLOBAL

// MUTEX PARA ATUALIZAÇÃO DA TABELA DE ROTEAMENTO
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int routing_table[NUMBER_OF_ROUTERS][3];   // tabela de roteamento  ->>  Destino, Custo, Next_hop

struct router_config{           //STRUCT INFORMAÇÕES DO ROTEADOR
        int numero;
        int porta;
        char ip[15];
    }config;

int links[NUMBER_OF_ROUTERS][NUMBER_OF_ROUTERS];  //Matriz de enlaces bidirecionais preenchida com -1 na inicialização, SOMENTE A LINHA DO ROTEADOR CORRESPONDENTE É ACESSADA

char *received_dv[NUMBER_OF_ROUTERS];             //VETOR DE PONTEIROS PARA OS VETORES DISTÂNCIA RECEBIDOS DOS VIZINHOS

FILE *configs, *enlaces;                          //ARQUIVOS DE CONFIGURAÇÃO DA REDE --> LINKS, CUSTOS, E DESTINOS(IP,PORTA)


void get_router_config(int* numero, int* porta, char* ip ){   //pega a linha de configuração do roteador correspondente
    char linha[50];
    char* substring;

    if((configs = fopen("roteador.config", "r")) == NULL){
        printf("Problemas na abertura do arquivo de configuração do roteador!\n");
        return;
    }

    while(!feof(configs)){                         //le a linha de configuração do roteador correspondente                  
            fgets(linha, 50, configs);  
            if(linha[0] == ('0' + *numero))
                break;
        }
    fseek(configs, 0, SEEK_SET);
    

    substring = strtok(linha, " ");                // quebra a linha de configurações em tokens e atribui a variaveis
    for (int i = 0; i < 3; i++)
    {
        if (i == 1)
            *porta = atoi(substring);
        else if (i == 2)
            strcpy(ip, substring);

        substring = strtok(NULL, " ");
    }
    fclose(configs);
}

int geth(){                                        //PRESSIONE PARA CONTINUAR (PAUSE)
	char s;
	scanf("%c",&s);
	return 0;
}


//#############           THREADS       #######################  

// THREAD  ->> SEND DATA
void* send_data(void* arg){
    
    char *buf;
    buf = (char*) arg;
    char *temp;
    char buf_cache[BUFLEN];

    int destino, porta, origem;
    char ip[15];
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);

    strcpy(buf_cache, buf);
    
    temp = strtok(&buf_cache[1], ",");
    
    origem = atoi(temp);
    
    temp = strtok(NULL, ",");
    
    destino = atoi(temp);
    
    temp = strtok(NULL, ",");
   
   
    // SE O NEXT_HOP FOR DIFERENTE DE 0 QUER DIZER QUE O PACOTE AINDA NÃO CHEGOU NO DESTINO CORRETO
    if(MSG_ROTEAMENTO == 1){
        printf("\nNEXT HOP:  %d   CUSTO: %d\n",routing_table[destino][NEXT_HOP], routing_table[destino-1][CUSTO]);
    }
    if(routing_table[destino-1][NEXT_HOP] != 0){
        if(MSG_ROTEAMENTO == 1){
            printf("\nPACOTE DE DADOS DE ORIGEM %d E DESTINO %d SENDO ROTEADO...\n", origem, destino);
        }
        destino = routing_table[destino-1][NEXT_HOP];

        get_router_config(&destino, &porta, ip);   //Pega as informações do roteador de destino

        s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  

        memset((char *) &si_other, 0, sizeof(si_other));
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(porta);

        inet_aton(ip , &si_other.sin_addr);

        sendto(s, buf, strlen(buf) , 0 , (struct sockaddr *) &si_other, slen);

        memset(buf,'\0', BUFLEN);
    
        //recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen);

        //puts(buf);
    
        close(s);
    }else{
        if(MSG_ROTEAMENTO == 1){
            printf("\nPACOTE DE DADOS CHEGOU AO DESTINO!\n");
        }
            printf("MENSAGEM: %s", temp);
        
    }
}



//THREAD  ->> LINK LOSS
void* link_loss(void* arg){
    char* temp;
    char stringtok[BUFLEN];
    int origem, time_stamp;
    
    while(1){
        sleep(1);
        for(int i=0; i<NUMBER_OF_ROUTERS; i++){
            if(received_dv[i] != NULL){
                strcpy(stringtok, received_dv[i]);
                temp = strtok(stringtok,",");
                temp = strtok(NULL, ",");
                origem = atoi(temp);
                temp = strtok(NULL, ",");
                time_stamp = atoi(temp);
                printf("\nLINK LOSS ITERATION!");
                if((time_s - time_stamp) > (3*BEACON)){
                    printf("\n########       LINK LOSS DETECTED %d tempo atual: %d  tempo armazenado: %d     ##########",origem, time_s, time_stamp);
                    pthread_mutex_lock(&lock);
                    for(int j=0;j<NUMBER_OF_ROUTERS;j++){
                        if(routing_table[j][NEXT_HOP] == origem){
                            routing_table[j][CUSTO] = INFINITE;
                            routing_table[j][NEXT_HOP] = INFINITE;
                        }
                    }
                    //routing_table[origem-1][CUSTO] = INFINITE;
                    //routing_table[origem-1][NEXT_HOP] = INFINITE;
                    free(received_dv[i]);
                    received_dv[i] = NULL;
                    resend_dv = 1;
                    pthread_mutex_unlock(&lock);
                }
            }
        }

    }

}

// THREAD ->>>  TIMER
void* timer(void* arg){
    while(1){
        sleep(1);
        time_s++;
        // Seta flag para reenviar o vetor distância após tempo definido no macro BEACON
        if((time_s % BEACON) == 0){
            if(ENVIO_BEACON == 1)
                resend_dv = 1;
        }
    }
}

//THREAD  ->>>  DISTANCE VECTOR UPDATE

void* distance_vector_update(void* arg){
    char *temp;
    char *buf;
    buf = (char*) arg;
    int origem;
    int custo_enlace;
    int temp_destino, temp_custo;
    int custo_alternativo;
    char buf_received[BUFLEN];


    strcpy(buf_received, buf);
    temp = strtok(&buf[1], ",");
    if(THREAD_DV_UPDATE == 1){
        printf("\nORIGEM: %s\n", temp);
    }
    origem = atoi(temp);
    custo_enlace = links[config.numero-1][origem-1];
    received_dv[origem-1] = malloc(BUFLEN*sizeof(char));
    strcpy(received_dv[origem-1], buf_received);
    if(THREAD_DV_UPDATE == 1){
        printf("\nCUSTO DO ENLACE = %d\n", custo_enlace);
    }
    temp = strtok(NULL, ",");
    if(THREAD_DV_UPDATE == 1){
        printf("TIMESTAMP: %s\n\n", temp);
    }
    temp = strtok(NULL, ",");

    for(int j=0; j<NUMBER_OF_ROUTERS;j++){
        if(THREAD_DV_UPDATE == 1){
            printf("DESTINO: %s          ", temp);
        }
        temp_destino = atoi(temp);
        temp = strtok(NULL, ",");
        if(THREAD_DV_UPDATE == 1){
            printf("CUSTO: %s\n", temp);
        }
        temp_custo = atoi(temp);
        temp = strtok(NULL, ",");
        custo_alternativo = temp_custo + custo_enlace;
        if(custo_alternativo < routing_table[temp_destino-1][CUSTO]){
            pthread_mutex_trylock(&lock);
            routing_table[temp_destino-1][CUSTO] = custo_alternativo;
            routing_table[temp_destino-1][NEXT_HOP] = origem;
            resend_dv = 1;
        }
    }

    pthread_mutex_unlock(&lock);

    free(buf);
}



// THREAD ->>>  DISTANCE VECTOR SENDER
void* distance_vector_sender(void* arg){
    int destino, porta;
    char ip[15];
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    char cache[10]={0};
    


    while(1){
        sleep(1);
        if(resend_dv == 1){
            if(THREAD_DV_SENDER == 1){
                printf("ENCAMINHANDO MENSAGEM DE CONTROLE PERIÓDICA PARA: ");
            }
            pthread_mutex_lock(&lock);
            for(int k=0; k<NUMBER_OF_ROUTERS; k++){
                if(links[config.numero-1][k] != -1){
                    if(THREAD_DV_SENDER == 1){
                        printf("%d ", k+1);
                    }
                    destino = k+1;
                    //printf("\n\n\n RESEND CHAMADA! \n\n");

                    get_router_config(&destino, &porta, ip);   //Pega as informações do roteador de destino
                
                    //printf("DESTINO %d, PORTA %d, IP %s\n\n",destino, porta, ip);

                    
                    
                    s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  

                    memset((char *) &si_other, 0, sizeof(si_other));
                    si_other.sin_family = AF_INET;
                    si_other.sin_port = htons(porta);

                    inet_aton(ip , &si_other.sin_addr);

                    memset(message, '\0', BUFLEN);
                    message[0] = CONTROLE;  //Define o tipo de mensagem

                    //strcpy(&message[1], "teste mensagem de controle");
                    strcat(message, ",");
                    sprintf(cache, "%d", config.numero);
                    strcat(message, cache);
                    strcat(message, ",");
                    sprintf(cache, "%d", time_s);
                    strcat(message, cache);
                    strcat(message, ",");
                    for(int j=0; j<NUMBER_OF_ROUTERS; j++){
                        sprintf(cache, "%d", routing_table[j][DESTINO]);
                        strcat(message, cache);
                        strcat(message, ",");
                        sprintf(cache, "%d", routing_table[j][CUSTO]);
                        strcat(message, cache);
                        strcat(message, ",");
                    }


                    sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen);

                    pthread_mutex_unlock(&lock);

                    memset(buf,'\0', BUFLEN);
                
                    //recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen);

                    //puts(buf);
                
                    close(s);
                }
            }    
        }
        resend_dv = 0;
    }
}


// THREAD ->>>  UDP RECEIVED MESSAGES
void* udp_server(void* arg){
    struct sockaddr_in si_me, si_other;

    int s, i, slen =  sizeof(si_other) , recv_len;
    char buf[BUFLEN];

    pthread_t dv_update;
    pthread_t msg_send;

    //criando um socket UDP
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        //die ("socket");
    }

    //zerando a estrutura
    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET; //IPV4
    si_me.sin_port = htons(config.porta);  // PORTA
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    //amarrando o socket a uma porta
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) 
    {
        //die("bind");
    }

    //permaneça escutando
    while(1)
    {
        if(THREAD_UDP_SERVER == 1)
            printf("Waiting for data...\n");
        fflush(stdout);
        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        memset(buf,'\0', BUFLEN);

        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
            //die("recvfrom()");
        }
         
        //print details of the client/peer and the data received
        if(buf[0] == CONTROLE){
            char *dv;
            dv = malloc(BUFLEN * sizeof(char));
            if(dv == NULL){
                printf("\nERRO NA ALOCAÇÃO DE MEMÓRIA!\n");
                exit(EXIT_FAILURE);
            }
            if(THREAD_UDP_SERVER == 1){
                printf("\nRECEBIDO PACOTE DE CONTROLE DE %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                printf("Data: %s\n" , &buf[1]);
                printf("\nVETOR DISTÂNCIA RECEBIDO!\n");
            }
            strcpy(dv, buf);
            pthread_create(&dv_update, NULL, distance_vector_update, dv);
        }else if(buf[0] == DADOS){
            if(THREAD_UDP_SERVER == 1){
                printf("\nRECEBIDO PACOTE DE DADOS DE %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                printf("Data: %s\n" , buf);
            }
            char *msg;
            msg = malloc(BUFLEN * sizeof(char));
            if(msg == NULL){
                printf("\nERRO NA ALOCAÇÃO DE MEMÓRIA!\n");
                exit(EXIT_FAILURE);
            }
            strcpy(msg, buf);
            pthread_create(&msg_send, NULL, send_data, msg);
        }
         
        //now reply the client with the same data
        if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            //die("sendto()");
        }
    }
    close(s);

}


int context_menu(int router){
    int menu;
    system("clear");
    printf("Eu sou o router numero: %d \n\n", router);
    printf("1) MANDAR MENSAGEM \n");
    printf("2) VER VIZINHOS \n");
    printf("3) VER CONFIGURAÇÕES DESSE ROUTER\n");
    printf("4) VER MATRIZ DE ENLACES\n");
    printf("5) VER TABELA DE ROTEAMENTO\n");
    printf("6) FORÇAR ENVIO VETOR DISTÂNCIA\n");
    printf("7) VER VETORES DISTANCIA RECEBIDOS\n");
    printf("0) SAIR DO PROGRAMA\n");
    printf("\n\n");
    printf("Incoming Messages...\n");
    printf("\n");
    scanf("%d", &menu);
    geth();
    setbuf(stdin, NULL);
    return menu;
}


////////////////////////////  MAIN  ///////////////////////////////////////////

int main(int argc, char *argv[]){

    int menu, selection;
    char linha [50];
    char* substring;

    //Configurações do Roteador de destino

    int numero_dest_r;
    int porta_dest_r;
    char ip_dest_r[15];

    //threads

    pthread_t udp_srv;
    pthread_t d_v;
    pthread_t t_timer;
    pthread_t link_l;
    
    

    if (argc != 2)
    {
         printf("\n Usage: %s <router number> \n", argv[0]);
        return 1;
    }

    
    if((enlaces = fopen("enlaces.config", "r")) == NULL){
        printf("Problemas na abertura do arquivo de enlaces!\n");
        return 1;
    }

    config.numero = *argv[1]- 48;                                //le a configuração corespondente do numero do router passado por parametro
    get_router_config(&config.numero, &config.porta, config.ip);


    if((configs = fopen("roteador.config", "r")) == NULL){
        printf("Problemas na abertura do arquivo de configuração do roteador!\n");
        return 1;
    }
    
    
    int j,k,i;

    //zero_fill_dv();

    for(j=0; j<NUMBER_OF_ROUTERS; j++){
        for(k=0; k<NUMBER_OF_ROUTERS; k++){
            links[j][k] = -1;
        }
    }
    
    
    while(!feof(enlaces)){                  // Matriz de enlaces bidirecionais atualizada com custo
        fgets(linha, 50, enlaces);  
        
        substring = strtok(linha, " ");
        for (i = 0; i < 3; i++)
        {
            if(i == 0)
                j = atoi(substring);
            else if (i == 1)
                k = atoi(substring);
            else if (i == 2){
                links[j-1][k-1] = atoi(substring);
                links[k-1][j-1] = atoi(substring);
            }
            substring = strtok(NULL, " ");
        }
    }
    fseek(enlaces, 0, SEEK_SET);
    fclose(enlaces);

    
    for(k=0; k<NUMBER_OF_ROUTERS; k++){
        if(config.numero == k+1){                    //Se o destino for o próprio roteador
            routing_table[k][DESTINO] = k+1;
            routing_table[k][CUSTO] = 0;
            routing_table[k][NEXT_HOP] = 0;
        }
        else if(links[config.numero-1][k] != -1){        //Se o destino for um vizinho
            routing_table[k][DESTINO] = k+1;
            routing_table[k][CUSTO] = links[config.numero-1][k];
            routing_table[k][NEXT_HOP] = k+1;
        }else{                                      //Se o destino não é um vizinho
            routing_table[k][DESTINO] = k+1;
            routing_table[k][CUSTO] = INFINITE;
            routing_table[k][NEXT_HOP] = INFINITE;
        }
    }    

if(THREAD_TIMER == 1){
    pthread_create(&t_timer, NULL, timer, NULL);
}
pthread_create(&udp_srv, NULL, udp_server, NULL);
pthread_create(&d_v, NULL, distance_vector_sender, NULL);
//pthread_create(&link_l, NULL, link_loss, NULL);

//////////////// MENU DE PROGRAMA ///////////////////

while ((menu = context_menu(config.numero)) != EXIT){
    switch (menu)
    {
    case 1:
        printf("Mandar mensagem\n");

        printf("Informe para qual dos roteadores abaixo você quer mandar a mensagem:\n\n");

        for(k=0; k<NUMBER_OF_ROUTERS; k++){
            if(k+1 != config.numero)
                printf("Roteador numero: %d\n", k+1);
        }
        while(1){
            scanf("%d", &selection);
            setbuf(stdin, NULL);
            if(selection <= NUMBER_OF_ROUTERS && selection > 0 && selection != config.numero){
                break;
            }else{
                printf("\nNão é possivel mandar mensagem para o mesmo roteador ou roteador inexistente!");
            }
        }

        numero_dest_r = routing_table[selection-1][NEXT_HOP];
        get_router_config(&numero_dest_r, &porta_dest_r, ip_dest_r);   //Pega as informações do roteador de destino

        struct sockaddr_in si_other;
        int s, i, slen=sizeof(si_other);
        char buf[BUFLEN];
        char cache[10]={0};
        char text[BUFLEN];
        char message[BUFLEN];
        memset(message, '\0', BUFLEN);
        message[0] = DADOS;  //Define o tipo de mensagem
        strcat(message, ",");
        sprintf(cache, "%d", config.numero);
        strcat(message, cache);
        strcat(message, ",");
        sprintf(cache, "%d", selection);
        strcat(message, cache);
        strcat(message, ",");

        if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) //(IPV4, DATAGRAMA, UDP)
        {
        //die("socket");
        }

        memset((char *) &si_other, 0, sizeof(si_other));
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(porta_dest_r);

         if (inet_aton(ip_dest_r , &si_other.sin_addr) == 0) //converte para o formato da rede
        {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
        }

        //while(1)
        //{
            printf("Insira a mensagem para enviar ao roteador numero: %d: ", selection);
            fgets(text, 400, stdin);
            
            strcat(message, text);

            //send the message
            if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
            {
                //die("sendto()");
            }
            
            //receive a reply and print it
            //clear the buffer by filling null, it might have previously received data
            memset(buf,'\0', BUFLEN);
            //try to receive some data, this is a blocking call
            if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
            {
                //die("recvfrom()");
            }
            
            //puts(buf);
        //}
        close(s);


        geth();
        break;
    case 2: 
        printf("Ver Vizinhos\n\n");
        for(k=0; k<NUMBER_OF_ROUTERS; k++){
            if(links[config.numero-1][k] != -1){
                printf("Vizinho numero: %d\nCusto: %d\n\n", k+1, links[config.numero-1][k]);
            }
        }
        geth();
        break;
    case 3:
        printf("Ver as configurações do router\n\n");

        printf("Numero do Roteador: %d\n", config.numero);
        printf("Numero da porta: %d\n", config.porta);
        printf("IP: %s\n", config.ip);
        geth();
        break;
    case 4:
        printf("Ver Matriz de Enlaces\n\n");

        for(j=0; j<NUMBER_OF_ROUTERS; j++){
            for(k=0; k<NUMBER_OF_ROUTERS; k++){
                printf("%d   ", links[j][k]);
            }
            printf("\n\n");
        }
        geth();
        
    case 5:
        printf("Ver tabela de roteamento:\n\n");
        printf("\nDESTINO    CUSTO    NEXT HOP\n\n");
        for(i=0;i<NUMBER_OF_ROUTERS;i++){
            for(k=0;k<3;k++){
                printf("%d          ", routing_table[i][k]);
            }
            printf("\n");
        }

        geth();

    case 6:
        printf("\nForçando envio do vetor distância...");
        resend_dv = 1;
        geth();
        break;

    case 7:
        printf("\nVETORES DISTÂNCIA RECEBIDOS:\n\n");
        for(i=0;i<NUMBER_OF_ROUTERS;i++){
            printf("\nDISTANCE VECTOR RECEIVED OF %d: %s",i+1, received_dv[i]);
        }
        geth();

    default:
        printf("Opção Inválida\n");
        geth();
        break;
    
    }

}

return 0;
}