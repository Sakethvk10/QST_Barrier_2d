import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl

# ==========================
# User configuration
# ==========================
sites_to_plot = None
background_color = "white"
figure_size = (10, 5)
line_width = 2
show_legend = True
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
lines = open(filename).readlines()

# --------------------------
# Extract Nx Ny
# --------------------------
Nx = Ny = None
for line in lines:
    if "Nx" in line and "Ny" in line:
        parts = line.replace("\t", " ").split()
        Nx = int(parts[2])
        Ny = int(parts[5])
        break
if Nx is None or Ny is None:
    raise ValueError("Nx and Ny not found")

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
    raise ValueError("'time' table not found")

# --------------------------
# Load time + probability data
# --------------------------
data = np.array([[float(x) for x in line.split()] for line in lines[data_start:] if line.strip()])
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

    # Assign color from colormaps for first/last qubit
    if idx == 0:
        color = cmap_first(probs[:, site].max())  # first qubit
        label = "First qubit"
    elif idx == Ncols - 1:
        color = cmap_last(probs[:, site].max())   # last qubit
        label = "Last qubit"
    else:
        color = 'grey'
        label = f"Site {site}"

    ax.plot(time, probs[:, site], label=label,
            linewidth=line_width, linestyle=line_style,
            marker=marker_style, markersize=marker_size,
            markevery=marker_every, color=color)

ax.set_xlabel("Time")
ax.set_ylabel("Probability")

if show_legend:
    ax.legend()

# ==========================
# Two separate colorbars
# ==========================
sm_first = mpl.cm.ScalarMappable(cmap=cmap_first, norm=mpl.colors.Normalize(vmin=0, vmax=1))
sm_first.set_array([])
cbar_first = fig.colorbar(sm_first, ax=ax, pad=0.02)
cbar_first.set_label("First qubit probability", rotation=270, labelpad=15)

sm_last = mpl.cm.ScalarMappable(cmap=cmap_last, norm=mpl.colors.Normalize(vmin=0, vmax=1))
sm_last.set_array([])
cbar_last = fig.colorbar(sm_last, ax=ax, pad=0.12)
cbar_last.set_label("Last qubit probability", rotation=270, labelpad=15)

plt.tight_layout()
plt.show()

# ==========================
# Save as PDF
# ==========================
output_pdf = "2d_dynamics.pdf"
fig.savefig(output_pdf)
print(f"Saved figure with two colorbars as PDF: {output_pdf}")