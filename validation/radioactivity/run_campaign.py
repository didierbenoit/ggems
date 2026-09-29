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

# Authors:
#     Julien BERT <julien.bert@univ-brest.fr>
#     Didier BENOIT <didier.benoit@inserm.fr>

"""Run and analyze a radionuclide campaign with activity chosen from a sample target."""

import argparse
import importlib.metadata
import platform
import subprocess
from decimal import Decimal
from pathlib import Path

from radionuclide_validation.analysis import analyze_campaign
from radionuclide_validation.model import Reference, Runtime, write_json
from radionuclide_validation.numerics import design_campaign


def invoke(command: list[str], log_path: Path, timeout_seconds: int) -> None:
    with log_path.open("w", encoding="utf-8", newline="\n") as log:
        log.write(subprocess.list2cmdline(command) + "\n")
        log.flush()
        subprocess.run(
            command,
            stdout=log,
            stderr=subprocess.STDOUT,
            check=True,
            timeout=timeout_seconds,
        )


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        epilog="Results distinguish nuclear data, decay counts, populations, birth times and energy spectra. Read analysis.json for scientific conclusions.",
    )
    parser.add_argument(
        "--exporter",
        type=Path,
        required=True,
        help="Path to the built ggems_radionuclide_exporter executable.",
    )
    parser.add_argument(
        "--reference",
        type=Path,
        required=True,
        help="Selected radionuclide reference.json, including emission yields and spectrum provenance.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        required=True,
        help="New directory for samples, analysis, settings and execution logs.",
    )
    parser.add_argument(
        "--device",
        default="cpu",
        help="GGEMS OpenCL selection: cpu, gpu, all, vendor, or device indices/ranges. Passed to GGEMS unchanged.",
    )
    parser.add_argument(
        "--workers",
        type=int,
        default=256,
        help="Persistent random-stream workers per selected OpenCL device.",
    )
    parser.add_argument(
        "--seed",
        type=int,
        default=77777,
        help="Philox seed for the campaign; host population replicas use seed+1, seed+2, etc.",
    )
    parser.add_argument(
        "--windows",
        type=int,
        default=32,
        help="Number of consecutive equal observation windows over the horizon.",
    )
    parser.add_argument(
        "--horizon-half-lives",
        type=Decimal,
        default=Decimal(4),
        help="Duration in compiled half-lives, rounded to picosecond windows. Very long horizons use 90%% of the Time range; 0.000001 exercises the small-decay regime.",
    )
    parser.add_argument(
        "--target-last-window",
        type=int,
        default=1000,
        help="Expected total primaries in the final window. Sets activity automatically using the sum of the original yields.",
    )
    parser.add_argument(
        "--population-replicates",
        type=int,
        default=128,
        help="Additional host-only population experiments per window for Poisson shape and dispersion analysis; no extra transported particles.",
    )
    parser.add_argument(
        "--family-alpha",
        type=Decimal,
        default=Decimal("0.01"),
        help="Family significance level, divided among the statistical comparisons by the existing Bonferroni policy.",
    )
    parser.add_argument(
        "--source-tree",
        type=Path,
        default=Path(__file__).resolve().parents[2],
        help="GGEMS source root used to read the built-in spectrum provenance comment.",
    )
    parser.add_argument(
        "--timeout-seconds",
        type=int,
        default=1800,
        help="Maximum elapsed seconds for each exporter invocation, including OpenCL compilation.",
    )
    args = parser.parse_args()

    # ------------------------------------------------------------------------
    # Read the compiled definition and choose activity before sampling.

    reference_path = args.reference.resolve()
    reference = Reference.load(reference_path)
    output = args.output.resolve()
    output.mkdir(parents=True)
    exporter = str(args.exporter.resolve())
    describe_command = [
        exporter,
        "--describe",
        "--nuclide",
        reference.name,
        "--output",
        str(output / "definition"),
    ]
    invoke(describe_command, output / "describe.log", args.timeout_seconds)
    runtime = Runtime.load(output / "definition")
    design = design_campaign(
        runtime,
        args.horizon_half_lives,
        args.windows,
        args.target_last_window,
    )
    command = [
        exporter,
        "--nuclide",
        reference.name,
        "--output",
        str(output / "run"),
        "--device",
        args.device,
        "--workers",
        str(args.workers),
        "--seed",
        str(args.seed),
        "--windows",
        str(design.windows),
        "--step-ps",
        str(design.step_ps),
        "--activity-bq",
        str(design.activity_bq),
        "--capacity",
        str(design.capacity),
        "--population-replicates",
        str(args.population_replicates),
    ]
    design_values = {
        "step_ps": design.step_ps,
        "windows": design.windows,
        "activity_bq": str(design.activity_bq),
        "capacity": design.capacity,
        "requested_horizon_half_lives": str(design.requested_horizon_half_lives),
        "actual_horizon_half_lives": str(design.actual_horizon_half_lives),
        "target_last_window": design.target_last_window,
        "horizon_policy": design.horizon_policy,
    }
    write_json(
        output / "settings.json",
        {
            "reference": str(reference_path),
            "source_tree": str(args.source_tree.resolve()),
            "design": design_values,
            "family_alpha": str(args.family_alpha),
            "commands": [describe_command, command],
            "python": platform.python_version(),
            "platform": platform.platform(),
            "dependencies": {
                name: importlib.metadata.version(name)
                for name in ("numpy", "scipy", "matplotlib")
            },
        },
    )

    # ------------------------------------------------------------------------
    # Generate Source births, then evaluate the fixed scientific comparisons.

    invoke(command, output / "exporter.log", args.timeout_seconds)
    result = analyze_campaign(output, reference, args.source_tree.resolve())
    print(
        f"Completed {reference.name}: {result['sample_count']} primaries; results in {output}"
    )
    print(
        "Read analysis.json for each scientific result; command completion alone is not validation."
    )


if __name__ == "__main__":
    main()
