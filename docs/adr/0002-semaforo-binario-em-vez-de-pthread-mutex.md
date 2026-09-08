# Semáforo binário para a exclusão mútua, em vez de `pthread_mutex_t`

A exclusão mútua sobre os índices do buffer vem de um `sem_t` inicializado em 1, não de
um `pthread_mutex_t` — que é a primitiva óbvia para a tarefa e a que qualquer leitor
esperaria encontrar. A escolha é deliberada: o enunciado pede semáforos, e usar a
**mesma** primitiva para as duas funções diferentes é o que torna visível, dentro do
próprio código, que *semáforo contador não é mutex*.

O contraste é o conteúdo do trabalho. Os três semáforos do buffer são do mesmo tipo e
atendidos pelas mesmas chamadas (`sem_wait`/`sem_post`): `empty` e `full` contam
recursos (capacidade), `mutex` garante exclusão mútua. A diferença está no valor
inicial e na intenção, não na API. Com um `pthread_mutex_t` no lugar do terceiro, o
leitor atribuiria a correção ao *tipo* da primitiva; com um semáforo, sobra a
explicação certa — o que separa `NO_MUTEX` de `FULL` é um semáforo binário sobre a
seção crítica, e é isso que o modo `NO_MUTEX` remove.

## Considered Options

- **`pthread_mutex_t`.** A ferramenta idiomática, provavelmente mais rápida (fast path
  em espaço de usuário, sem a semântica de contagem que aqui não se usa) e mais clara
  em código de produção. Rejeitada porque o enunciado pede explicitamente semáforos e
  porque ela apagaria justamente a confusão que o experimento existe para desfazer: se
  a exclusão mútua vem de um tipo chamado "mutex", a lição de que um semáforo *contador*
  não a garante deixa de ser demonstrada e passa a ser apenas afirmada.
- **`pthread_cond_t` + mutex** para a capacidade, no lugar de `empty`/`full`. Rejeitada
  pelo mesmo motivo, e por ser a construção que os semáforos contadores dispensam.

## Consequences

- O código não usa `pthread_mutex_t` nem `pthread_cond_t` em lugar nenhum — só
  `sem_t`. Introduzir um deles reabre esta decisão.
- `buffer_init` inicializa apenas os semáforos que o modo ativo usa, e `buffer_destroy`
  destrói apenas esses. É o que permite a cada modo remover exatamente uma coisa.
- A ordem de aquisição é sempre contador antes do binário (`empty` → `mutex` no
  produtor, `full` → `mutex` no consumidor). A ordem inversa dá deadlock: uma thread
  dormiria no contador segurando a seção crítica.
