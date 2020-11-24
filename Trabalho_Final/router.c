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

int context_menu(){
    int menu;
    system("clear");
    printf("Eu sou o router numero: \n\n");
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




int main(){

    int menu;

while ((menu = context_menu()) != EXIT){

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