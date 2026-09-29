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

"""Reanalyze retained samples with the campaign's original reference and thresholds."""

import argparse
from pathlib import Path

from radionuclide_validation.analysis import analyze_campaign
from radionuclide_validation.model import Reference, load_json


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "campaign",
        type=Path,
        help="Campaign directory containing settings.json and run/; replaces analysis.json without drawing new samples.",
    )
    args = parser.parse_args()
    directory = args.campaign
    settings = load_json(directory / "settings.json")
    reference = Reference.load(Path(str(settings["reference"])))
    result = analyze_campaign(directory, reference, Path(str(settings["source_tree"])))
    print(f"Analyzed {result['sample_count']} primaries; analysis.json updated.")


if __name__ == "__main__":
    main()
