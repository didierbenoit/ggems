# *****************************************************************************
# * This file is part of GGEMS.                                               *
# *                                                                           *
# * SPDX-License-Identifier: GPL-3.0-or-later                                 *
# * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
# * Inserm.                                                                   *
# *                                                                           *
# * GGEMS is free software: you can redistribute it and/or modify             *
# * it under the terms of the GNU General Public License as published by      *
# * the Free Software Foundation, either version 3 of the License, or         *
# * (at your option) any later version.                                       *
# *                                                                           *
# * GGEMS is distributed in the hope that it will be useful,                  *
# * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
# * GNU General Public License for more details.                              *
# *                                                                           *
# * You should have received a copy of the GNU General Public License         *
# * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
# *****************************************************************************

"""Run the GGEMS Source time validation campaign.

Authors:
    Julien BERT <julien.bert@univ-brest.fr>
    Didier BENOIT <didier.benoit@inserm.fr>
"""

import argparse
import json
import subprocess
from pathlib import Path

# Direct script entry points: Python adds this directory to sys.path.
from cases import (
    CASES,
    DEFAULT_PRIMARIES,
    DEFAULT_SEED,
    DEFAULT_WORKERS,
)


def nonnegative_integer(value: str) -> int:
    number = int(value)
    if number < 0:
        raise argparse.ArgumentTypeError("Expected a nonnegative integer.")

    return number


def positive_integer(value: str) -> int:
    number = nonnegative_integer(value)
    if number == 0:
        raise argparse.ArgumentTypeError("Expected a positive integer.")

    return number


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run exact T1 CountDriven chronology cases through production GGEMS Source."
    )
    parser.add_argument("--exporter", type=Path, required=True)
    parser.add_argument(
        "--device", required=True, help="Forwarded unchanged to GGEMS SelectDevices."
    )
    parser.add_argument(
        "--cases",
        nargs="+",
        choices=[case.name for case in CASES],
        default=[case.name for case in CASES],
    )
    parser.add_argument(
        "--primaries",
        type=positive_integer,
        default=DEFAULT_PRIMARIES,
        help="Primaries per logical Run; 256 is sufficient for exact time equality.",
    )
    parser.add_argument("--workers", type=positive_integer, default=DEFAULT_WORKERS)
    parser.add_argument("--seed", type=nonnegative_integer, default=DEFAULT_SEED)
    parser.add_argument(
        "--no-plots", action="store_true", help="Use only the Python standard library."
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "results" / "time",
    )
    return parser.parse_args()


def checkout_commit() -> str | None:
    try:
        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            cwd=Path(__file__).resolve().parents[3],
            check=True,
            capture_output=True,
            text=True,
            encoding="utf-8",
        )
    except (OSError, subprocess.CalledProcessError):
        return None

    return result.stdout.strip() or None


def main() -> int:
    args = parse_arguments()
    from analyze import analyze_case

    exporter = args.exporter.resolve()
    if not exporter.is_file():
        raise FileNotFoundError(f"Source sample exporter not found: {exporter}")

    output_root = args.output_dir.resolve()
    commit = checkout_commit()

    for case in CASES:
        if case.name not in args.cases:
            continue

        # One exporter process per CASE; repeated Runs remain inside the same
        # initialized GGEMSRun object. A new directory excludes stale results.
        output = output_root / case.name
        output.mkdir(parents=True, exist_ok=False)
        metadata_path = output / "metadata.json"
        command = [
            str(exporter),
            "--device",
            args.device,
            "--geometry",
            "point",
            "--angular",
            "fixed",
            "--energy-mode",
            "mono",
            "--mono-energy-kev",
            "511",
            "--case-name",
            case.name,
            "--chronology",
            case.chronology,
            "--primaries",
            str(args.primaries),
            "--workers",
            str(args.workers),
            "--seed",
            str(args.seed),
            "--sequence-dir",
            str(output / "runs"),
            "--sequence-runs",
            str(len(case.windows_ps)),
            "--metadata",
            str(metadata_path),
        ]
        if case.requested_ns is not None:
            start, stop, step = case.requested_ns
            command.extend(
                [
                    "--time-start-ns",
                    str(start),
                    "--time-stop-ns",
                    str(stop),
                    "--time-step-ns",
                    str(step),
                ]
            )

        if case.reset_before_run is not None:
            command.extend(["--reset-before-run", str(case.reset_before_run)])

        print(
            f"{case.name}: {len(case.windows_ps)} logical Run(s), "
            + f"{args.primaries} Source primaries per Run on {args.device}",
            flush=True,
        )
        with (output / "export.log").open("w", encoding="utf-8") as log:
            try:
                subprocess.run(
                    command,
                    cwd=output,
                    stdout=log,
                    stderr=subprocess.STDOUT,
                    check=True,
                )
            except (OSError, subprocess.CalledProcessError) as error:
                raise RuntimeError(
                    f"{case.name}: exporter failed; see {output / 'export.log'}"
                ) from error

        if commit is not None:
            raw = json.loads(metadata_path.read_text(encoding="utf-8"))
            if not isinstance(raw, dict):
                raise TypeError(f"Expected an object in {metadata_path}.")
            metadata = raw
            metadata["git_commit"] = commit
            metadata_path.write_text(
                json.dumps(metadata, indent=2, allow_nan=False) + "\n", encoding="utf-8"
            )

        analyze_case(metadata_path, output, make_figures=not args.no_plots)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
