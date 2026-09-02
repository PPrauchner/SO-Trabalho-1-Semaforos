# Trabalho 1 — Produtor-Consumidor com Semáforos

Um experimento controlado sobre um buffer limitado compartilhado entre threads
produtoras e consumidoras. O contexto existe para demonstrar em que condições a
exclusão mútua é — e não é — garantida, medindo o custo de cada uma.

## Language

### Atores e dados

**Produtor** (`producer`):
Thread que gera itens e os deposita no buffer.
_Avoid_: escritor, gerador, emissor

**Consumidor** (`consumer`):
Thread que retira itens do buffer e os contabiliza.
_Avoid_: leitor, receptor

**Buffer** (`buffer`):
Estrutura circular de capacidade fixa que guarda os itens entre a produção e o
consumo. É o recurso compartilhado cuja corrupção o experimento mede.
_Avoid_: fila, pilha, canal

**Item** (`item`):
Valor depositado por um produtor e retirado por um consumidor. Existe para entrar no
checksum — não carrega significado próprio de domínio.
_Avoid_: mensagem, tarefa, job

**Slot** (`slot`):
Uma posição individual do buffer.
_Avoid_: célula, casa, índice

### Sincronização

**Semáforo contador** (`empty`, `full`):
Semáforo cujo valor representa uma quantidade de recursos disponíveis: `empty` conta
slots livres, `full` conta itens disponíveis. Controla **capacidade**, não exclusão
mútua.
_Avoid_: chamar de mutex, ou de "trava de capacidade"

**Mutex** (`mutex`):
Semáforo binário, inicializado em 1, que protege os índices do buffer. É o que garante
a **exclusão mútua** — e é exatamente o que o modo `NO_MUTEX` remove.
_Avoid_: lock, trava, monitor

### Experimento

**Modo de sincronização** (`sync_mode`, enum `SyncMode`):
Qual conjunto de semáforos está ativo numa execução — `NONE`, `NO_MUTEX` ou `FULL`. É
a única variável que muda entre execuções.
_Avoid_: `condition` (colide com `pthread_cond_t`), variant, scenario

**Condição experimental**:
Termo em pt-BR, usado **apenas no relatório**, para uma execução sob um dado modo de
sincronização. Não tem identificador correspondente no código.
_Avoid_: usar a palavra "condição" dentro do código

**Checksum** (`checksum`):
Soma de todos os itens produzidos, comparada à soma de todos os consumidos. A
divergência entre as duas é a evidência de que a exclusão mútua falhou.
_Avoid_: hash, total, contador
