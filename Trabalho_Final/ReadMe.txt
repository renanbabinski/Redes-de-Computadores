Trabalho Rede de computadores

Para compilar use:

gcc router.c -o router -lpthread
./router

Apenas a parte 1 do trabalho foi implementada completamente, os vizinhos conseguem mandar mensagens de dados entre si.

Na parte 2 do trabalho só foi implementado a inicialização da tabela de roteamento e um cabeçalho nas mensagens para 
identificar se é uma mensagem de controle 'c' ou de dados 'd'.

Nenhuma atualização é feita na tabela de roteamento e nenhum pacote de "dados" é roteado para além de seus vizinhos.