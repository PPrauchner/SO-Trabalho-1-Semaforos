"""
Verifica que todas as cópias dos números da bateria concordam com o Apêndice A.

Os blocos ```text do Apêndice A de `docs/relatorio.md` são cópia literal da saída
de `tests/battery.sh` e, por isso, a fonte da verdade. Este script relê esses
blocos e confere contra eles as demais cópias dos mesmos números.

Responsabilidades:
- Parsear os blocos verbatim da bateria (-O0 e -O2) do relatório.
- Conferir os dicts `O0`/`O2` de `docs/relatorio/figuras.py` (por texto: matplotlib
  não é dependência deste script).
- Conferir as tabelas em pt-BR do relatório e do `README.md`.
- Conferir as razões derivadas citadas na prosa ("1,65×", "33×").

Só usa a biblioteca padrão. Os caminhos são resolvidos a partir da localização
deste arquivo, portanto pode ser invocado de qualquer diretório:

    python3 docs/relatorio/checa-numeros.py

Sai com status 0 se tudo confere; 1 e um relatório das divergências caso contrário.
"""

from __future__ import annotations

import ast
import re
import sys
from decimal import Decimal, ROUND_HALF_UP
from pathlib import Path

REPORT_PATH = Path(__file__).resolve().parents[1] / "relatorio.md"
FIGURES_PATH = Path(__file__).resolve().parent / "figuras.py"
README_PATH = Path(__file__).resolve().parents[2] / "README.md"

MODES = ("full", "no-mutex", "none")

BATTERY_LINE_RE = re.compile(
    r"mode=(?P<mode>\S+)\s+runs=(?P<runs>\d+)\s+divergent=(?P<divergent>\d+)\s+"
    r"\S+\s+time_ms\s+min=(?P<tmin>[\d.]+)\s+mean=(?P<tmean>[\d.]+)\s+"
    r"max=(?P<tmax>[\d.]+)"
)
FENCE_RE = re.compile(r"```text\n(.*?)```", re.DOTALL)
RATIO_RE = re.compile(r"\*{0,2}(\d+(?:,\d+)?)×\*{0,2}")

# Razão média(FULL) / média(NO_MUTEX): citada com duas casas decimais.
RATIO_FULL_OVER_NO_MUTEX_DECIMALS = 2
# Razão média(FULL) / média(NONE): citada como inteiro.
RATIO_FULL_OVER_NONE_DECIMALS = 0


def read_text(path: Path) -> str:
    """Lê um arquivo de texto normalizando fins de linha CRLF.

    Args:
        path: Caminho do arquivo a ler.

    Returns:
        O conteúdo do arquivo com `\\n` como único fim de linha.
    """
    return path.read_text(encoding="utf-8").replace("\r\n", "\n")


def format_br(value: float, decimals: int) -> str:
    """Formata um número no padrão pt-BR, com arredondamento half-up.

    Args:
        value: Valor a formatar.
        decimals: Quantidade de casas decimais.

    Returns:
        O número como texto, com vírgula decimal.
    """
    quantum = Decimal(1).scaleb(-decimals)
    rounded = Decimal(repr(value)).quantize(quantum, rounding=ROUND_HALF_UP)
    return f"{rounded}".replace(".", ",")


def parse_battery_blocks(report: str) -> dict[str, dict[str, dict[str, float]]]:
    """Extrai os blocos verbatim da bateria no Apêndice A do relatório.

    Args:
        report: Conteúdo do relatório.

    Returns:
        Mapa de flag de compilação ("O0"/"O2") para modo e suas métricas
        (`divergent`, `tmin`, `tmean`, `tmax`).

    Raises:
        SystemExit: Se os dois blocos esperados não forem encontrados.
    """
    blocks: dict[str, dict[str, dict[str, float]]] = {}
    for body in FENCE_RE.findall(report):
        if "bateria sobre" not in body:
            continue
        key = "O2" if "prodcons-o2" in body else "O0"
        metrics = {
            match.group("mode"): {
                "divergent": float(match.group("divergent")),
                "tmin": float(match.group("tmin")),
                "tmean": float(match.group("tmean")),
                "tmax": float(match.group("tmax")),
            }
            for match in BATTERY_LINE_RE.finditer(body)
        }
        if set(metrics) != set(MODES):
            sys.exit(
                f"ERRO: bloco da bateria {key} no Apêndice A não traz os três modos; "
                f"encontrados: {sorted(metrics)}"
            )
        blocks[key] = metrics

    if set(blocks) != {"O0", "O2"}:
        sys.exit(
            "ERRO: o Apêndice A deveria conter os blocos da bateria -O0 e -O2; "
            f"encontrados: {sorted(blocks)}"
        )
    return blocks


def parse_figures_dicts(figures: str) -> dict[str, dict[str, dict[str, float]]]:
    """Extrai os dicts `O0` e `O2` de `figuras.py` sem importar o módulo.

    O módulo importa matplotlib no topo, que não é dependência deste script; por
    isso a leitura é textual, com `ast` avaliando cada chamada `dict(...)`.

    Args:
        figures: Conteúdo de `figuras.py`.

    Returns:
        Mapa de flag de compilação para modo e suas métricas.

    Raises:
        SystemExit: Se um dict esperado não for encontrado.
    """
    parsed: dict[str, dict[str, dict[str, float]]] = {}
    for key in ("O0", "O2"):
        block = re.search(rf"^{key}\s*=\s*\{{(.*?)^\}}", figures, re.DOTALL | re.MULTILINE)
        if block is None:
            sys.exit(f"ERRO: dict {key} não encontrado em {FIGURES_PATH.name}")

        metrics: dict[str, dict[str, float]] = {}
        entry_re = re.compile(r"\"(?P<mode>[\w-]+)\"\s*:\s*dict\((?P<args>[^)]*)\)")
        for match in entry_re.finditer(block.group(1)):
            call = ast.parse(f"dict({match.group('args')})", mode="eval").body
            metrics[match.group("mode")] = {
                keyword.arg: float(ast.literal_eval(keyword.value))
                for keyword in call.keywords  # type: ignore[attr-defined]
                if keyword.arg is not None
            }
        if set(metrics) != set(MODES):
            sys.exit(
                f"ERRO: dict {key} em {FIGURES_PATH.name} não traz os três modos; "
                f"encontrados: {sorted(metrics)}"
            )
        parsed[key] = metrics
    return parsed


def parse_markdown_table(text: str, header_terms: tuple[str, ...]) -> dict[str, list[str]]:
    """Localiza uma tabela markdown pelo cabeçalho e devolve suas linhas.

    Args:
        text: Conteúdo do arquivo markdown.
        header_terms: Termos que devem todos aparecer na linha de cabeçalho.

    Returns:
        Mapa do modo (nome canônico, ex.: "no-mutex") para as células restantes
        da linha, como texto cru.

    Raises:
        SystemExit: Se nenhuma tabela com esse cabeçalho for encontrada.
    """
    lines = text.split("\n")
    for index, line in enumerate(lines):
        if not line.startswith("|") or not all(term in line for term in header_terms):
            continue
        rows: dict[str, list[str]] = {}
        for row in lines[index + 2:]:
            if not row.startswith("|"):
                break
            cells = [cell.strip() for cell in row.strip().strip("|").split("|")]
            mode = canonical_mode(cells[0])
            if mode is not None:
                rows[mode] = cells[1:]
        if set(rows) == set(MODES):
            return rows

    sys.exit(f"ERRO: tabela com cabeçalho {header_terms} não encontrada")


def canonical_mode(cell: str) -> str | None:
    """Traduz o rótulo de um modo, como aparece nas tabelas, para o nome canônico.

    Args:
        cell: Primeira célula da linha da tabela (ex.: "`NO_MUTEX`").

    Returns:
        O nome usado na saída da bateria, ou None se a célula não nomear um modo.
    """
    label = cell.strip("`* ").lower().replace("_", "-")
    return label if label in MODES else None


def check_figures(
    source: dict[str, dict[str, dict[str, float]]],
    figures: dict[str, dict[str, dict[str, float]]],
) -> list[str]:
    """Confere os dicts de `figuras.py` contra os blocos da bateria.

    Args:
        source: Métricas da fonte da verdade, por flag de compilação.
        figures: Métricas lidas de `figuras.py`, por flag de compilação.

    Returns:
        Lista de divergências encontradas, vazia se tudo confere.
    """
    problems: list[str] = []
    for key in ("O0", "O2"):
        for mode in MODES:
            for field in ("divergent", "tmin", "tmean", "tmax"):
                expected = source[key][mode][field]
                found = figures[key][mode][field]
                if expected != found:
                    problems.append(
                        f"{FIGURES_PATH.name}: {key}[{mode!r}][{field!r}] — "
                        f"esperado {expected:g}, encontrado {found:g}"
                    )
    return problems


def check_results_table(source: dict[str, dict[str, dict[str, float]]], report: str) -> list[str]:
    """Confere a tabela de "Resultados" (-O0) do relatório.

    Args:
        source: Métricas da fonte da verdade, por flag de compilação.
        report: Conteúdo do relatório.

    Returns:
        Lista de divergências encontradas, vazia se tudo confere.
    """
    rows = parse_markdown_table(report, ("Execuções", "Divergentes", "Mín."))
    problems: list[str] = []
    columns = (("Divergentes", "divergent", 0), ("Mín.", "tmin", 1),
               ("Média", "tmean", 1), ("Máx.", "tmax", 1))
    for mode in MODES:
        cells = rows[mode]
        for offset, (column, field, decimals) in enumerate(columns, start=1):
            expected = format_br(source["O0"][mode][field], decimals)
            found = cells[offset].strip("* ")
            if expected != found:
                problems.append(
                    f"relatorio.md (tabela Resultados): {mode} / {column} — "
                    f"esperado {expected}, encontrado {found}"
                )
    return problems


def check_comparison_table(
    source: dict[str, dict[str, dict[str, float]]], report: str
) -> list[str]:
    """Confere a tabela de comparação -O0 vs -O2 do relatório.

    Args:
        source: Métricas da fonte da verdade, por flag de compilação.
        report: Conteúdo do relatório.

    Returns:
        Lista de divergências encontradas, vazia se tudo confere.
    """
    rows = parse_markdown_table(report, ("Diverg.", "Média", "`-O2`"))
    problems: list[str] = []
    columns = (("Diverg. -O0", "O0", "divergent", 0), ("Diverg. -O2", "O2", "divergent", 0),
               ("Média -O0", "O0", "tmean", 1), ("Média -O2", "O2", "tmean", 1))
    for mode in MODES:
        cells = rows[mode]
        for offset, (column, key, field, decimals) in enumerate(columns):
            expected = format_br(source[key][mode][field], decimals)
            found = cells[offset].strip("* ")
            if expected != found:
                problems.append(
                    f"relatorio.md (tabela -O0 vs -O2): {mode} / {column} — "
                    f"esperado {expected}, encontrado {found}"
                )
    return problems


def check_readme_table(source: dict[str, dict[str, dict[str, float]]], readme: str) -> list[str]:
    """Confere a tabela de resultados do `README.md`.

    Args:
        source: Métricas da fonte da verdade, por flag de compilação.
        readme: Conteúdo do README.

    Returns:
        Lista de divergências encontradas, vazia se tudo confere.
    """
    rows = parse_markdown_table(readme, ("Divergentes", "Tempo médio"))
    problems: list[str] = []
    for mode in MODES:
        cells = rows[mode]
        expected_divergent = format_br(source["O0"][mode]["divergent"], 0)
        found_divergent = cells[2].strip("* ")
        if expected_divergent != found_divergent:
            problems.append(
                f"README.md: {mode} / Divergentes — "
                f"esperado {expected_divergent}, encontrado {found_divergent}"
            )
        expected_mean = f"{format_br(source['O0'][mode]['tmean'], 1)} ms"
        found_mean = cells[3].strip("* ")
        if expected_mean != found_mean:
            problems.append(
                f"README.md: {mode} / Tempo médio — "
                f"esperado {expected_mean}, encontrado {found_mean}"
            )
    return problems


def check_prose_ratios(
    source: dict[str, dict[str, dict[str, float]]], report: str, readme: str
) -> list[str]:
    """Confere as razões derivadas citadas na prosa do relatório e do README.

    As duas razões citadas — FULL/NO_MUTEX com duas decimais e FULL/NONE inteira —
    são recalculadas das médias da fonte e arredondadas como a prosa as escreve;
    qualquer razão citada que não seja uma das duas é reportada.

    Args:
        source: Métricas da fonte da verdade, por flag de compilação.
        report: Conteúdo do relatório.
        readme: Conteúdo do README.

    Returns:
        Lista de divergências encontradas, vazia se tudo confere.
    """
    means = source["O0"]
    expected = {
        format_br(means["full"]["tmean"] / means["no-mutex"]["tmean"],
                  RATIO_FULL_OVER_NO_MUTEX_DECIMALS),
        format_br(means["full"]["tmean"] / means["none"]["tmean"],
                  RATIO_FULL_OVER_NONE_DECIMALS),
    }

    problems: list[str] = []
    for name, text in (("relatorio.md", report), ("README.md", readme)):
        for line_number, line in enumerate(text.split("\n"), start=1):
            for found in RATIO_RE.findall(line):
                if found not in expected:
                    problems.append(
                        f"{name}:{line_number} (prosa): razão {found}× — "
                        f"esperada uma de {'×, '.join(sorted(expected))}×"
                    )
    return problems


def main() -> int:
    """Roda todas as verificações e reporta o resultado.

    Returns:
        0 se todas as cópias concordam com o Apêndice A, 1 caso contrário.
    """
    report = read_text(REPORT_PATH)
    readme = read_text(README_PATH)
    figures = read_text(FIGURES_PATH)

    source = parse_battery_blocks(report)
    groups = (
        ("figuras.py (dicts O0/O2)", check_figures(source, parse_figures_dicts(figures))),
        ("relatorio.md (tabela Resultados)", check_results_table(source, report)),
        ("relatorio.md (tabela -O0 vs -O2)", check_comparison_table(source, report)),
        ("README.md (tabela de resultados)", check_readme_table(source, readme)),
        ("prosa (razões derivadas)", check_prose_ratios(source, report, readme)),
    )

    failed = False
    for label, problems in groups:
        if problems:
            failed = True
            print(f"FALHA  {label}")
            for problem in problems:
                print(f"       - {problem}")
        else:
            print(f"ok     {label}")

    if failed:
        print("\nDivergência: o Apêndice A de docs/relatorio.md é a fonte da verdade.")
        return 1
    print("\nTodas as cópias concordam com o Apêndice A.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
