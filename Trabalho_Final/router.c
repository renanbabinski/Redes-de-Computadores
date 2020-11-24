//Universidade Federal da Fronteira Sul - UFFS
//Estudante: Renan Luiz Babinski
//Disciplina: Redes de Computadores

#include <stdio.h>
#include <stdlib.h>
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

    if (argc != 2)
    {
         printf("\n Usage: %s <router number> \n", argv[0]);
        return 1;
    }
    

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
        printf("Você Escolheu: %d", menu);
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