#!/usr/bin/env bash
#
# Compõe docs/relatorio.pdf a partir de docs/relatorio.md.
#
# Requer: pandoc, xelatex (texlive-xetex), fontes TeX Gyre, python3 + matplotlib.
# Uso: bash docs/relatorio/build.sh
#
# As dependencias sao verificadas antes de qualquer trabalho: sem o preflight, a
# falta de matplotlib abortava o script no primeiro comando com um traceback de
# Python, que nao dizia o que instalar.
set -euo pipefail
cd "$(dirname "$0")/.."          # docs/

# Verifica a toolchain e lista de uma vez tudo o que falta, em vez de morrer no
# primeiro ausente.
preflight() {
    local missing=()

    for command_name in pandoc xelatex python3; do
        command -v "$command_name" >/dev/null 2>&1 || missing+=("$command_name")
    done
    if command -v python3 >/dev/null 2>&1 &&
       ! python3 -c 'import matplotlib' >/dev/null 2>&1; then
        missing+=("python3-matplotlib")
    fi

    if [ "${#missing[@]}" -eq 0 ]; then
        return 0
    fi

    printf 'build.sh: dependencias ausentes: %s

' "${missing[*]}" >&2
    cat >&2 <<'HELP'
Este script compoe o PDF a partir do Markdown; ele nao e necessario para rodar o
experimento (`make check` nao depende dele). O docs/relatorio.pdf versionado foi
composto num ambiente com a toolchain completa.

Para instalar no WSL Ubuntu:

    sudo apt update
    sudo apt install -y pandoc texlive-xetex texlive-fonts-extra python3-matplotlib

texlive-fonts-extra traz as fontes TeX Gyre (Pagella e Heros) que preamble.tex usa.
HELP
    return 1
}

if ! preflight; then
    exit 1
fi

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
