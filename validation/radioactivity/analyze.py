"""Reanalyze retained campaign files using their original reference and settings."""

import argparse
from pathlib import Path
from typing import Protocol, cast

from validation.radioactivity.radionuclide_validation.analysis import analyze_campaign
from validation.radioactivity.radionuclide_validation.model import (
    Reference,
    load_json,
    string_value,
)


class Arguments(Protocol):
    campaign: Path


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    _ = parser.add_argument("campaign", type=Path)
    args = cast(Arguments, cast(object, parser.parse_args()))
    settings = load_json(args.campaign / "settings.json")
    reference = Reference.load(Path(string_value(settings["reference"])))
    _ = reference.verify_raw_files(Path(string_value(settings["reference"])))
    result = analyze_campaign(
        args.campaign, reference, Path(string_value(settings["source_tree"]))
    )
    print(f"Analyzed {result['sample_count']} primaries; analysis.json updated.")


if __name__ == "__main__":
    main()
