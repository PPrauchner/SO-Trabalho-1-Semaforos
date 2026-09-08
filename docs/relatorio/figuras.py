"""Figuras do relatório — produtor-consumidor com semáforos.

Paleta categórica (dataviz, slots 1–3, validada all-pairs em modo claro):
  FULL = azul, NO_MUTEX = laranja, NONE = aqua.
A cor segue a *condição experimental*, não o resultado — a mesma cor
identifica o mesmo modo em todas as figuras.
"""

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib import font_manager

# ---------------------------------------------------------------- tokens
SURFACE = "#fcfcfb"
INK = "#0b0b0b"
INK_2 = "#52514e"
MUTED = "#898781"
GRID = "#e1e0d9"
BASELINE = "#c3c2b7"

MODES = ["full", "no-mutex", "none"]
LABEL = {"full": "FULL", "no-mutex": "NO_MUTEX", "none": "NONE"}
COLOR = {"full": "#2a78d6", "no-mutex": "#eb6834", "none": "#1baf7a"}

# ------------------------------------------------------------------ dados
# Bateria oficial (-O0), 20 execuções por modo.
O0 = {
    "full":     dict(divergent=0,  tmin=142.221, tmean=159.051, tmax=196.115),
    "no-mutex": dict(divergent=20, tmin=79.909,  tmean=96.429,  tmax=117.327),
    "none":     dict(divergent=20, tmin=3.836,   tmean=4.801,   tmax=5.857),
}
# Rodada à parte (-O2).
O2 = {
    "full":     dict(divergent=0,  tmin=127.942, tmean=164.513, tmax=265.145),
    "no-mutex": dict(divergent=20, tmin=82.933,  tmean=96.193,  tmax=116.359),
    "none":     dict(divergent=20, tmin=0.705,   tmean=2.854,   tmax=4.202),
}
RUNS = 20

# ------------------------------------------------------------------ estilo
def pick_font():
    for name in ("TeX Gyre Heros", "DejaVu Sans"):
        try:
            font_manager.findfont(font_manager.FontProperties(family=name),
                                  fallback_to_default=False)
            return name
        except Exception:
            continue
    return "DejaVu Sans"

plt.rcParams.update({
    "font.family": pick_font(),
    "font.size": 9,
    "text.color": INK,
    "axes.labelcolor": INK_2,
    "axes.edgecolor": BASELINE,
    "xtick.color": MUTED,
    "ytick.color": MUTED,
    "figure.facecolor": SURFACE,
    "axes.facecolor": SURFACE,
    "savefig.facecolor": SURFACE,
    "axes.spines.top": False,
    "axes.spines.right": False,
    "axes.spines.left": False,
})


def dress(ax, ylabel=None):
    ax.yaxis.grid(True, color=GRID, linewidth=0.6, zorder=0)
    ax.set_axisbelow(True)
    ax.spines["bottom"].set_color(BASELINE)
    ax.spines["bottom"].set_linewidth(0.8)
    ax.tick_params(axis="both", length=0, labelsize=8.5)
    if ylabel:
        ax.set_ylabel(ylabel, fontsize=8.5, color=INK_2, labelpad=8)


def br(x, decimals=0):
    """Formata número no padrão pt-BR."""
    s = f"{x:,.{decimals}f}"
    return s.replace(",", " ").replace(".", ",")


# ------------------------------------------------- fig 1: divergências
fig, ax = plt.subplots(figsize=(5.6, 2.5), dpi=300)
xs = range(len(MODES))
vals = [O0[m]["divergent"] for m in MODES]
bars = ax.bar(xs, vals, width=0.5, color=[COLOR[m] for m in MODES], zorder=3)

for x, m, v in zip(xs, MODES, vals):
    ax.text(x, v + 0.7 if v else 0.7, f"{v}/{RUNS}",
            ha="center", va="bottom", fontsize=10, color=INK, fontweight="bold")

ax.set_xticks(list(xs))
ax.set_xticklabels([LABEL[m] for m in MODES], fontsize=9, color=INK_2)
ax.set_ylim(0, 24)
ax.set_yticks([0, 5, 10, 15, 20])
dress(ax, "execuções divergentes")
fig.tight_layout(pad=0.6)
fig.savefig("relatorio/fig1-divergencias.pdf"); fig.savefig("relatorio/fig1-divergencias.png", dpi=300)
plt.close(fig)

# ------------------------------------------------------ fig 2: tempos -O0
fig, ax = plt.subplots(figsize=(5.6, 2.7), dpi=300)
means = [O0[m]["tmean"] for m in MODES]
lo = [O0[m]["tmean"] - O0[m]["tmin"] for m in MODES]
hi = [O0[m]["tmax"] - O0[m]["tmean"] for m in MODES]

ax.bar(xs, means, width=0.5, color=[COLOR[m] for m in MODES], zorder=3)
ax.errorbar(list(xs), means, yerr=[lo, hi], fmt="none",
            ecolor=INK_2, elinewidth=1.1, capsize=5, capthick=1.1, zorder=4)

for x, m in zip(xs, MODES):
    d = O0[m]
    ax.text(x, d["tmax"] + 7, f"{br(d['tmean'], 1)} ms",
            ha="center", va="bottom", fontsize=9.5, color=INK, fontweight="bold")

ax.set_xticks(list(xs))
ax.set_xticklabels([LABEL[m] for m in MODES], fontsize=9, color=INK_2)
ax.set_ylim(0, 235)
dress(ax, "tempo da fase concorrente (ms)")
ax.text(0.0, 1.045, "barra = média das 20 execuções   ·   haste = faixa mín.–máx.",
        transform=ax.transAxes, fontsize=7.6, color=MUTED)
fig.tight_layout(pad=0.6)
fig.savefig("relatorio/fig2-tempos.pdf"); fig.savefig("relatorio/fig2-tempos.png", dpi=300)
plt.close(fig)

# ------------------------------------------- fig 3: -O0 vs -O2 (múltiplos)
fig, axes = plt.subplots(1, 3, figsize=(6.0, 2.5), dpi=300)
for ax, m in zip(axes, MODES):
    vals = [O0[m]["tmean"], O2[m]["tmean"]]
    b = ax.bar([0, 1], vals, width=0.55, color=COLOR[m], zorder=3)
    b[1].set_alpha(0.42)
    b[1].set_hatch("////")
    b[1].set_edgecolor(COLOR[m])
    top = max(vals)
    for x, v in zip([0, 1], vals):
        ax.text(x, v + top * 0.05, br(v, 1), ha="center", va="bottom",
                fontsize=8.5, color=INK, fontweight="bold")
    ax.set_xticks([0, 1])
    ax.set_xticklabels(["-O0", "-O2"], fontsize=8.5, color=INK_2)
    ax.set_ylim(0, top * 1.32)
    ax.set_title(LABEL[m], fontsize=9, color=INK, pad=8)
    dress(ax)
axes[0].set_ylabel("tempo médio (ms)", fontsize=8.5, color=INK_2, labelpad=8)
fig.text(0.005, 0.965, "escalas independentes por painel — comparar apenas dentro de cada modo",
         fontsize=7.2, color=MUTED, va="top")
fig.tight_layout(pad=0.6, rect=(0, 0, 1, 0.93))
fig.savefig("relatorio/fig3-o0-o2.pdf"); fig.savefig("relatorio/fig3-o0-o2.png", dpi=300)
plt.close(fig)

print("figuras geradas")
