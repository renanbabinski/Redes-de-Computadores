//Universidade Federal da Fronteira Sul - UFFS
//Estudante: Renan Luiz Babinski
//Disciplina: Redes de Computadores

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#define CONTROLE 'c'
#define DADOS 'd'
#define EXIT 0
#define INFINITE 1000
#define DESTINO 0
#define CUSTO 1
#define NEXT_HOP 2
#define BUFLEN 512  //Max length of buffer
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
        int numero;
        int porta;
        char ip[15];
    }router_config;

int init_flag = 1;

FILE *configs, *enlaces;

void* udp_server(void* arg){
    router_config *router_svr;
    router_svr = (router_config*)arg;

    struct sockaddr_in si_me, si_other;

    int s, i, slen =  sizeof(si_other) , recv_len;
    char buf[BUFLEN];

    //criando um socket UDP
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        //die ("socket");
    }

    //zerando a estrutura
    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET; //IPV4
    si_me.sin_port = htons(router_svr->porta);  // PORTA
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    //amarrando o socket a uma porta
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) 
    {
        //die("bind");
    }

    //permaneça escutando
    while(1)
    {
        printf("Waiting for data...");
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
            printf("RECEBIDO PACOTE DE CONTROLE DE %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        }else if(buf[0] == DADOS){
            printf("RECEBIDO PACOTE DE DADOS DE %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
            printf("Data: %s\n" , &buf[2]);
        }
         
        //now reply the client with the same data
        if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            //die("sendto()");
        }
    }
    close(s);

}





int geth(){
	char s;
	scanf("%c",&s);
	return 0;
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
    printf("0) SAIR DO PROGRAMA\n");
    printf("\n\n");
    printf("Incoming Messages...\n");
    printf("\n");
    scanf("%d", &menu);
    geth();
    return menu;
}

void get_router_config(int* numero, int* porta, char* ip ){   //pega a linha de configuração do roteador correspondente
    char linha[50];
    char* substring;

    if((configs = fopen("roteador.config", "r")) == NULL){
        printf("Problemas na abertura do arquivo de configuração do roteador!\n");
        return;
    }

    while(!feof(configs)){                      
            fgets(linha, 50, configs);  
            if(linha[0] == ('0' + *numero))
                break;
        }
    fseek(configs, 0, SEEK_SET);
    



    substring = strtok(linha, " ");
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

////////////////////////////  MAIN  ///////////////////////////////////////////

int main(int argc, char *argv[]){

    int menu, count=0, selection;
    char linha [50];
    char* substring;

    //Configurações do router
    int numero_r;
    int porta_r;
    char ip_r[15];

    //Configurações do Roteador de destino

    int numero_dest_r;
    int porta_dest_r;
    char ip_dest_r[15];

    //threads

    pthread_t udp_srv;
    pthread_t d_v;
    
    

    if (argc != 2)
    {
         printf("\n Usage: %s <router number> \n", argv[0]);
        return 1;
    }

    
    if((enlaces = fopen("enlaces.config", "r")) == NULL){
        printf("Problemas na abertura do arquivo de enlaces!\n");
        return 1;
    }

    numero_r = *argv[1]- 48;
    get_router_config(&numero_r, &porta_r, ip_r);
    router_config config, *router_c;
    router_c = &config;
    router_c->numero = numero_r;
    router_c->porta = porta_r;
    strcpy(router_c->ip, ip_r);

    if((configs = fopen("roteador.config", "r")) == NULL){
        printf("Problemas na abertura do arquivo de configuração do roteador!\n");
        return 1;
    }

    while(!feof(configs)){                     //Conta a quantidade de destinos possiveis
        fgets(linha, 50, configs);
        count++;
    }
    fseek(configs, 0, SEEK_SET);
    fclose(configs);
    
    
    int links[count][count];  //Matriz de enlaces bidirecionais preenchida com -1
    int j,k,i;
    for(j=0; j<count; j++){
        for(k=0; k<count; k++){
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

    int routing_table[count][3];   // tabela de roteamento    Destino, Custo, Next_hop

    for(k=0; k<count; k++){
        if(numero_r == k+1){                    //Se o destino for o próprio roteador
            routing_table[k][DESTINO] = k+1;
            routing_table[k][CUSTO] = 0;
            routing_table[k][NEXT_HOP] = 0;
        }
        else if(links[numero_r-1][k] != -1){        //Se o destino for um vizinho
            routing_table[k][DESTINO] = k+1;
            routing_table[k][CUSTO] = links[numero_r-1][k];
            routing_table[k][NEXT_HOP] = 0;
        }else{                                      //Se o destino não é um vizinho
            routing_table[k][DESTINO] = k+1;
            routing_table[k][CUSTO] = INFINITE;
            routing_table[k][NEXT_HOP] = INFINITE;
        }
    }    


pthread_create(&udp_srv, NULL, udp_server, router_c);

//////////////// MENU DE PROGRAMA ///////////////////

while ((menu = context_menu(numero_r)) != EXIT){
    switch (menu)
    {
    case 1:
        printf("Mandar mensagem\n");

        printf("Informe para qual dos vizinhos abaixo você quer mandar a mensagem:\n\n");

        for(k=0; k<count; k++){
            if(links[numero_r-1][k] != -1){
                printf("Vizinho numero: %d\nCusto: %d\n\n", k+1, links[numero_r-1][k]);
            }
        }
        while(1){
            scanf("%d", &selection);
            if(links[numero_r-1][selection-1] != -1){
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
        message[0] = 'd';  //Define o tipo de mensagem

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
            //gets(message);
            scanf("%s",&message[2]);
            
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
            
            puts(buf);
        //}
        close(s);


        geth();
        break;
    case 2: 
        printf("Ver Vizinhos\n\n");
        for(k=0; k<count; k++){
            if(links[numero_r-1][k] != -1){
                printf("Vizinho numero: %d\nCusto: %d\n\n", k+1, links[numero_r-1][k]);
            }
        }
        geth();
        break;
    case 3:
        printf("Ver as configurações do router\n\n");

        printf("Numero do Roteador: %d\n", numero_r);
        printf("Numero da porta: %d\n", porta_r);
        printf("IP: %s\n", ip_r);
        geth();
        break;
    case 4:
        printf("Ver Matriz de Enlaces\n\n");

        for(j=0; j<count; j++){
            for(k=0; k<count; k++){
                printf("%d   ", links[j][k]);
            }
            printf("\n\n");
        }
        geth();
        
    case 5:
        printf("Ver tabela de roteamento:\n\n");

        for(i=0;i<count;i++){
            for(k=0;k<3;k++){
                printf("%d      ", routing_table[i][k]);
            }
            printf("\n");
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