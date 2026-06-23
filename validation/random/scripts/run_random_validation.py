#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import Any

################
def LoadJSON(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)

################
def ResolvePath(path: str | Path, root: Path) -> Path:
    candidate = Path(path)

    if candidate.is_absolute():
        return candidate

    return root / candidate

################
def RunCommand(command: list[str], working_directory: Path) -> None:
    print()
    print(" ".join(command))

    completed = subprocess.run(
        command,
        cwd=working_directory,
        check=False,
    )

    if completed.returncode != 0:
        raise RuntimeError(
            f"Command failed with return code {completed.returncode}: "
            f"{' '.join(command)}"
        )

################
def GetConfigValue(config: dict[str, Any], path: list[str], default: Any) -> Any:
    node: Any = config

    for key in path:
        if not isinstance(node, dict) or key not in node:
            return default

        node = node[key]

    return node

################
def RunRandomValidation(
    config_path: Path,
    generator_path: Path,
    rng_test_path: Path,
    max_size: str | None,
    force: bool
) -> None:
    script_path = Path(__file__).resolve()
    ggems_root = script_path.parents[3]

    config = LoadJSON(config_path)

    name = str(config["name"])

    random_config = config["random"]
    opencl_config = config["opencl"]
    output_config = config["output"]

    engine = str(random_config["engine"])
    seed = str(random_config["seed"])
    particle_count = str(random_config["particle_count"])
    words_per_particle = str(random_config["words_per_particle"])
    local_size = str(GetConfigValue(config, ["opencl", "local_size"], 64))
    device_selector = str(opencl_config["device_selector"])

    stream_path = Path(output_config["stream_path"])
    manifest_path = Path(output_config["manifest_path"])

    practrand_max_size = max_size
    if practrand_max_size is None:
        practrand_max_size = str(
            GetConfigValue(config, ["validation", "practrand", "max_size"], "4MB")
        )

    raw_log_path = Path(
        GetConfigValue(
            config,
            ["validation", "practrand", "raw_log_path"],
            f"validation/random/results/raw/{name}_practrand.log",
        )
    )

    practrand_summary_path = Path(
        GetConfigValue(
            config,
            ["validation", "practrand", "summary_path"],
            f"validation/random/results/summary/{name}_practrand_summary.json",
        )
    )

    generator_path = ResolvePath(generator_path, Path.cwd()).resolve()
    rng_test_path = ResolvePath(rng_test_path, Path.cwd()).resolve()

    finalise_script = ggems_root / "validation/random/scripts/finalise_random_manifest.py"
    check_script = ggems_root / "validation/random/scripts/check_random_manifest.py"
    practrand_script = ggems_root / "validation/random/scripts/run_practrand.py"

    generator_command = [
        str(generator_path),
        "--engine",
        engine,
        "--seed",
        seed,
        "--particles",
        particle_count,
        "--words-per-particle",
        words_per_particle,
        "--local-size",
        local_size,
        "--device",
        device_selector,
        "--output",
        stream_path.as_posix(),
        "--manifest",
        manifest_path.as_posix(),
    ]

    if force:
        generator_command.append("--force")

    finalise_command = [
        sys.executable,
        str(finalise_script),
        "--manifest",
        manifest_path.as_posix(),
    ]

    check_command = [
        sys.executable,
        str(check_script),
        "--manifest",
        manifest_path.as_posix(),
    ]

    practrand_command = [
        sys.executable,
        str(practrand_script),
        "--manifest",
        manifest_path.as_posix(),
        "--rng-test",
        str(rng_test_path),
        "--max-size",
        practrand_max_size,
        "--raw-log",
        raw_log_path.as_posix(),
        "--summary",
        practrand_summary_path.as_posix(),
    ]

    if force:
        practrand_command.append("--force")

    print("GGEMS random validation")
    print(f"Config    : {config_path}")
    print(f"Name      : {name}")
    print(f"Engine    : {engine}")
    print(f"Seed      : {seed}")
    print(f"Device    : {device_selector}")
    print(f"Stream    : {stream_path}")
    print(f"Manifest  : {manifest_path}")
    print(f"PractRand : {practrand_max_size}")

    RunCommand(generator_command, ggems_root)
    RunCommand(finalise_command, ggems_root)
    RunCommand(check_command, ggems_root)
    RunCommand(practrand_command, ggems_root)

    print()
    print("GGEMS random validation completed successfully.")
    print(f"Manifest          : {manifest_path}")
    print(f"PractRand log     : {raw_log_path}")
    print(f"PractRand summary : {practrand_summary_path}")

################
def main() -> None:
    parser = argparse.ArgumentParser(
            description="Run a complete GGEMS random validation workflow from a JSON configuration."
            )

    parser.add_argument(
        "--config",
        required=True,
        type=Path,
        help="Path to the GGEMS random validation configuration.",
    )

    parser.add_argument(
        "--generator",
        required=True,
        type=Path,
        help="Path to ggems_random_stream_generator executable.",
    )

    parser.add_argument(
        "--rng-test",
        required=True,
        type=Path,
        help="Path to PractRand RNG_test executable.",
    )

    parser.add_argument(
        "--max-size",
        default=None,
        help="Override PractRand maximum test size, for example 4MB or 1GB.",
    )

    parser.add_argument(
        "--force",
        action="store_true",
        help="Overwrite existing generated files.",
    )

    args = parser.parse_args()

    try:
        RunRandomValidation(
            config_path=args.config.resolve(),
            generator_path=args.generator,
            rng_test_path=args.rng_test,
            max_size=args.max_size,
            force=args.force,
        )
    except Exception as error:
        print("GGEMS random validation failed:")
        print(error)
        raise SystemExit(1) from error

################
if __name__ == "__main__":
    main()
