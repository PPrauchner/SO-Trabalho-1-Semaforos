Trabalho 1 - Threads + Semáforos

Vivemos em um ambiente multiprocessado. Isto é, o sistema operacional executada múltiplas aplicações "ao mesmo tempo". Embora isso seja uma grande vantagem em termos de eficiência na utilização dos recursos computacionais, isso pode trazer também resultados numéricos inconsistentes em virtude do compartilhamento de memória, como por exemplo, através do uso de várias threads, somado ao fato de que temos a preempção, isto é, um processo pode deixar de executar a qualquer momento, quando o sistema operacional decide o "congelá-lo".

Diante disso, Implemente um programa que utilize threads e faça uso de semáforos.

A noção de semáforos foi proposta por Dijkstra há muito tempo atrás.

Neste trabalho é solicitado a implementação de um algoritmo que faça uso de semáforos. 

A linguagem e o tipo de problema ficam de livre escolha. Java, por exemplo, possui uma classe que garante mecanismos de exclusão mutua e semáforos. Utilize as classes runnable | thread para ativar a concorrência.

Você deve provar que o código ao ser executado não garante exclusão mútua. E que após a utilização de recursos como semáforos isso é garantido. 

Para garantir exclusão mútua não é necessário utilizar semáforos. No entanto, procure problemas cuja viabilidade de corretude ocorra através de semáforos. 

Sugestões de problemas: vários produtores e consumidores, jantar dos filósofos, Pix (entrada e saída de saldo), Restaurante Universitário com múltiplas filas (pessoas entram, pessoas vão se servir), controle de estoque, ...

Se possível, meça também o tempo de execução em cada condição. 

Forneça o código e um relatório contendo uma descrição da aplicação, qual teste implementado e de como foi a execução do experimento.