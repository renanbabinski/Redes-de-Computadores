//Universidade Federal da Fronteira Sul - UFFS
//Estudante: Renan Luiz Babinski
//Disciplina: Redes de Computadores

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define EXIT 0


int geth(){
	char s;
	scanf("%c",&s);
	return 0;
}

int context_menu(char* router){
    int menu;
    system("clear");
    printf("Eu sou o router numero: %s \n\n", router);
    printf("1) MANDAR MENSAGEM \n");
    printf("2) VER VIZINHOS \n");
    printf("3) VER CONFIGURAÇÕES DESSE ROUTER\n");
    printf("0) SAIR DO PROGRAMA\n");
    printf("\n\n");
    printf("Sua Opção:");
    scanf("%d", &menu);
    getchar();
    return menu;
}


int main(int argc, char *argv[]){

    int menu;
    FILE *configs, *enlaces;
    char linha [50], routern;
    char *result;

    if (argc != 2)
    {
         printf("\n Usage: %s <router number> \n", argv[0]);
        return 1;
    }

    if((configs = fopen("roteador.config", "rt")) == NULL){
        printf("Problemas na abertura do arquivo de configuração do roteador!\n");
        return 1;
    }

    while(!feof(configs)){                      //pega a linha de configuração do roteador correspondente
            fgets(linha, 50, configs);  
            if(linha[0] == *argv[1])
                break;
        }
        fseek(configs, 0, SEEK_SET);


    

while ((menu = context_menu(argv[1])) != EXIT){

    switch (menu)
    {
    case 1:
        printf("Você Escolheu: %d\n", menu);
        geth();
        break;
    case 2: 
        printf("Você Escolheu: %d", menu);
        geth();
        break;
    case 3:
        printf("Ver as configurações do router\n\n");

        printf("Numero do Roteador: %c", linha[0]);
        printf("Numero da porta: %s", linha[2]);
        printf("IP: %s", linha[8]);
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