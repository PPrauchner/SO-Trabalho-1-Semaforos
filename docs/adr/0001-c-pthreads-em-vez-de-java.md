# C com pthreads e semáforos POSIX, em vez de Java

O enunciado sugere explicitamente Java (`Runnable`/`Thread` e a classe `Semaphore`), e
Java está instalado nativamente na máquina — então escolher C é a decisão surpreendente
que precisa de registro. Escolhemos **C11 com `pthreads` e semáforos POSIX**, rodando no
WSL Ubuntu, porque neste trabalho as primitivas *são* o conteúdo: `sem_wait` e
`sem_post` são o P e o V de Dijkstra citados no enunciado, sem nenhuma camada entre o
que se escreve e o que se explica no relatório.

## Considered Options

- **Java.** Nativo no Windows, sem WSL, e literalmente sugerido pelo enunciado. Rejeitado
  por dois motivos: o `Semaphore` fica sobre o `AbstractQueuedSynchronizer`, uma
  abstração a mais a justificar no relatório; e o aquecimento do JIT contamina a medição
  de tempo, que é um dos itens pedidos — sem rodadas de warmup, mede-se compilação, não
  sincronização.
- **Python.** Rejeitado pelo **GIL**: o semáforo é real e a race acontece de fato (o GIL
  é liberado entre bytecodes), mas duas threads nunca executam bytecode simultaneamente.
  Mediríamos contenção de GIL e chamaríamos de custo de sincronização, num trabalho cujo
  enunciado abre falando em ambiente multiprocessado.
- **C com a API Win32** (`CreateThread`, `CreateSemaphore`), para rodar no Windows nativo.
  Rejeitado por acrescentar código específico de plataforma a explicar, sem ganho
  conceitual sobre POSIX.

## Consequences

- **O projeto não compila no Windows nativo.** Edita-se no Windows, compila-se e roda-se
  via `wsl`. Reverter para Java significaria reescrever o programa inteiro.
- A gerência de memória e do ciclo de vida das threads é manual — aceitável porque o
  buffer é circular e estático.
- O nível de otimização passa a ser uma variável do experimento: com `-O2` o gcc pode
  manter estado compartilhado em registrador e alterar a visibilidade da race. Por isso
  `-O0` é a medição oficial, e `-O2` vira achado documentado, não acidente.
