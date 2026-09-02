# Convenções de Código

> Lido pelo agente ao escrever ou revisar código. Para o modelo de domínio, ver
> `CONTEXT.md`; para as decisões de arquitetura e seus porquês, `docs/adr/`.
>
> As restrições específicas deste projeto devem ser definidas na sessão de *grill
> with docs* (skill `grill-with-docs`, log em `docs/grills_logs/`) e registradas na
> seção [Restrições deste projeto](#restrições-deste-projeto) abaixo.

---

## Idioma

- **Código** (módulos, identificadores, docstrings) em **inglês**.
- **Documentação** (ADRs, `CONTEXT.md`, README, log) em **português**.
- O glossário em `CONTEXT.md` faz a ponte entre o termo de domínio (pt-BR) e o
  identificador no código (inglês).

---

## Convenções por linguagem

As regras específicas de cada linguagem vivem em arquivos separados, para não
assumir uma stack que o projeto não usa:

- **C** → [`c-conventions.md`](./c-conventions.md)
- Outras linguagens: adicionar `rules/<linguagem>-conventions.md` conforme o
  projeto precisar (ex.: `typescript-conventions.md`).

---

## Clean Code

- Funções com responsabilidade única — se o nome precisar de "e"/"ou", dividir.
- Nomes descritivos: sem abreviações opacas (`nd` → `node`, `sz` → `size`).
- Constantes em `UPPER_SNAKE_CASE`; variáveis e funções em `snake_case`; classes em
  `PascalCase`.
- Comentários explicam *por quê*, não *o quê*.
- Ver também `.claude/rules/karpathy-principles.md` (simplicidade primeiro,
  mudanças cirúrgicas, execução orientada a metas).

---

## Restrições deste projeto

> Preencher na sessão de *grill with docs* deste projeto. Modelo de domínio em
> [`CONTEXT.md`](../../CONTEXT.md); decisões e porquês em [`docs/adr/`](../../docs/adr/).
>
> Exemplos do que entra aqui: linguagem/versão, dependências centrais vs.
> opcionais, tipo de interface (CLI/web/API), formato de persistência,
> reprodutibilidade, estrutura de pastas — o que for específico e não-óbvio deste
> projeto.

- **Linguagem: C (C11), sem dependências externas.** Só a biblioteca padrão,
  `pthread.h` e `semaphore.h`. Escolha justificada no
  [ADR 0001](../../docs/adr/0001-c-pthreads-em-vez-de-java.md) — não trocar sem revisitá-lo.
- **Sincronização por semáforos POSIX** (`sem_t`). Não usar `pthread_mutex_t` nem
  `pthread_cond_t`: o enunciado pede semáforos, e a exclusão mútua deve vir de um
  semáforo binário para que o contraste entre semáforo contador e mutex fique explícito.
- **Ambiente de execução: WSL Ubuntu.** Não há compilador C no Windows nativo. Não
  introduzir código específico da API Win32 — o alvo é POSIX.
- **`-O0` é a flag da medição oficial**, igual para as três condições. `-O2` só como
  execução extra documentada no relatório.
- **Nunca usar `volatile` para forçar a race a aparecer.** `volatile` não dá
  atomicidade, apenas inibe otimização; usá-lo faria a demonstração medir o compilador
  em vez do sistema operacional.
- **Parâmetros do experimento são constantes no código** (`N_PRODUCERS`,
  `N_CONSUMERS`, `N_ITEMS`, `BUFFER_SIZE`). Só o `sync_mode` vem de `argv`. Isso é
  requisito de reprodutibilidade, não preguiça de parsing.
- **O checksum é a evidência.** Toda alteração no fluxo de produção/consumo tem que
  preservar a propriedade "soma produzida = soma consumida sob `FULL`".
- **`docs/Enunciado.md` não se edita** — é o enunciado original do professor.
- **O relatório (`docs/relatorio.md`) é entregável**, em pt-BR, versionado junto do
  código.
