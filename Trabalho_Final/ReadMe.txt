Trabalho Redes de computadores

Universidade Federal da Fronteira Sul - UFFS - Campus Chapecó-SC
Estudante: Renan Luiz Babinski
Disciplina: Redes de Computadores
Professor: MARCO AURÉLIO SPOHN
Trabalho: SIMULAÇÃO DO PROTOCOLO DE ROTEAMENTO RIP COM ALGORITMO DE BELLMAN-FORD DISTRIBUÍDO


1) Antes de tudo os arquivos enlaces.cfg e config.cfg deverão ser ajustados para a topologia a ser testada.

2) Dentro do programa router.c alguns MACROS precisam ser ajustados antes da compilação:

a) (OBRIGATÓRIO) a macro NUMBER_OF_ROUTERS deverá ser ajustada para o número exato de nodos da rede (roteadores)

b) (OPCIONAL) a macro INFINITE pode ser ajustada para definir um valor de parada mais alto ou mais baixo

c) (OPCIONAL) a macro BEACON define a cada quanto tempo os vetores distância serão enviados em SEGUNDOS

d) (OPCIONAL) as macros abaixo da LABEL  //  DEBUG servem para debugar o programa, podem ser ativadas setadas como 1 ou desativadas setadas como 0


Após configurações dos parâmetros acima
Para compilar use:

gcc router.c -o router -lpthread
./router <numero do roteador>

Após compilado o programa deverá ser executado como apresentado acima, informando como parametro o numero do roteador correspondente.
Exemplo:
./router 1

Cada roteador deve ser executado em uma janela separada (processo)


////////  Explicação funcionamento do programa: ///////

A definição de uso de cada váriavel esta comentada no código


1) cabeçalho das mensagem de CONTROLE e de DADOS:

    CADA CAMPO DO CABEÇALHO É SEPARADO POR UMA VIRGULA ",", INCLUINDO OS DESTINOS E CUSTOS DO VETOR DISTÂNCIA

    a) cabeçalho mensagem de controle(vetor distãncia):
        ,tipo("c" para controle), origem(roteador), time_stamp(tempo em que foi gerado), destino, custo, destino, custo....

        Exemplo: ,c,1,150,1,0,2,10,...,
    
    b) cabeçalho mensagem de dados(texto):
        ,tipo("d" para dados), origem(roteador), destino(roteador), texto de Exemplo...

        Exemplo: ,d,2,3,TESTE MENSAGEM DE DADOS

2) threads:

a) udp_server:
    Esta Thread é a mais importante para o funcinamento do programa. Ela é reponsável por receber os pacotes de dados "d" e de controle "c".
    Ela sempre está escutando em sua porta por mensagens vindas de seus vizinhos.
    Quando a thread recebe um pacote da rede ela irá tomar 1 de 2 decisões:

    1) Caso o pacote seja de controle "c" ela irá chamar a THREAD distance_vector_update e passará por parâmetro o pacote recebido. A Thread chamada faz o resto do trabalho e udp_server volta a escutar na sua porta

    2) Caso o pacote seja de dados "d" ela irá chamar a THREAD send_data e passará por parâmetro o pacote recebido. A Thread chamada faz o resto do trabalho e udp_server volta a escutar na sua porta.

b) send_data
    Esta Thread é reponsável por encaminhar os pacotes de dados(mensagens de texto) ao seus destinos.
    Ela identifica se o pacote recebido já está no destino(quando o NEXT_HOP do router for igual a 0), caso esteja no destino imprime o texto na tela, se não está no destino ela consultará a tabela de roteamento e enviará o pacote adiante de acordo com NEXT_HOP.

c) distance_vector_update
    Esta Thread é reponsável por receber os vetores distância, atualizar a tabela de roteamento (região crítica), e armazenar os mesmos para consulta se necessário(em caso de link_loss).
    Quando uma alteração é detectada a flag resend_dv é setada para 1 a fim de reenviar o vetor distância para seus vizinhos. Quem reenvia é a thread distance_vector_sender.

d) distance_vector_sender
    Esta Thread envia o vetor distância para os vizinhos.
    Está sempre verificando a váriavel global resend_dv a fim de determinar quando o vetor distância deve ser enviado para seus vizinhos.
    Está váriavel é modificada para 1 em duas situações:
    
    1) A cada x SEGUNDOS definida no macro BEACON, quem altera o valor é a Thread TIMER

    2) Quando o vetor distância é atualizado

e) link_loss
    Esta Thread está sempre verificando o TIMESTAMP dos vetores distância recebidos dos vizinhos. Caso o relógio global time_s e o time_stamp do vetor tenha uma diferença maior do que BEACON*3 será entendido que a conexão do enlace correspondente foi perdida e o custo e NEXT_HOP desse enlace e dos demais destinos com esse enlace será setado para INFINITE

f) timer
    Esta Thread incrementa a váriavel global time_s em 1 a cada 1 segundo, caso tenha se passado BEACON segundos, a flag resend_dv é setada para 1 para que o vetor distância periódico seja enviado.

g)main
    A Thread main inicializa o roteador de acordo com os arquivos de configuração e inicializa as threads que ficam sempre ativas.
    Após inicialização ela é responsável por manter a interface com o usuário e chamar as funções definidas no MENU.

