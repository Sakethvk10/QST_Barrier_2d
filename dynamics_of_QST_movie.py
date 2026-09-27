#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import os
import re
import shutil
import sys

import matplotlib as mpl
import matplotlib.animation as animation
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection
from matplotlib.colors import Normalize, PowerNorm
from matplotlib.patches import Polygon
import numpy as np

# Try importing tqdm for terminal progress bar
try:
    from tqdm import tqdm
    HAS_TQDM = True
except ImportError:
    HAS_TQDM = False

# ------------------------------- Settings ------------------------------- #
OUTPUT_FILE = "animation_2d.mp4"
DEFAULT_FPS = 30

# Custom Range Adjustments (Set to None for automatic sizing)
# Example: X_MIN = -0.5, X_MAX = 6.5, Y_MIN = -0.5, Y_MAX = 6.5
X_MIN = None
X_MAX = None
Y_MIN = None
Y_MAX = None

# Plot aesthetics
NODE_CMAP = "magma"   # or viridis/cividis
EDGE_CMAP = "cool"
BARRIER_COLOR = "#1f1f1f"
FONT_SIZE = 18
TITLE_SIZE = 22
TICK_SIZE = 16
TIME_SIZE = 20

# UI Toggles
SHOW_TIME_TEXT = True
SHOW_BARRIER = True

plt.rcParams.update(
    {
        "text.usetex": False,  # Disabled to prevent missing LaTeX package errors
        "font.family": "serif",
        "font.size": FONT_SIZE,
        "axes.titlesize": TITLE_SIZE,
        "axes.labelsize": FONT_SIZE,
        "xtick.labelsize": TICK_SIZE,
        "ytick.labelsize": TICK_SIZE,
        "legend.fontsize": FONT_SIZE,
        "figure.titlesize": TITLE_SIZE,
    }
)
# ----------------------------------------------------------------------- #


def parse_dims(filename):
    with open(filename, "r") as f_in:
        for line in f_in:
            if not line.strip():
                continue
            match = re.search(r"Nx\s*=\s*(\d+)\s*Ny\s*=\s*(\d+)", line)
            if match:
                return int(match.group(1)), int(match.group(2))

    basename = os.path.basename(filename)
    match = re.search(r"Nx(\d+)_?Ny(\d+)", basename)
    if match:
        return int(match.group(1)), int(match.group(2))

    raise ValueError(f"Could not parse Nx and Ny from file {filename}")


def infer_barrier_width(filename):
    basename = os.path.basename(filename)
    match = re.search(r"(?:barrier_width|w)(\d+)", basename)
    if match:
        return int(match.group(1))

    try:
        with open(filename, "r") as f_in:
            for line in f_in:
                match = re.search(r"barrier_width\s*=\s*(\d+)", line)
                if match:
                    return int(match.group(1))
    except Exception:
        pass
    return 0


def parse_inversion_flag(filename):
    basename = os.path.basename(filename)
    match = re.search(r"inversion([01])", basename)
    if match:
        return match.group(1) == "1"

    try:
        with open(filename, "r") as f_in:
            for line in f_in:
                if "inversion" in line:
                    if "true" in line.lower() or "= 1" in line:
                        return True
                    elif "false" in line.lower() or "= 0" in line:
                        return False
    except Exception:
        pass

    return True


def read_probabilities(filename, nsites):
    header_idx = None
    with open(filename, "r") as f_in:
        for idx, line in enumerate(f_in):
            if line.startswith("time") or "time" in line:
                header_idx = idx
                break
    if header_idx is None:
        raise ValueError("Could not find time/probability header in data file")

    data = np.loadtxt(filename, skiprows=header_idx + 1)
    if data.ndim == 1:
        data = data[None, :]

    time = data[:, 0]
    prob_i = data[:, 1 : 1 + nsites]

    if prob_i.shape[1] != nsites:
        raise ValueError(f"Probability columns ({prob_i.shape[1]}) do not match Nx * Ny ({nsites})")
    return time, prob_i


def read_couplings(filename, n_couplings):
    values = []
    in_section = False
    with open(filename, "r") as f_in:
        for line in f_in:
            if "Converged couplings" in line:
                in_section = True
                continue
            if in_section and ("Converged fidelity" in line or "fidelity" in line):
                break
            if in_section:
                parts = line.split()
                if not parts:
                    continue
                try:
                    values.append(float(parts[-1]))
                except ValueError:
                    continue

    if len(values) >= 2 * n_couplings:
        jx_val = np.array(values[:n_couplings])
        jy_val = np.array(values[n_couplings : 2 * n_couplings])
    elif len(values) >= n_couplings:
        jx_val = np.array(values[:n_couplings])
        jy_val = jx_val.copy()
    else:
        print("[NOTE] Couplings section incomplete in file; generating placeholder values.")
        jx_val = np.ones(n_couplings)
        jy_val = np.ones(n_couplings)

    return jx_val, jy_val


def get_coords(nx, ny):
    xcoord = np.empty(nx * ny, dtype=int)
    ycoord = np.empty(nx * ny, dtype=int)
    for i in range(nx * ny):
        xcoord[i] = i % nx
        ycoord[i] = i // nx
    return xcoord, ycoord


def indexsetsq(nx, ny):
    nsites = nx * ny
    xplus = np.full(nsites, -1, dtype=int)
    xminus = np.full(nsites, -1, dtype=int)
    yplus = np.full(nsites, -1, dtype=int)
    yminus = np.full(nsites, -1, dtype=int)

    for i in range(nsites):
        x = i % nx
        y = i // nx

        if x + 1 < nx:
            xplus[i] = i + 1
        if x - 1 >= 0:
            xminus[i] = i - 1
        if y + 1 < ny:
            yplus[i] = i + nx
        if y - 1 >= 0:
            yminus[i] = i - nx

    return xplus, yplus, xminus, yminus


def build_segments_inversion(nx, ny, xcoord, ycoord, jx_val, jy_val):
    nsites = nx * ny
    xplus, yplus, xminus, yminus = indexsetsq(nx, ny)
    segments = []
    values = []

    idx = 0
    idy = 0
    for i in range(nsites // 2):
        if idx < len(jx_val):
            j = xplus[i]
            if j != -1:
                segments.append([(xcoord[i], ycoord[i]), (xcoord[j], ycoord[j])])
                values.append(abs(jx_val[idx]))

                i_inv = nsites - 1 - i
                j_inv = xminus[i_inv]
                if j_inv != -1:
                    segments.append([(xcoord[i_inv], ycoord[i_inv]), (xcoord[j_inv], ycoord[j_inv])])
                    values.append(abs(jx_val[idx]))
                idx += 1

        if idy < len(jy_val):
            j = yplus[i]
            if j != -1:
                segments.append([(xcoord[i], ycoord[i]), (xcoord[j], ycoord[j])])
                values.append(abs(jy_val[idy]))

                i_inv = nsites - 1 - i
                j_inv = yminus[i_inv]
                if j_inv != -1:
                    segments.append([(xcoord[i_inv], ycoord[i_inv]), (xcoord[j_inv], ycoord[j_inv])])
                    values.append(abs(jy_val[idy]))
                idy += 1

    return segments, np.array(values)


def build_segments_full(nx, ny, xcoord, ycoord, jx_val, jy_val):
    nsites = nx * ny
    xplus, yplus, _, _ = indexsetsq(nx, ny)
    segments = []
    values = []

    idx = 0
    idy = 0
    for i in range(nsites):
        j = xplus[i]
        if j != -1:
            if idx >= len(jx_val):
                break
            segments.append([(xcoord[i], ycoord[i]), (xcoord[j], ycoord[j])])
            values.append(abs(jx_val[idx]))
            idx += 1

        j = yplus[i]
        if j != -1:
            if idy >= len(jy_val):
                break
            segments.append([(xcoord[i], ycoord[i]), (xcoord[j], ycoord[j])])
            values.append(abs(jy_val[idy]))
            idy += 1

    return segments, np.array(values)


def add_barrier_patch(ax, nx, ny, width, pad):
    if width <= 0:
        return

    diag = nx - 1
    half_w = 0.5 * width
    xmin, xmax = -pad, nx - 1 + pad
    ymin, ymax = -pad, ny - 1 + pad

    def line_box_intersections(c):
        points = []
        y = c - xmin
        if ymin <= y <= ymax:
            points.append((xmin, y))
        y = c - xmax
        if ymin <= y <= ymax:
            points.append((xmax, y))
        x = c - ymin
        if xmin <= x <= xmax:
            points.append((x, ymin))
        x = c - ymax
        if xmin <= x <= xmax:
            points.append((x, ymax))

        unique = []
        for point in points:
            if not any(np.allclose(point, other) for other in unique):
                unique.append(point)
        if len(unique) < 2:
            return None
        if len(unique) > 2:
            unique = sorted(unique, key=lambda p: (p[0], p[1]))
            unique = [unique[0], unique[-1]]
        return unique

    lower = line_box_intersections(diag - half_w)
    upper = line_box_intersections(diag + half_w)
    if not lower or not upper:
        return

    lower = sorted(lower, key=lambda p: (p[0], p[1]))
    upper = sorted(upper, key=lambda p: (p[0], p[1]))
    polygon = Polygon(
        [lower[0], lower[1], upper[1], upper[0]],
        closed=True,
        facecolor=BARRIER_COLOR,
        edgecolor="none",
        alpha=0.18,
        zorder=0,
    )
    ax.add_patch(polygon)


def main():
    parser = argparse.ArgumentParser(description="Animate 2D lattice quantum probabilities.")
    parser.add_argument("file", help="Path to .out data file")
    args = parser.parse_args()

    filename = args.file
    nx, ny = parse_dims(filename)
    nsites = nx * ny
    inversion_flag = parse_inversion_flag(filename)
    layout = "inversion" if inversion_flag else "full"

    if inversion_flag:
        n_couplings = ((nx - 1) * ny) // 2
    else:
        n_couplings = (nx - 1) * ny

    time, prob_i = read_probabilities(filename, nsites)
    num_frames = len(time)
    jx_val, jy_val = read_couplings(filename, n_couplings)

    xcoord, ycoord = get_coords(nx, ny)

    if layout == "inversion":
        segments, edge_values = build_segments_inversion(nx, ny, xcoord, ycoord, jx_val, jy_val)
    else:
        segments, edge_values = build_segments_full(nx, ny, xcoord, ycoord, jx_val, jy_val)

    fig = plt.figure(figsize=(10, 10))
    gs = fig.add_gridspec(
        2,
        2,
        width_ratios=[1.0, 0.06],
        height_ratios=[1.0, 0.06],
        wspace=0.08,
        hspace=0.2,
    )
    ax = fig.add_subplot(gs[0, 0])
    cax_prob = fig.add_subplot(gs[0, 1])
    cax_coup = fig.add_subplot(gs[1, 0])
    fig.add_subplot(gs[1, 1]).axis("off")

    ax.set_aspect("equal")
    pad = 0.6

    # Determine scale ranges
    xlim = (
        X_MIN if X_MIN is not None else -pad,
        X_MAX if X_MAX is not None else nx - 1 + pad,
    )
    ylim = (
        Y_MIN if Y_MIN is not None else -pad,
        Y_MAX if Y_MAX is not None else ny - 1 + pad,
    )

    ax.set_xlim(xlim)
    ax.set_ylim(ylim)

    ax.set_xticks(range(nx))
    ax.set_yticks(range(ny))
    ax.set_xticklabels(range(1, nx + 1))
    ax.set_yticklabels(range(1, ny + 1))

    if len(edge_values) > 0:
        edge_values = np.asarray(edge_values)
        edge_norm = Normalize(vmin=edge_values.min(), vmax=edge_values.max())
        line_collection = LineCollection(
            segments,
            cmap=EDGE_CMAP,
            norm=edge_norm,
            linewidths=1.8,
            alpha=0.9,
            zorder=1,
        )
        line_collection.set_array(edge_values)
        ax.add_collection(line_collection)
        cbar_coup = fig.colorbar(line_collection, cax=cax_coup, orientation="horizontal")
        cbar_coup.set_label(r"$|J_{ij}|$")

    node_size = 800.0 * (10.0 / max(nx, ny)) ** 2
    node_size = max(120.0, min(1200.0, node_size))

    prob_norm = PowerNorm(gamma=0.30, vmin=0.0, vmax=1.0)

    nodes = ax.scatter(
        xcoord,
        ycoord,
        c=prob_i[0],
        s=node_size,
        cmap=NODE_CMAP,
        norm=prob_norm,
        edgecolors="#222222",
        linewidths=0.8,
        zorder=2,
    )

    if SHOW_BARRIER:
        barrier_width = infer_barrier_width(filename)
        add_barrier_patch(ax, nx, ny, barrier_width, pad)

    cbar_prob = fig.colorbar(nodes, cax=cax_prob)
    cbar_prob.set_label(r"$|\psi_i|^2$")

    time_text = ax.text(
        0.02,
        1.02,
        "",
        transform=ax.transAxes,
        fontsize=TIME_SIZE,
        ha="left",
        va="bottom",
    )

    # Setup terminal progress bar
    if HAS_TQDM:
        pbar = tqdm(total=num_frames, desc="Rendering Frames", unit="frame")
    else:
        pbar = None

    def update(frame):
        nodes.set_array(prob_i[frame])
        if SHOW_TIME_TEXT:
            time_text.set_text(r"$t = %.4f$" % time[frame])

        if pbar is not None:
            pbar.update(1)
        elif frame % max(1, num_frames // 10) == 0:
            print(f"Rendering frame {frame}/{num_frames}...")

        return nodes, time_text

    anim = animation.FuncAnimation(
        fig,
        update,
        frames=num_frames,
        interval=int(1000 / DEFAULT_FPS),
        blit=False,
    )

    print(f"[INFO] Rendering 2D lattice animation ({num_frames} frames)...")

    # Windows/WSL compatible MP4 encoding settings
    writer = animation.FFMpegWriter(
        fps=DEFAULT_FPS,
        codec="libx264",
        extra_args=["-pix_fmt", "yuv420p"],
        bitrate=-1,
    )

    try:
        anim.save(OUTPUT_FILE, writer=writer)
        print(f"\n[SUCCESS] Saved 2D movie to {OUTPUT_FILE}")
    except Exception as e:
        print(f"\n[WARN] FFmpeg writer failed ({e}). Falling back to GIF output...")
        gif_out = "animation_2d.gif"
        anim.save(gif_out, writer="pillow", fps=DEFAULT_FPS)
        print(f"[SUCCESS] Saved fallback animation to {gif_out}")
    finally:
        if pbar is not None:
            pbar.close()


if __name__ == "__main__":
    main()