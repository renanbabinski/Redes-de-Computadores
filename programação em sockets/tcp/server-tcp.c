#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; //estruturas para configuração de sockets

    char sendBuff[1025];
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);  //função para criação de sockets (IPV4, TCP, 0)
    memset(&serv_addr, '0', sizeof(serv_addr)); //alocação
    memset(sendBuff, '0', sizeof(sendBuff));  //alocação

    serv_addr.sin_family = AF_INET;   // IPV4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serv_addr.sin_port = htons(5000); //porta usada

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));  //amarração das configurações
    //bind (descritor do socket, struct socksddr, tamanho)


    listen(listenfd, 10); //coloca socket no modo escuta (descritor, conexões pendentes)


    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); // aceita uma conexão pendente

        ticks = time(NULL);
        snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        printf("\n (Server side) sending %s \n", sendBuff);
        write(connfd, sendBuff, strlen(sendBuff)); //escreve no socket como se fosse um arquivo

        close(connfd); //fecha conexão
        sleep(1);
     }
}
