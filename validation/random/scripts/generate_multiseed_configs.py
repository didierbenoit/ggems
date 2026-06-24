#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations

import json
from pathlib import Path

ENGINES = ["philox", "pcg32", "jkiss"]
SEEDS = [777776, 777777, 777778, 123456789, 987654321, 1122334455, 13579, 12357, 6245453]

PARTICLE_COUNT = 1048576
WORDS_PER_PARTICLE = 1024

BYTES_PER_UINT32 = 4
TOTAL_WORDS = PARTICLE_COUNT * WORDS_PER_PARTICLE
TOTAL_BYTES = TOTAL_WORDS * BYTES_PER_UINT32


def FormatSizeForName(byte_count: int) -> str:
    if byte_count % (1024**3) == 0:
        return f"{byte_count // (1024**3)}gib"

    if byte_count % (1024**2) == 0:
        return f"{byte_count // (1024**2)}mib"

    if byte_count % 1024 == 0:
        return f"{byte_count // 1024}kib"

    return f"{byte_count}b"


def FormatSizeForDescription(byte_count: int) -> str:
    if byte_count % (1024**3) == 0:
        return f"{byte_count // (1024**3)} GiB"

    if byte_count % (1024**2) == 0:
        return f"{byte_count // (1024**2)} MiB"

    if byte_count % 1024 == 0:
        return f"{byte_count // 1024} KiB"

    return f"{byte_count} bytes"


def FormatSizeForPractRand(byte_count: int) -> str:
    if byte_count % (1024**3) == 0:
        return f"{byte_count // (1024**3)}GB"

    if byte_count % (1024**2) == 0:
        return f"{byte_count // (1024**2)}MB"

    if byte_count % 1024 == 0:
        return f"{byte_count // 1024}KB"

    return str(byte_count)


SIZE_NAME = FormatSizeForName(TOTAL_BYTES)
SIZE_DESCRIPTION = FormatSizeForDescription(TOTAL_BYTES)
PRACTRAND_MAX_SIZE = FormatSizeForPractRand(TOTAL_BYTES)

################
def BuildConfig(engine: str, seed: int) -> dict:
    name = f"{engine}_uint32_{SIZE_NAME}_seed{seed}"

    engine_label = {
        "philox": "Philox",
        "pcg32": "PCG32",
        "jkiss": "JKiss"
    }[engine]

    return {
        "schema_version": 1,
        "name": name,
        "description": (
            f"{SIZE_DESCRIPTION} {engine_label} raw uint32 stream for GGEMS "
            f"multi-seed random validation with PractRand."
        ),
        "ggems": {
            "root": ".",
            "commit": "auto",
        },
        "random": {
            "engine": engine,
            "seed": seed,
            "stream_type": "raw_uint32",
            "generation_api": "engine_raw_uint32",
            "particle_count": PARTICLE_COUNT,
            "words_per_particle": WORDS_PER_PARTICLE,
            "total_words": TOTAL_WORDS,
            "layout": "particle_major",
            "endianness": "little",
        },
        "opencl": {
            "device_selector": "gpu",
            "platform": "auto",
            "device": "auto",
            "local_size": 64,
            "build_options": [
                "-cl-std=CL2.0",
            ],
        },
        "output": {
            "stream_path": f"validation/random/streams/{name}.bin",
            "manifest_path": f"validation/random/results/summary/{name}_manifest.json",
            "summary_path": f"validation/random/results/summary/{name}_summary.json",
        },
        "validation": {
            "intended_tools": [
                "PractRand",
            ],
            "purpose": "one_gib_multiseed_practrand_validation",
            "practrand": {
                "max_size": PRACTRAND_MAX_SIZE,
                "raw_log_path": f"validation/random/results/raw/{name}_practrand.log",
                "summary_path": (
                    f"validation/random/results/summary/"
                    f"{name}_practrand_summary.json"
                ),
            },
        },
    }

################
def main() -> None:
    script_path = Path(__file__).resolve()
    ggems_root = script_path.parents[3]
    config_dir = ggems_root / "validation/random/configs"

    config_dir.mkdir(parents=True, exist_ok=True)

    print("GGEMS multi-seed random configuration generator")
    print(f"Particle count     : {PARTICLE_COUNT}")
    print(f"Words per particle : {WORDS_PER_PARTICLE}")
    print(f"Total words        : {TOTAL_WORDS}")
    print(f"Total bytes        : {TOTAL_BYTES}")
    print(f"Size label         : {SIZE_NAME}")
    print(f"PractRand max size : {PRACTRAND_MAX_SIZE}")

    for engine in ENGINES:
        for seed in SEEDS:
            config = BuildConfig(engine, seed)
            output_path = config_dir / f"{config['name']}.json"

            with output_path.open("w", encoding="utf-8") as stream:
                json.dump(config, stream, indent=2)
                stream.write("\n")

            print(f"Generated: {output_path}")

################
if __name__ == "__main__":
    main()
