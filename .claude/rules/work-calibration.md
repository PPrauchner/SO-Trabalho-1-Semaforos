# Calibragem de Trabalho

> Como **este projeto** divide trabalho: quando uma issue é grande demais para uma
> tacada, e o que conta como uma camada na hora de fatiar os commits.
>
> Preencher na sessão de *grill with docs* (skill `grill-with-docs`, log em
> `docs/grills_logs/`). Seção vazia significa: valem os defaults genéricos, descritos
> nos guias que apontam para cá.
>
> Este arquivo é **do projeto**, não do template: o `/update-claude` só o instala se
> faltar, e nunca sobrescreve o que você escreveu aqui.

---

## Quebra de trabalho

> Consumido por [`complexity-guide.md`](../commands/start-issue/complexity-guide.md),
> que o `/start-issue` usa para decidir se a issue vira implementação direta ou plano
> de sub-tarefas.
>
> O que entra aqui: limiares diferentes ("neste repo, dois módulos já bastam para
> quebrar"), a unidade que o repositório usa no lugar de "módulo/diretório" (pacote,
> serviço, contexto), ou critérios próprios que a régua genérica não captura.

O projeto é **um único programa** de algumas centenas de linhas. A régua genérica vale,
com um ajuste: aqui a unidade de quebra não é "módulo", é **entregável do enunciado** —
o programa, a medição de tempo e o relatório.

Uma issue vira plano de sub-tarefas quando toca mais de um desses três entregáveis ao
mesmo tempo. Dentro do código, `buffer` e `main` juntos ainda contam como uma tacada só:
são as duas metades da mesma mudança.

---

## Camadas deste projeto

> Consumido por [`atomicity-rules.md`](../commands/commit/atomicity-rules.md), que o
> `/commit` usa para agrupar mudanças em commits atômicos.
>
> O que entra aqui: quais pastas são camadas neste repositório, ou a declaração de
> que ele não é organizado em camadas — nesse caso vale só "um domínio por commit".

**Este repositório não é organizado em camadas.** Vale apenas a regra de um domínio por
commit — o buffer e seus semáforos, a cronometragem, o relatório, cada um no seu commit.

`src/` e `docs/` são pastas, não camadas: uma mudança no experimento que exija atualizar
o relatório pode ir num commit só, se o relatório estiver apenas registrando o que o
código passou a fazer.
