# `-O0` como flag da medição oficial, e nunca `volatile`

A bateria oficial compila as três condições com `-O0`, e a rodada `-O2` existe à parte,
como achado documentado. Medir desempenho com otimização desligada é contraintuitivo o
suficiente para precisar de registro: a razão não é desempenho, é **controle de
variável**. O nível de otimização interfere na visibilidade da corrida — com `-O2` o
gcc pode manter estado compartilhado em registrador —, então ele precisa ser mantido
fixo, e não escolhido pelo melhor número.

O que o experimento afirma é uma **razão entre condições**, não um tempo absoluto.
Comparar o `-O0` de uma condição com o `-O2` de outra mediria o compilador, não a
sincronização. Sob a mesma flag, a comparação é válida — e é por isso que a mesma flag
vale para as três condições, mesmo sendo a flag "errada" para medir performance real.

Decorre disso uma segunda regra: **nunca usar `volatile` para forçar a corrida a
aparecer.** `volatile` não dá atomicidade — apenas inibe otimização. Recorrer a ele
faria a demonstração medir o compilador em vez do sistema operacional, e ainda
sugeriria, falsamente, que a corrida é um artefato de otimização que `volatile`
corrige. A corrida é uma propriedade do acesso concorrente não serializado, e a rodada
`-O2` mostra que ela sobrevive à otimização: 20 de 20 execuções divergentes em
`NO_MUTEX` sob as duas flags.

## Consequences

- Os tempos absolutos do relatório **não** representam desempenho de código otimizado,
  e o relatório diz isso explicitamente. A afirmação sustentável é a razão
  `FULL`/`NO_MUTEX`, a única comparação em que só a exclusão mútua muda.
- O Makefile mantém dois binários separados (`bin/prodcons` e `bin/prodcons-o2`) e dois
  alvos de bateria. Eles não se misturam de propósito.
- `make battery-opt` tolera falha (prefixo `-` na receita): um `FAIL` sob `-O2` seria
  resultado a relatar — a janela da corrida teria fechado naquela plataforma —, não
  quebra de build.
- Nenhuma variável do programa é `volatile`, e nenhum `sleep` ou ajuste artificial
  provoca a corrida. Introduzir qualquer um dos dois invalida a medição e reabre esta
  decisão.
