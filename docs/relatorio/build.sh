#!/usr/bin/env bash
#
# Compõe docs/relatorio.pdf a partir de docs/relatorio.md.
#
# Requer: pandoc, xelatex (texlive-xetex), fontes TeX Gyre, python3 + matplotlib.
# Uso: bash docs/relatorio/build.sh
set -euo pipefail
cd "$(dirname "$0")/.."          # docs/

python3 relatorio/figuras.py     # regera as figuras (PNG para o .md, PDF para o LaTeX)

# No PDF final as figuras entram em vetorial; no Markdown, em PNG (renderiza no GitHub).
sed 's|relatorio/\(fig[a-z0-9-]*\)\.png|relatorio/\1.pdf|g' relatorio.md > .relatorio-print.md

pandoc .relatorio-print.md \
  --from markdown+smart+tex_math_dollars+table_captions+raw_tex \
  --to pdf \
  --pdf-engine=xelatex \
  --highlight-style=kate \
  --template=relatorio/template.tex \
  --include-in-header=relatorio/preamble.tex \
  --include-before-body=<(printf '\\capa\n') \
  -V documentclass=article \
  -V papersize=a4 \
  -V fontsize=11pt \
  -V geometry:top=2.6cm,bottom=2.4cm,left=2.6cm,right=2.6cm,headsep=14pt \
  -V secnumdepth=3 \
  --number-sections \
  -o relatorio.pdf

rm -f .relatorio-print.md
echo "docs/relatorio.pdf gerado"
