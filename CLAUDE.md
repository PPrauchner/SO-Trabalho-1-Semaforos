# CLAUDE.md — Trabalho 1: Threads + Semáforos (SO)

Trabalho da disciplina de **Sistemas Operacionais** (4º semestre). Implementa o
problema **produtor-consumidor com buffer limitado** em C, para demonstrar
experimentalmente que sem sincronização a exclusão mútua não é garantida, e que com
semáforos POSIX ela é.

O enunciado original está em [`docs/Enunciado.md`](./docs/Enunciado.md) — é a fonte da
verdade sobre o que precisa ser entregue. O glossário de domínio está em
[`CONTEXT.md`](./CONTEXT.md); as decisões e seus porquês, em [`docs/adr/`](./docs/adr/).

---

## O que precisa ser entregue

1. **Código** — o programa em C.
2. **Relatório** (`docs/relatorio.md`) — descrição da aplicação, qual teste foi
   implementado e como foi a execução do experimento.
3. **Medição de tempo** em cada condição experimental.

---

## Stack

| Item | Escolha |
|---|---|
| Linguagem | C (C11) |
| Threads | POSIX `pthreads` |
| Sincronização | semáforos POSIX (`sem_t` — `sem_init`, `sem_wait`, `sem_post`) |
| Compilador | `gcc` 15.2 (glibc NPTL 2.43) |
| Build | `make` |
| Dependências externas | **nenhuma** |

> ⚠️ **O código não compila no Windows nativo** — não há `gcc`, `clang` nem `cl`
> instalados. Ambiente de execução é o **WSL Ubuntu** (6 núcleos). Fluxo: editar no
> Windows, compilar e rodar via `wsl`.

---

## Comandos

```bash
wsl make            # compila com -O0 (medição oficial)
wsl make opt        # compila com -O2 (achado extra do relatório)
wsl make clean

wsl ./bin/prodcons none      # sem sincronização nenhuma
wsl ./bin/prodcons no-mutex  # semáforos contadores, SEM exclusão mútua
wsl ./bin/prodcons full      # sincronização completa
```

---

## O experimento

Três **modos de sincronização**, comparados sob os mesmos parâmetros e a mesma flag de
compilação. O checksum (soma dos itens produzidos × soma dos consumidos) é a evidência:

| `sync_mode` | `empty`/`full` | `mutex` | Resultado esperado |
|---|---|---|---|
| `NONE` | ✗ | ✗ | buffer estoura/esvazia **e** índices corrompem — checksum diverge |
| `NO_MUTEX` | ✓ | ✗ | capacidade respeitada, mas produtores escrevem no mesmo slot — checksum diverge |
| `FULL` | ✓ | ✓ | checksum bate, sempre |

`NO_MUTEX` é a condição que **isola a exclusão mútua** como variável única — é ela que
prova o ponto do enunciado, e é por isso que o experimento tem três condições e não
duas.

**Otimização:** `-O0` é a medição oficial (mesma flag para as três condições, race
garantidamente visível). `-O2` roda à parte, como achado documentado no relatório: o
compilador também interfere na visibilidade da race. Não usar `volatile` para forçar a
race a aparecer — `volatile` não dá atomicidade, só inibe otimização, e isso distorce o
experimento.

**Parâmetros** (constantes no código, não configuráveis): `N_PRODUCERS`,
`N_CONSUMERS`, `N_ITEMS`, `BUFFER_SIZE`. Só o `sync_mode` vem de `argv` — o
experimento é reprodutível por construção.

---

## Estrutura

```
.
├── CONTEXT.md              # glossário de domínio
├── Makefile
├── src/
│   ├── buffer.h / buffer.c # buffer circular + os três semáforos
│   └── main.c              # threads, cronometragem, tabela de resultados
└── docs/
    ├── Enunciado.md        # enunciado original (não editar)
    ├── relatorio.md        # entregável
    ├── adr/                # decisões e porquês
    └── grills_logs/        # sessões de grill
```

`buffer.c` isola o recurso compartilhado e os semáforos — é onde mora (ou falta) a
exclusão mútua. `main.c` cria as threads, cronometra e imprime. A separação existe para
que o relatório possa apontar o arquivo exato onde a exclusão mútua é decidida.

---

## Agent skills

### Issue tracker

Issues vivem no GitHub Issues de `PPrauchner/SO-Trabalho-1-Semaforos`, operadas pelo
`gh` CLI. Ver `docs/agents/issue-tracker.md`.

### Triage labels

Vocabulário canônico, sem renomeações: `needs-triage`, `needs-info`, `ready-for-agent`,
`ready-for-human`, `wontfix`. Ver `docs/agents/triage-labels.md`.

### Domain docs

Contexto único — `CONTEXT.md` e `docs/adr/` na raiz. Ver `docs/agents/domain.md`.
