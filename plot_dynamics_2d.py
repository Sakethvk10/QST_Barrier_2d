import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl

# ==========================
# User configuration
# ==========================
sites_to_plot = None  # None plots all columns; or set a list like [0, 12, 24]
background_color = "white"
figure_size = (10, 5)
line_width = 2
show_legend = False
line_styles = ["solid", "dashed", "dotted", "dashdot"]
marker_styles = ["", "", "", ""]
marker_size = 6
marker_every = 50

# Colormaps for first and last qubit
cmap_first = plt.get_cmap("Blues")
cmap_last = plt.get_cmap("Reds")

# ==========================
# Load file
# ==========================
if len(sys.argv) < 2:
    print("Usage: python3 plot_dynamics_2d.py <filename>")
    sys.exit(1)

filename = sys.argv[1]
with open(filename, "r") as f:
    lines = f.readlines()

# --------------------------
# Extract Nx Ny
# --------------------------
Nx = Ny = None
for line in lines:
    if "Nx" in line and "Ny" in line:
        # Standardize whitespace separators
        parts = line.replace("\t", " ").split()
        for i, part in enumerate(parts):
            if "Nx" in part and i + 2 < len(parts):
                Nx = int(parts[i + 2])
            if "Ny" in part and i + 2 < len(parts):
                Ny = int(parts[i + 2])
        break

if Nx is None or Ny is None:
    raise ValueError("Nx and Ny parameters were not found in the input file.")

Nsites = Nx * Ny

# --------------------------
# Locate time table
# --------------------------
data_start = None
for i, line in enumerate(lines):
    if line.strip().startswith("time"):
        data_start = i + 1
        break

if data_start is None:
    raise ValueError("'time' header row was not found in the input file.")

# --------------------------
# Load time + probability data
# --------------------------
parsed_rows = []
for line in lines[data_start:]:
    cleaned_line = line.strip()
    if cleaned_line:
        # Split across any combination of tabs and spaces
        tokens = cleaned_line.split()
        parsed_rows.append([float(x) for x in tokens])

data = np.array(parsed_rows)
time = data[:, 0]
probs = data[:, 1:]
Ncols = probs.shape[1]

if sites_to_plot is None:
    sites_to_plot = list(range(Ncols))

# ==========================
# Plot
# ==========================
fig, ax = plt.subplots(figsize=figure_size, facecolor=background_color)
ax.set_facecolor(background_color)

for idx, site in enumerate(sites_to_plot):
    if site < 0 or site >= Ncols:
        continue

    line_style = line_styles[idx % len(line_styles)]
    marker_style = marker_styles[idx % len(marker_styles)]

    # Assign color from colormaps dynamically for first / last qubit
    if site == 0:
        color = cmap_first(probs[:, site].max())
        label = f"First qubit (Site {site})"
    elif site == Ncols - 1:
        color = cmap_last(probs[:, site].max())
        label = f"Last qubit (Site {site})"
    else:
        color = "grey"
        label = f"Site {site}"

    ax.plot(
        time,
        probs[:, site],
        label=label,
        linewidth=line_width,
        linestyle=line_style,
        marker=marker_style,
        markersize=marker_size,
        markevery=marker_every,
        color=color,
    )

ax.set_xlabel("Time")
ax.set_ylabel("Probability")

if show_legend:
    ax.legend(bbox_to_anchor=(1.05, 1), loc="upper right")

# ==========================
# Two separate colorbars
# ==========================
sm_first = mpl.cm.ScalarMappable(
    cmap=cmap_first, norm=mpl.colors.Normalize(vmin=0, vmax=1)
)
sm_first.set_array([])
cbar_first = fig.colorbar(sm_first, ax=ax, pad=0.18)
cbar_first.set_label("First qubit probability", rotation=270, labelpad=15)

sm_last = mpl.cm.ScalarMappable(
    cmap=cmap_last, norm=mpl.colors.Normalize(vmin=0, vmax=1)
)
sm_last.set_array([])
cbar_last = fig.colorbar(sm_last, ax=ax, pad=0.08)
cbar_last.set_label("Last qubit probability", rotation=270, labelpad=15)

plt.tight_layout()
plt.show()

# ==========================
# Save as PDF
# ==========================
output_pdf = "2d_dynamics.pdf"
fig.savefig(output_pdf, bbox_inches="tight")
print(f"Saved figure with two colorbars as PDF: {output_pdf}")
