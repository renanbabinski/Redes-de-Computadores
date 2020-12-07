//Universidade Federal da Fronteira Sul - UFFS
//Estudante: Renan Luiz Babinski
//Disciplina: Redes de Computadores

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define EXIT 0

FILE *configs, *enlaces;

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
    printf("0) SAIR DO PROGRAMA\n");
    printf("\n\n");
    printf("Sua Opção:");
    scanf("%d", &menu);
    geth();
    return menu;
}

void get_router_config(int* numero, int* porta, char* ip){    //pega a linha de configuração do roteador correspondente
    char linha[50];
    char* substring;

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
}

////////////////////////////  MAIN  ///////////////////////////////////////////

int main(int argc, char *argv[]){

    int menu, count=0;
    char linha [50];
    char* substring;

    //Configurações do router
    int numero_r;
    int porta_r;
    char ip_r[15];

    if (argc != 2)
    {
         printf("\n Usage: %s <router number> \n", argv[0]);
        return 1;
    }

    if((configs = fopen("roteador.config", "rt")) == NULL){
        printf("Problemas na abertura do arquivo de configuração do roteador!\n");
        return 1;
    }
    if((enlaces = fopen("enlaces.config", "rt")) == NULL){
        printf("Problemas na abertura do arquivo de enlaces!\n");
        return 1;
    }

    numero_r = *argv[1]- 48;
    get_router_config(&numero_r, &porta_r, ip_r);

    while(!feof(configs)){                     
        fgets(linha, 50, configs);
        count++;
    }
    fseek(configs, 0, SEEK_SET);
    
    int links[count][count];  //Matriz de enlaces bidirecionais
    int j,k;
    for(j=0; j<count; j++){
        for(k=0; k<count; k++){
            links[j][k] = -1;
        }
    }
    
    
    while(!feof(enlaces)){                  
        fgets(linha, 50, enlaces);  
        
        substring = strtok(linha, " ");
        for (int i = 0; i < 3; i++)
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


//////////////// MENU DE PROGRAMA ///////////////////

while ((menu = context_menu(numero_r)) != EXIT){

    switch (menu)
    {
    case 1:
        printf("Você Escolheu: %d\n", menu);
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
        
    default:
        printf("Opção Inválida\n");
        geth();
        break;
    }




}


return 0;
}