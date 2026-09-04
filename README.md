# Produtor–Consumidor com Semáforos POSIX

> Trabalho 1 — **Sistemas Operacionais** (4º semestre) · Unipampa

Implementação em **C11** do problema **produtor–consumidor com buffer limitado**,
usando `pthreads` e semáforos POSIX, montada como um **experimento controlado**: o
programa demonstra numericamente que, sem exclusão mútua, o acesso concorrente ao
recurso compartilhado corrompe o resultado — e que, com um semáforo binário, ele
passa a ser sempre correto.

A evidência é um **checksum**: a soma dos itens produzidos comparada à soma dos itens
consumidos. Sob sincronização correta as duas somas são necessariamente iguais;
qualquer divergência é corrupção do buffer.

---

## Resultado em uma tabela

Bateria oficial (`-O0`), 20 execuções por condição, WSL2 Ubuntu / 6 núcleos:

| Condição | Semáforos ativos | Execuções | Divergentes | Tempo médio |
|:--|:--|--:|--:|--:|
| `full` | `empty`, `full`, `mutex` | 20 | **0** | 159,1 ms |
| `no-mutex` | `empty`, `full` | 20 | **20** | 96,4 ms |
| `none` | nenhum | 20 | **20** | 4,8 ms |

Com exclusão mútua, zero divergências. Sem ela — mas com a capacidade do buffer ainda
respeitada — divergência em **todas** as execuções. A correção custou cerca de
**1,65×** o tempo da versão sem exclusão mútua.

📄 **Relatório completo:** [`docs/relatorio.md`](docs/relatorio.md) ·
[versão em PDF](docs/relatorio.pdf)  
📋 **Enunciado original:** [`docs/Enunciado.md`](docs/Enunciado.md)

---

## Requisitos

- `gcc` com suporte a C11 e `-pthread` (testado com gcc 15.2)
- `make`
- `bash` (para a bateria de execuções)
- **Nenhuma dependência externa** — só a biblioteca padrão, `pthread.h` e `semaphore.h`

> ⚠️ O alvo é **POSIX**. O código não compila no Windows nativo; o ambiente de
> desenvolvimento e execução é o **WSL Ubuntu**. Nos comandos abaixo, a partir do
> Windows, prefixe com `wsl -d Ubuntu`.

## Como executar

```bash
make            # compila com -O0 (flag da medição oficial)
make check      # testes da API do buffer + bateria oficial
make clean

./bin/prodcons full        # com exclusão mútua
./bin/prodcons no-mutex    # sem exclusão mútua, capacidade respeitada
./bin/prodcons none        # sem sincronização nenhuma
```

Cada execução imprime uma linha de resultado:

```text
sync_mode=full produced_checksum=5000050000 consumed_checksum=5000050000 difference=0 time_ms=150.750
```

Alvos adicionais:

| Alvo | O que faz |
|:--|:--|
| `make test` | Só o seam determinístico: testes da API do buffer, sem threads. |
| `make battery` | Só a bateria oficial `-O0`: 20 execuções por modo. |
| `make opt` | Compila um segundo binário com `-O2`, em `bin/prodcons-o2`. |
| `make battery-opt` | Roda a bateria sobre o binário `-O2` (achado à parte do relatório). |

---

## As três condições experimentais

O modo de sincronização é o **único** parâmetro que vem da linha de comando —
`N_PRODUCERS`, `N_CONSUMERS`, `N_ITEMS` e `BUFFER_SIZE` são constantes no código, por
requisito de reprodutibilidade. Cada modo remove exatamente uma coisa:

| `sync_mode` | `empty` / `full` (capacidade) | `mutex` (exclusão mútua) | O que é removido |
|:--|:--:|:--:|:--|
| `full` | presente | presente | nada — é a referência |
| `no-mutex` | presente | **ausente** | só a exclusão mútua |
| `none` | **ausente** | **ausente** | capacidade **e** exclusão mútua |

**Por que três condições, e não duas.** Sob `none`, uma divergência de checksum tem
duas explicações concorrentes: os índices se corromperam, **ou** o buffer simplesmente
estourou porque nada fazia os produtores esperarem por espaço livre. `no-mutex` fecha
essa saída — os semáforos contadores continuam ativos, a capacidade é respeitada, e a
única ausência é o semáforo binário sobre os índices. **É `no-mutex` que isola a
exclusão mútua como variável única**, e é ela que sustenta a conclusão do trabalho.

O mecanismo da corrida é visível na aritmética: `write_index = (write_index + 1) %
BUFFER_SIZE` não é atômico — é leitura, cálculo e escrita. Dois produtores podem ler o
mesmo índice e escrever no mesmo slot.

---

## O teste

Dois **seams**, respondendo a perguntas diferentes:

- **`tests/test_buffer.c`** — exercita o contrato público do buffer em uma thread só,
  com chamadas que nunca bloqueiam. Existe para separar *defeito de estrutura de
  dados* de *defeito de concorrência*: se esses testes falham, a suspeita não é a
  sincronização.
- **`tests/battery.sh`** — executa o binário 20 vezes por modo, com **veredito
  assimétrico**: `full` reprova se *qualquer* execução divergir (é uma invariante);
  `no-mutex` e `none` reprovam se *nenhuma* divergir (uma corrida de dados não se
  testa com uma execução só).

---

## Estrutura

```
.
├── src/
│   ├── buffer.h / buffer.c   # buffer circular, índices e os três semáforos POSIX
│   └── main.c                # threads, cronometragem e linha de resultado
├── tests/
│   ├── test_buffer.c         # seam 1 — API do buffer, determinístico
│   └── battery.sh            # seam 2 — o binário, 20 execuções por modo
├── docs/
│   ├── Enunciado.md          # enunciado original (não editar)
│   ├── relatorio.md          # entregável, em pt-BR
│   ├── relatorio.pdf         # composto por docs/relatorio/build.sh
│   ├── relatorio/            # figuras, template LaTeX e pipeline pandoc/xelatex
│   └── adr/                  # decisões de arquitetura e seus porquês
├── CONTEXT.md                # glossário de domínio
└── Makefile
```

Toda a decisão sobre exclusão mútua mora em `buffer_put` e `buffer_take`, nas linhas
de `sem_wait`/`sem_post` sobre o semáforo `mutex`. A separação entre `buffer.c` e
`main.c` existe para que o relatório possa apontar o arquivo e a função exatos.

---

## Decisões de projeto

Registradas em [`docs/adr/`](docs/adr/) — em resumo:

- **C em vez de Java** ([ADR 0001](docs/adr/0001-c-pthreads-em-vez-de-java.md)) — em C,
  `sem_wait`/`sem_post` *são* o P e o V de Dijkstra, sem camada intermediária; e o
  aquecimento do JIT contaminaria a medição de tempo, que é um dos itens pedidos.
- **Semáforo binário em vez de `pthread_mutex_t`** — o enunciado pede semáforos, e usar
  a mesma primitiva para contar recursos e para excluir mutuamente torna explícito que
  *semáforo contador não é mutex*.
- **`-O0` como flag da medição oficial**, igual para as três condições — tempos só se
  comparam entre binários compilados com a mesma flag. `-O2` roda à parte, como achado
  documentado (a corrida **não** desaparece sob `-O2`).
- **Nunca `volatile` para forçar a corrida a aparecer** — `volatile` não dá
  atomicidade, apenas inibe otimização; usá-lo faria o experimento medir o compilador
  em vez do sistema operacional.

---

## Gerar o PDF do relatório

```bash
bash docs/relatorio/build.sh
```

Requer `pandoc`, `xelatex` (texlive-xetex), fontes TeX Gyre e `python3` com
`matplotlib` (para regerar as figuras).
