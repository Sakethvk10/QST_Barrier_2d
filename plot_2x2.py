import glob
import os
import re
import sys
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

######################## Plotting Setup ####################################
plt.rc("text", usetex=False)
plt.rc("font", family="serif")
mpl.rcParams["xtick.direction"] = "in"
mpl.rcParams["ytick.direction"] = "in"
mpl.rcParams["xtick.top"] = True
mpl.rcParams["ytick.right"] = True

txt_size = 22
label_size = 22


def extract_file_data(filename):
    """Extracts Nx, Ny, and converged fidelity from a .out or text file."""
    if not os.path.isfile(filename):
        return None, None, None

    nx, ny, fidelity = None, None, None

    try:
        with open(filename, "r") as f:
            for line in f:
                # Extract Nx and Ny
                grid_match = re.search(r"Nx\s*=\s*(\d+)\s+Ny\s*=\s*(\d+)", line)
                if grid_match:
                    nx, ny = int(grid_match.group(1)), int(grid_match.group(2))

                # Extract Converged fidelity (handles both 'fidelity' and 'fidelity_QST')
                fid_match = re.search(
                    r"Converged\s+fidelity(?:_QST)?\s*=\s*([0-9.eE+-]+)", line
                )
                if fid_match:
                    try:
                        fidelity = float(fid_match.group(1))
                    except ValueError:
                        pass

                if nx is not None and ny is not None and fidelity is not None:
                    break
    except Exception as e:
        print(f"[WARN] Error reading {filename}: {e}")
        return None, None, None

    return nx, ny, fidelity


def parse_folder_or_filename(name):
    """Extracts barrier height ('h') and width ('w') from names like 'Nx5Ny5h10w2'
    or filenames containing 'h10w2'.
    """
    match = re.search(r"h(-?\d+\.?\d*)w(-?\d+\.?\d*)", name)
    if not match:
        return None, None
    return float(match.group(1)), float(match.group(2))


def plot_fidelities_2d(target_nx, target_ny):
    """Creates fidelity maps for specified target Nx and Ny dimensions."""
    grid_configs = [(target_nx, target_ny)]

    nrows = len(grid_configs)
    ncols = 1

    fig, axs = plt.subplots(
        nrows, ncols, figsize=(8, 7), constrained_layout=True, squeeze=False
    )
    axs = axs.flatten()

    vmin_global, vmax_global = np.inf, -np.inf
    pcm_list = []

    for idx, (nx, ny) in enumerate(grid_configs):
        ax = axs[idx]
        title = rf"$N_x = {nx},\ N_y = {ny}$"

        # 1. Search folders matching the exact prefix results_2d/Nx{nx}Ny{ny}h*
        folder_prefix = f"results_2d/Nx{nx}Ny{ny}h*"
        folders = sorted(
            [f for f in glob.glob(folder_prefix) if os.path.isdir(f)]
        )

        # 2. Search loose files matching Nx{nx}Ny{ny} in their filenames or current directory
        loose_pattern = f"*Nx{nx}Ny{ny}*.out"
        out_files_direct = glob.glob(os.path.join(".", loose_pattern)) + \
                           glob.glob("*.out") + glob.glob("*.txt")
        # Remove duplicates
        out_files_direct = list(set(out_files_direct))

        print(
            f"[INFO] Processing Nx={nx}, Ny={ny}: found {len(folders)} folders and checking {len(out_files_direct)} candidate loose files."
        )

        heights_all, widths_all, data = set(), set(), []

        # --- Collect data from matched folders ---
        for folder in folders:
            folder_name = os.path.basename(folder)
            height, width = parse_folder_or_filename(folder_name)

            out_files = glob.glob(os.path.join(folder, "*.out")) + glob.glob(
                os.path.join(folder, "*.txt")
            )
            for fname in out_files:
                if height is None or width is None:
                    h_val, w_val = parse_folder_or_filename(
                        os.path.basename(fname)
                    )
                else:
                    h_val, w_val = height, width

                file_nx, file_ny, fidelity = extract_file_data(fname)
                
                # Strict check: ensure internal file dimensions match target Nx, Ny (if present)
                if (file_nx is not None and file_nx != nx) or (file_ny is not None and file_ny != ny):
                    continue

                if fidelity is not None and h_val is not None and w_val is not None:
                    heights_all.add(h_val)
                    widths_all.add(w_val)
                    data.append((h_val, w_val, fidelity))

        # --- Collect data from loose files with strict filtering ---
        for fname in out_files_direct:
            h_val, w_val = parse_folder_or_filename(os.path.basename(fname))
            file_nx, file_ny, fidelity = extract_file_data(fname)

            # Strict Filter: Only accept file if internal Nx and Ny match target (or filename specifies Nx and Ny)
            if file_nx is not None and file_ny is not None:
                if file_nx != nx or file_ny != ny:
                    continue
            else:
                # If content doesn't specify Nx/Ny, check if filename explicitly specifies a different grid size
                fn_match = re.search(r"Nx(\d+)Ny(\d+)", os.path.basename(fname))
                if fn_match:
                    if int(fn_match.group(1)) != nx or int(fn_match.group(2)) != ny:
                        continue

            if fidelity is not None and h_val is not None and w_val is not None:
                heights_all.add(h_val)
                widths_all.add(w_val)
                data.append((h_val, w_val, fidelity))

        if not data:
            print(f"[WARN] No matching data found for Nx={nx}, Ny={ny}.")
            ax.set_title(title + " (no matching data found)")
            out_filename = f"fidelity_map_Nx{nx}_Ny{ny}.pdf"
            plt.savefig(out_filename, dpi=300, format="pdf")
            plt.close()
            return

        # Convert to grids
        unique_heights = np.sort(list(heights_all))
        unique_widths = np.sort(list(widths_all))

        if len(unique_heights) == 1:
            unique_heights = np.array(
                [unique_heights[0] - 0.5, unique_heights[0] + 0.5]
            )
        if len(unique_widths) == 1:
            unique_widths = np.array(
                [unique_widths[0] - 0.5, unique_widths[0] + 0.5]
            )

        H, W = np.meshgrid(unique_heights, unique_widths, indexing="ij")
        F = np.zeros(H.shape)

        for h, w, f in data:
            i = np.abs(unique_heights - h).argmin()
            j = np.abs(unique_widths - w).argmin()
            F[i, j] = f

        # --- Plotting ---
        pcm = ax.pcolormesh(W, H, F, shading="auto", cmap="viridis", vmin=0.9, vmax=1)
        pcm_list.append(pcm)
        vmin_global = min(vmin_global, F.min())
        vmax_global = max(vmax_global, F.max())

        panel_label = rf"(a)\ {title}"
        ax.text(
            0.04,
            0.94,
            panel_label,
            transform=ax.transAxes,
            fontsize=16,
            ha="left",
            va="top",
            color="white",
        )

        ax.set_xlabel(r"${\rm Barrier\ width}\ (w)$", fontsize=label_size)
        ax.set_ylabel(r"${\rm Barrier\ height}\ (h)$", fontsize=label_size)
        ax.tick_params(axis="both", which="major", labelsize=txt_size)

    # Apply color scale
    if pcm_list:
        cbar = fig.colorbar(
            pcm_list[-1],
            ax=axs,
            location="right",
            fraction=0.046,
            pad=0.04,
        )
        cbar.set_label(
            r"${\rm Converged\ fidelity}_{\rm QST}$", fontsize=label_size
        )
        cbar.ax.tick_params(labelsize=txt_size)

    out_filename = f"fidelity_map_Nx{target_nx}_Ny{target_ny}.pdf"
    plt.savefig(out_filename, dpi=300, format="pdf")
    plt.close()
    print(f"[INFO] Successfully saved plot to {out_filename}")


if __name__ == "__main__":
    # Get Nx and Ny from command-line arguments or prompt user
    if len(sys.argv) >= 3:
        target_nx = int(sys.argv[1])
        target_ny = int(sys.argv[2])
    else:
        try:
            target_nx = int(input("Enter Nx: "))
            target_ny = int(input("Enter Ny: "))
        except ValueError:
            print("[ERROR] Please enter valid integer values for Nx and Ny.")
            sys.exit(1)

    plot_fidelities_2d(target_nx, target_ny)