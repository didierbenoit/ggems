"""Reanalyze retained samples with the campaign's original reference and thresholds."""

import argparse
from pathlib import Path
from typing import cast

from radionuclide_validation.analysis import analyze_campaign
from radionuclide_validation.model import Reference, load_json


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    _ = parser.add_argument(
        "campaign",
        type=Path,
        help="Campaign directory containing settings.json and run/; replaces analysis.json without drawing new samples.",
    )
    args = cast(dict[str, Path], vars(parser.parse_args()))
    directory = args["campaign"]
    settings = load_json(directory / "settings.json")
    reference = Reference.load(Path(str(settings["reference"])))
    result = analyze_campaign(directory, reference, Path(str(settings["source_tree"])))
    print(f"Analyzed {result['sample_count']} primaries; analysis.json updated.")


if __name__ == "__main__":
    main()
