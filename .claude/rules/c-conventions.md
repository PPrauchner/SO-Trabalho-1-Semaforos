# Convenções de Código — C

> Complementa [`code-conventions.md`](./code-conventions.md). Só se aplica a
> projetos C.

---

## Documentação

Três níveis, em inglês (a documentação em pt-BR vive em `docs/`, não no código):

**1. Arquivo** — todo `.c` e `.h` começa com um bloco descritivo:
```c
/*
 * One-line summary of what this translation unit does.
 *
 * Responsibilities:
 * - First responsibility.
 * - Second responsibility.
 */
```

**2. Função** — comentário **no header**, junto da declaração, obrigatório quando há
≥ 2 parâmetros ou o retorno não é óbvio. O `.c` não repete o comentário:
```c
/*
 * Inserts an item into the buffer, blocking while it is full.
 *
 * buffer: target buffer, already initialised.
 * item:   value to store.
 *
 * Returns 0 on success, -1 if the buffer was shut down.
 */
int buffer_put(Buffer *buffer, int item);
```

**3. Struct / enum** — comentário no `typedef`, descrevendo o que o tipo *é*:
```c
/* Which set of semaphores is active in a given run. */
typedef enum {
    SYNC_MODE_NONE,
    SYNC_MODE_NO_MUTEX,
    SYNC_MODE_FULL,
} SyncMode;
```

Comentários explicam **por quê**, não **o quê** — com uma exceção neste projeto: onde a
sincronização é deliberadamente omitida, o comentário deve dizer que a omissão é
intencional, senão parece bug.

---

## Tipos

- **Sem tipos implícitos.** Toda função declara parâmetros e retorno; função sem
  parâmetros é `void f(void)`, nunca `void f()`.
- Tipos de largura fixa de `<stdint.h>` quando o tamanho importa (`int64_t` para o
  checksum, que soma muitos itens e transborda em 32 bits).
- `size_t` para tamanhos e índices; `bool` de `<stdbool.h>` para booleanos.
- `const` em todo ponteiro que a função não modifica.
- Funções auxiliares que não saem do arquivo são `static`.

```c
/* ✅ correto */
int64_t buffer_consumed_checksum(const Buffer *buffer);
void experiment_run(SyncMode mode);

/* ❌ errado */
int buffer_checksum();        /* sem parâmetros declarados, retorno estreito demais */
void run(void *cfg);          /* void* apaga o tipo */
```

---

## Nomes

Estende as regras de `code-conventions.md` para C:

| Elemento | Convenção | Exemplo |
|---|---|---|
| Função, variável | `snake_case` | `buffer_put`, `items_consumed` |
| Constante, macro | `UPPER_SNAKE_CASE` | `BUFFER_SIZE`, `N_PRODUCERS` |
| `typedef` de struct/enum | `PascalCase` | `Buffer`, `SyncMode` |
| Valor de enum | `UPPER_SNAKE_CASE` com prefixo do tipo | `SYNC_MODE_FULL` |
| Campo de struct | `snake_case` | `buffer->write_index` |

Funções públicas de um módulo levam o nome do módulo como prefixo: `buffer_init`,
`buffer_put`, `buffer_take`, `buffer_destroy`.

---

## Headers

- Include guard em `PASCAL_CASE_H` derivado do nome do arquivo (`BUFFER_H`).
- O header declara só o que é público; tudo o mais fica `static` no `.c`.
- Cada arquivo inclui o que usa — não depender de include transitivo.

---

## Erros

Chamada de sistema ou de `pthread`/`sem` que pode falhar tem retorno verificado.
Falha na criação de thread ou na inicialização de semáforo é erro fatal: reportar em
`stderr` e sair com status diferente de zero.

Não escrever tratamento de erro para cenários impossíveis — a regra de simplicidade de
`karpathy-principles.md` continua valendo.
