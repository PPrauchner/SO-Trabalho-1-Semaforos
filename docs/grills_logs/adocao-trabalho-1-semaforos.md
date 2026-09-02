# Grill — Adoção do repositório: Trabalho 1 (Threads + Semáforos)

Início: 2026-09-02 19:43

Sessão conduzida pela skill `adopt-repo`, com o grill delegado à `grill-with-docs`.
O repositório estava sem código: apenas o template `.claude/` (ARK v4.1.0) e
`docs/Enunciado.md`.

## Sessão — 2026-09-02 19:43

**P:** Qual linguagem vai ser usada no trabalho? (opções: Java / Python / C)
**R:** "Qual a mais eficiente? E, pq? Acredito que c, mas preciso saber explicar o
que foi implementado, mantenha o log dessa sessão de grill."

**P:** Qual problema clássico vai ser implementado?
**R:** Produtor-consumidor.

**P:** O projeto vai virar um repositório git com issue tracker?
**R:** Sim — GitHub.

### Recon de toolchain (feito para responder à pergunta de eficiência)

- `javac`/`java` versão 25 — nativos no Windows, no PATH.
- `gcc` 15.2.0 disponível no WSL Ubuntu; `nproc` = 6.
- Sem compilador C nativo no Windows (`gcc`, `clang`, `cl` ausentes).
- Python 3.11.9 disponível.

### Decisão de stack

**P:** Fechamos C + pthreads + semáforos POSIX, compilando e rodando no WSL Ubuntu?
**R:** Sim — C no WSL.

**Raciocínio registrado (resposta à pergunta "qual a mais eficiente e por quê"):**

Dois sentidos de "eficiente", que não apontam para o mesmo lugar:

1. *Runtime*: C ganha. `pthread_create` cria thread do SO direto; `sem_t` é futex
   (~20–25 ns sem contenção); sem runtime, sem GC, sem JIT. Java, após aquecimento do
   JIT, fica tipicamente dentro de 1,5–2× — mas "após aquecimento" não é detalhe: sem
   rodadas de warmup a medição mede compilação JIT, não sincronização. Python foi
   descartado pelo **GIL**: o semáforo é real e a race acontece (o GIL é liberado entre
   bytecodes), mas duas threads nunca executam bytecode simultaneamente — mediríamos
   contenção de GIL chamando de custo de sincronização, num trabalho cujo enunciado
   abre falando em ambiente multiprocessado.

2. *Eficiência para produzir a prova pedida*: C tem vantagem que não é de velocidade —
   **as primitivas são os conceitos**. `sem_wait`/`sem_post` são o P e o V de Dijkstra
   citados no enunciado, sem camada intermediária a justificar. Java esconde o
   escalonamento atrás da JVM e o semáforo atrás do `AbstractQueuedSynchronizer`.

Custos aceitos do C: roda no WSL (edita no Windows, compila com `wsl gcc -pthread`);
atenção deliberada a `-O0` vs `-O2` (com otimização o gcc pode manter o contador
compartilhado em registrador e a race muda de cara ou desaparece — vira achado do
relatório, não acidente); gerência manual de memória, trivial num buffer circular
estático.

### Desenho do experimento

**P:** Quantas condições o experimento vai comparar?
**R:** Três — (1) sem sincronização nenhuma; (2) só `empty`/`full`, sem mutex;
(3) completo. A condição 2 isola a exclusão mútua como variável única: o buffer nunca
estoura, mas dois produtores escrevem no mesmo slot.

**P:** Como o programa vai detectar que a exclusão mútua falhou?
**R:** Checksum — cada produtor soma os itens que gerou, cada consumidor soma os que
retirou; ao final as somas têm que bater. Dá número quantificável para o relatório, em
vez de apenas "deu errado". Descartadas: contagem simples (cega à sobrescrita de slot)
e flag de slot ocupado (a própria flag seria dado compartilhado sem proteção, com race
na detecção).

### Terminologia

**Colisão apontada no grill:** em C com threads, `condition` já significa
`pthread_cond_t` (variável de condição). Usar `condition` para as variantes do
experimento confundiria justamente o leitor que entende do assunto.

**P:** Como nomear as três variantes no código?
**R:** `sync_mode` — enum `SyncMode { NONE, NO_MUTEX, FULL }`. Diz o que de fato varia
entre as variantes. Em pt-BR, no relatório, continua "condição experimental" (termo
correto em ciência, sem colisão no código). Descartados: `variant` (não diz o que
varia) e `scenario` (vocabulário de teste, não de domínio).

**P:** Os parâmetros do experimento ficam fixos ou configuráveis?
**R:** Constantes fixas (`N_PRODUCERS`, `N_CONSUMERS`, `N_ITEMS`, `BUFFER_SIZE`); só o
`sync_mode` vem de `argv`. Reprodutível por construção.

### Restrições

**Nota do grill:** recorrer a `volatile` para forçar a race a aparecer sob otimização é
ela própria uma distorção do experimento — `volatile` não dá atomicidade, apenas inibe
otimização; seria intervenção do compilador, não do sistema operacional.

**P:** Com qual nível de otimização as três condições vão ser medidas?
**R:** `-O0` como medição oficial (mesma flag para as três, comparação justa e race
garantidamente visível) **e `-O2` como achado extra**, documentando se a race muda ou
desaparece sob otimização.

**P:** Onde vive o relatório que o enunciado pede?
**R:** `docs/relatorio.md`, versionado no repositório, em pt-BR. Tabelas de medição
vindas do stdout do programa. Conversão para PDF/DOCX só na entrega, se exigido.

**P:** Qual o nome do repositório no GitHub?
**R:** `PPrauchner/SO-Trabalho-1-Semaforos`.

### Estrutura e calibragem

**P:** Como dividir o código em arquivos?
**R:** `src/buffer.{c,h}` + `src/main.c` + `Makefile`. `buffer.c` isola o buffer
circular e os três semáforos (o que o `sync_mode` liga e desliga); `main.c` cria as
threads, cronometra e imprime. A separação torna óbvio no relatório *onde* mora a
exclusão mútua.

**P:** O projeto tem camadas, para efeito de commits atômicos?
**R:** Não — um domínio por commit. `src/` e `docs/` são pastas, não camadas.

**P:** Vale um ADR retroativo para a escolha de C/pthreads sobre Java?
**R:** Sim — `docs/adr/0001-c-pthreads-em-vez-de-java.md`. Passa nos três critérios:
difícil de reverter, surpreendente sem contexto (o enunciado sugere Java) e fruto de
trade-off real.

### Configuração do tracker (setup-matt-pocock-skills)

- **Issue tracker:** GitHub Issues via `gh` CLI (`gh` autenticado como `PPrauchner`).
- **Triage labels:** vocabulário canônico, sem renomeações.
- **Domain docs:** contexto único — `CONTEXT.md` + `docs/adr/` na raiz.
- Repositório criado como **privado**.

Fim da sessão de adoção.
