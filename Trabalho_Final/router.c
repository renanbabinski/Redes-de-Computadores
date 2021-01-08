//Universidade Federal da Fronteira Sul - UFFS
//Estudante: Renan Luiz Babinski
//Disciplina: Redes de Computadores


// LIBRARY
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

//  MACROS
#define NUMBER_OF_ROUTERS 6
#define CONTROLE 'c'
#define DADOS 'd'
#define EXIT 0
#define INFINITE 100
#define DESTINO 0
#define CUSTO 1
#define NEXT_HOP 2
#define TIMESTAMP 3
#define BUFLEN 512  //Max length of buffer
#define BEACON 10  //Periodo em que o vetor distância será enviado

// FLAGS

int resend_dv = 0;    //indica quando a thread dv_sender deverá enviar o vetor distância

// TIMER

int time_s = 0;



//////////

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int routing_table[NUMBER_OF_ROUTERS][3];   // tabela de roteamento    Destino, Custo, Next_hop

struct router_config{
        int numero;
        int porta;
        char ip[15];
    }config;

int links[NUMBER_OF_ROUTERS][NUMBER_OF_ROUTERS];  //Matriz de enlaces bidirecionais preenchida com -1 na inicialização

FILE *configs, *enlaces;

char *  itoa ( int value, char * str )
{
    char temp;
    int i =0;
    while (value > 0) {
        int digito = value % 10;

        str[i] = digito + '0';
        value /= 10;
        i++;

    }
   i = 0;
   int j = strlen(str) - 1;

   while (i < j) {
      temp = str[i];
      str[i] = str[j];
      str[j] = temp;
      i++;
      j--;
   }
    return str;
}

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

int geth(){
	char s;
	scanf("%c",&s);
	return 0;
}



// THREAD ->>>  TIMER
void* timer(void* arg){
    while(1){
        sleep(1);
        time_s++;
        if((time_s % BEACON) == 0){
            //printf("\nBEACON SEND  -- time: %d",time_s);
            resend_dv = 1;
        }else{
            //printf("\ntime: %d",time_s);
        }
    }
}

// THREAD ->>>  DISTANCE VECTOR SENDER
void* distance_vector_sender(void* arg){
    int destino, porta;
    char ip[15];
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    message[0] = CONTROLE;  //Define o tipo de mensagem
    char test[20] = "teste msg controle";
    char cache[10]={0};
    


    while(1){
        sleep(1);
        if(resend_dv == 1){
            printf("ENCAMINHANDO MENSAGEM DE CONTROLE PERIÓDICA PARA: ");
            for(int k=0; k<NUMBER_OF_ROUTERS; k++){
                if(links[config.numero-1][k] != -1){
                    printf("%d ", k+1);
                    destino = k+1;
                    //printf("\n\n\n RESEND CHAMADA! \n\n");

                    get_router_config(&destino, &porta, ip);   //Pega as informações do roteador de destino
                
                    //printf("DESTINO %d, PORTA %d, IP %s\n\n",destino, porta, ip);

                    
                    
                    s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  

                    memset((char *) &si_other, 0, sizeof(si_other));
                    si_other.sin_family = AF_INET;
                    si_other.sin_port = htons(porta);

                    inet_aton(ip , &si_other.sin_addr);
                
                    //printf("Insira a mensagem para enviar ao roteador numero: %d: ", destino);
                    //gets(message);
                    //scanf("teste msg controle",&message[2]);
                    //setbuf(stdin, NULL);

                    //strcpy(&message[1], "teste mensagem de controle");
                    for(int j=0; j<NUMBER_OF_ROUTERS; j++){
                        sprintf(cache, "%d", routing_table[j][DESTINO]);
                        strcat(message, cache);
                        strcat(message, ",");
                        sprintf(cache, "%d", routing_table[j][CUSTO]);
                        strcat(message, cache);
                        strcat(message, ",");
                    }


                    sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen);

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
    char *temp;

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
            printf("\nRECEBIDO PACOTE DE CONTROLE DE %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
            printf("Data: %s\n" , &buf[1]);
            printf("\nVETOR DISTÂNCIA RECEBIDO!\n");
            temp = strtok(&buf[1], ",");
            for(int j=0; j<NUMBER_OF_ROUTERS;j++){
                printf("DESTINO: %s          ", temp);
                temp = strtok(NULL, ",");
                printf("CUSTO: %s\n", temp);
                temp = strtok(NULL, ",");
            }
        }else if(buf[0] == DADOS){
            printf("\nRECEBIDO PACOTE DE DADOS DE %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
            //setbuf(stdin,NULL);
            printf("Data: %s\n" , &buf[1]);
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

    //Configurações do router
    //int numero_r;
    //int porta_r;
    //char ip_r[15];

    //Configurações do Roteador de destino

    int numero_dest_r;
    int porta_dest_r;
    char ip_dest_r[15];

    //threads

    pthread_t udp_srv;
    pthread_t d_v;
    pthread_t t_timer;
    
    

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

    /*while(!feof(configs)){                     //Conta a quantidade de destinos possiveis
        fgets(linha, 50, configs);
        NUMBER_OF_ROUTERS++;
    }
    fseek(configs, 0, SEEK_SET);
    fclose(configs); */
    //^^^^^^  INSERIDO NUMERO DE DESTINOS MANUALMENTE NA INICIALIZAÇÃO
    
    
    
    int j,k,i;
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

//pthread_create(&t_timer, NULL, timer, NULL);
pthread_create(&udp_srv, NULL, udp_server, NULL);
pthread_create(&d_v, NULL, distance_vector_sender, NULL);

//////////////// MENU DE PROGRAMA ///////////////////

while ((menu = context_menu(config.numero)) != EXIT){
    switch (menu)
    {
    case 1:
        printf("Mandar mensagem\n");

        printf("Informe para qual dos vizinhos abaixo você quer mandar a mensagem:\n\n");

        for(k=0; k<NUMBER_OF_ROUTERS; k++){
            if(links[config.numero-1][k] != -1){
                printf("Vizinho numero: %d\nCusto: %d\n\n", k+1, links[config.numero-1][k]);
            }
        }
        while(1){
            scanf("%d", &selection);
            setbuf(stdin, NULL);
            if(links[config.numero-1][selection-1] != -1){
                break;
            }else{
                printf("O roteador não é um vizinho!");
            }
        }

        numero_dest_r = selection;
        get_router_config(&numero_dest_r, &porta_dest_r, ip_dest_r);   //Pega as informações do roteador de destino

        struct sockaddr_in si_other;
        int s, i, slen=sizeof(si_other);
        char buf[BUFLEN];
        char message[BUFLEN];
        message[0] = DADOS;  //Define o tipo de mensagem

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
            printf("Insira a mensagem para enviar ao roteador numero: %d: ", numero_dest_r);
            fgets(&message[1], 400, stdin);
            
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

    default:
        printf("Opção Inválida\n");
        geth();
        break;
    
    }




}


return 0;
}