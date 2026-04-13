import argparse
import os
import re

import matplotlib
import numpy as np

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def parse_flags_log(log_path):
    rows = []
    current = None

    with open(log_path, "r", encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()

            m_run = re.match(r"RUN flag=(O\d) prog=(\w+) threads=(\d+)", line)
            if m_run:
                current = (m_run.group(1), m_run.group(2), int(m_run.group(3)))
                continue

            m_time = re.match(r"sum:\s*(\d+),\s*time:\s*([0-9.]+)\s*seconds", line)
            if m_time and current:
                flag, prog, threads = current
                rows.append(
                    {
                        "flag": flag,
                        "program": prog,
                        "threads": threads,
                        "sum": int(m_time.group(1)),
                        "time": float(m_time.group(2)),
                    }
                )

    return rows


def parse_default_log(log_path):
    rows = []
    current = None

    with open(log_path, "r", encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()

            m_run = re.match(r"RUN prog=(\w+) threads=(\d+)", line)
            if m_run:
                current = (m_run.group(1), int(m_run.group(2)))
                continue

            m_time = re.match(r"sum:\s*(\d+),\s*time:\s*([0-9.]+)\s*seconds", line)
            if m_time and current:
                prog, threads = current
                rows.append(
                    {
                        "program": prog,
                        "threads": threads,
                        "sum": int(m_time.group(1)),
                        "time": float(m_time.group(2)),
                    }
                )

    return rows


def parse_repeated_log(log_path):
    """Parse repeated-run log: RUN flag=O0 prog=slow threads=1 rep=1 / time: ..."""
    rows = []
    current = None

    with open(log_path, "r", encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()

            m_run = re.match(
                r"RUN flag=(O\d) prog=(\w+) threads=(\d+) rep=(\d+)",
                line,
            )
            if m_run:
                current = (
                    m_run.group(1),
                    m_run.group(2),
                    int(m_run.group(3)),
                    int(m_run.group(4) or 1),
                )
                continue

            m_time = re.match(r"sum:\s*(\d+),\s*time:\s*([0-9.]+)\s*seconds", line)
            if m_time and current:
                flag, prog, threads, rep = current
                rows.append(
                    {
                        "flag": flag,
                        "program": prog,
                        "threads": threads,
                        "rep": rep,
                        "time": float(m_time.group(2)),
                    }
                )

    return rows


def get_flags_time(rows, flag, prog, threads):
    for row in rows:
        if row["flag"] == flag and row["program"] == prog and row["threads"] == threads:
            return row["time"]
    raise ValueError(f"Missing entry for flag={flag}, program={prog}, threads={threads}")


def make_default_runtime_vs_threads(rows, programs, colors, out_path):
    fig, ax = plt.subplots(figsize=(8, 4.5))
    for prog in programs:
        series = sorted(
            [r for r in rows if r["program"] == prog],
            key=lambda r: r["threads"],
        )
        ax.plot(
            [r["threads"] for r in series],
            [r["time"] for r in series],
            marker="o",
            label=prog,
            color=colors[prog],
        )

    ax.set_xlabel("Threads (OMP_NUM_THREADS)")
    ax.set_ylabel("Time (s)")
    ax.set_title("Default Compile: Runtime vs Threads")
    ax.legend()
    ax.grid(alpha=0.25)
    fig.tight_layout()
    fig.savefig(out_path, dpi=160)
    plt.close(fig)


def make_flags_runtime_vs_threads(rows, flags, programs, flag_colors, out_path):
    fig, axes = plt.subplots(1, 3, figsize=(12, 4), sharex=True)

    for ax, prog in zip(axes, programs):
        for flag in flags:
            series = sorted(
                [r for r in rows if r["program"] == prog and r["flag"] == flag],
                key=lambda r: r["threads"],
            )
            ax.plot(
                [r["threads"] for r in series],
                [r["time"] for r in series],
                marker="o",
                label=flag,
                color=flag_colors[flag],
            )
        ax.set_title(prog)
        ax.set_xlabel("Threads")
        ax.grid(alpha=0.25)

    axes[0].set_ylabel("Time (s)")
    handles, labels = axes[-1].get_legend_handles_labels()
    fig.legend(handles, labels, loc="upper center", ncol=4, bbox_to_anchor=(0.5, 1.02))
    fig.suptitle("Optimization Flags: Runtime vs Threads", y=1.08)
    fig.tight_layout()
    fig.savefig(out_path, dpi=160, bbox_inches="tight")
    plt.close(fig)


def make_stability_illustration(rows, flag, programs, colors, out_path):
    """Runtime vs threads with error bars (std across reps) — illustrates stability."""
    fig, ax = plt.subplots(figsize=(8, 5))
    for prog in programs:
        subset = [r for r in rows if r["flag"] == flag and r["program"] == prog]
        by_threads = {}
        for r in subset:
            t = r["threads"]
            if t not in by_threads:
                by_threads[t] = []
            by_threads[t].append(r["time"])
        threads = sorted(by_threads.keys())
        means = [np.mean(by_threads[t]) for t in threads]
        stds = [np.std(by_threads[t]) for t in threads]
        ax.errorbar(
            threads,
            means,
            yerr=stds,
            marker="o",
            capsize=3,
            label=prog,
            color=colors[prog],
        )
    ax.set_xlabel("Threads (OMP_NUM_THREADS)")
    ax.set_ylabel("Time (s)")
    ax.set_title(
        f"Stability: Runtime vs Threads (O0, 5 reps) — error bars = std dev"
    )
    ax.legend()
    ax.grid(alpha=0.25)
    fig.tight_layout()
    fig.savefig(out_path, dpi=160, bbox_inches="tight")
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser(
        description="Generate default and flags runtime figures for Exercise 1"
    )
    parser.add_argument(
        "--default-log",
        default="results_default.log",
        help="Path to default run log (RUN prog=... threads=...)",
    )
    parser.add_argument(
        "--flags-log",
        default="results_flags.log",
        help="Path to flags run log (RUN flag=... prog=... threads=...)",
    )
    parser.add_argument(
        "--repeated-log",
        default="results_flags_repeated.log",
        help="Path to repeated-run log (RUN flag=... prog=... threads=... rep=...)",
    )
    parser.add_argument(
        "--out-dir",
        default="../figures",
        help="Output directory for PNG figures",
    )
    args = parser.parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    default_log = (
        args.default_log
        if os.path.isabs(args.default_log)
        else os.path.join(script_dir, args.default_log)
    )
    flags_log = (
        args.flags_log
        if os.path.isabs(args.flags_log)
        else os.path.join(script_dir, args.flags_log)
    )
    repeated_log = (
        args.repeated_log
        if os.path.isabs(args.repeated_log)
        else os.path.join(script_dir, args.repeated_log)
    )
    out_dir = args.out_dir if os.path.isabs(args.out_dir) else os.path.join(script_dir, args.out_dir)

    default_rows = parse_default_log(default_log)
    flags_rows = parse_flags_log(flags_log)
    os.makedirs(out_dir, exist_ok=True)

    flags = ["O0", "O1", "O2", "O3"]
    programs = ["slow", "medium", "fast"]
    program_colors = {"slow": "#1f77b4", "medium": "#ff7f0e", "fast": "#2ca02c"}
    flag_colors = {"O0": "#1f77b4", "O1": "#ff7f0e", "O2": "#2ca02c", "O3": "#d62728"}

    make_default_runtime_vs_threads(
        default_rows,
        programs,
        program_colors,
        out_path=os.path.join(out_dir, "ex1_default_runtime_vs_threads.png"),
    )
    make_flags_runtime_vs_threads(
        flags_rows,
        flags,
        programs,
        flag_colors,
        out_path=os.path.join(out_dir, "ex1_flags_runtime_vs_threads.png"),
    )

    if os.path.isfile(repeated_log):
        repeated_rows = parse_repeated_log(repeated_log)
        make_stability_illustration(
            repeated_rows,
            "O0",
            programs,
            program_colors,
            out_path=os.path.join(out_dir, "ex1_stability_illustration.png"),
        )

    print("Generated:")
    print(os.path.join(out_dir, "ex1_default_runtime_vs_threads.png"))
    print(os.path.join(out_dir, "ex1_flags_runtime_vs_threads.png"))
    if os.path.isfile(repeated_log):
        print(os.path.join(out_dir, "ex1_stability_illustration.png"))


if __name__ == "__main__":
    main()
