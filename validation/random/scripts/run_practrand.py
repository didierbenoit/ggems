#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations

import argparse
import datetime as dt
import json
import shutil
import subprocess
from pathlib import Path
from typing import Any

################
def ParseByteSize(text: str) -> int:
    value = text.strip().upper()

    units = {
        "B": 1,
        "KB": 1024,
        "KIB": 1024,
        "MB": 1024**2,
        "MIB": 1024**2,
        "GB": 1024**3,
        "GIB": 1024**3,
        "TB": 1024**4,
        "TIB": 1024**4,
    }

    for suffix, multiplier in sorted(units.items(), key=lambda item: len(item[0]), reverse=True):
        if value.endswith(suffix):
            number = value[: -len(suffix)].strip()

            if not number.isdigit():
                raise ValueError(f"Invalid byte size: {text}")

            return int(number) * multiplier

    if value.isdigit():
        return int(value)

    raise ValueError(
        f"Invalid byte size: {text}. "
        "Expected forms such as 4MB, 1GB, 16GiB, or raw bytes."
    )

################
def GetManifestStreamByteCount(manifest: dict[str, Any], stream_path: Path) -> int:
    output = manifest.get("output", {})

    if "byte_count_actual" in output:
        return int(output["byte_count_actual"])

    random_section = manifest.get("random", {})

    if "byte_count" in random_section:
        return int(random_section["byte_count"])

    return stream_path.stat().st_size

################
def LoadJSON(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)

################
def WriteJSON(path: Path, data: dict[str, Any], force: bool) -> None:
    if path.exists() and not force:
        raise FileExistsError(f"File already exists: {path}. Use --force to overwrite it.")

    path.parent.mkdir(parents=True, exist_ok=True)

    with path.open("w", encoding="utf-8") as stream:
        json.dump(data, stream, indent=2)
        stream.write("\n")

################
def EnsureWritable(path: Path, force: bool) -> None:
    if path.exists() and not force:
        raise FileExistsError(f"File already exists: {path}. Use --force to overwrite it.")

    path.parent.mkdir(parents=True, exist_ok=True)

################
def ResolveExecutable(executable: str) -> str:
    resolved = shutil.which(executable)

    if resolved is not None:
        return resolved

    candidate = Path(executable)

    if candidate.exists():
        return str(candidate)

    raise FileNotFoundError(
        f"PractRand executable not found: {executable}\n"
        "Provide it with --rng-test, for example:\n"
        "  --rng-test C:/Tools/PractRand/RNG_test.exe"
    )

################
def ParsePractRandLog(output: str) -> dict[str, Any]:
    lines = output.splitlines()

    version = "unknown"
    tested_length = "unknown"
    test_result_count: int | None = None
    anomalies: list[str] = []
    status = "unknown"

    for line in lines:
        stripped = line.strip()
        lowered = stripped.lower()

        if stripped.startswith("RNG_test.exe using PractRand version"):
            version = stripped.removeprefix(
                "RNG_test.exe using PractRand version"
            ).strip()

        if stripped.startswith("length="):
            tested_length = stripped

        if stripped.startswith("...and ") and "test result" in lowered:
            words = stripped.split()

            if len(words) >= 2:
                try:
                    clean_count = int(words[1])
                    test_result_count = clean_count + len(anomalies)
                except ValueError:
                    pass

        if "no anomalies in" in lowered and "test result" in lowered:
            status = "passed_no_anomalies"

            words = stripped.split()

            for index, word in enumerate(words):
                if word == "in" and index + 1 < len(words):
                    try:
                        test_result_count = int(words[index + 1])
                    except ValueError:
                        test_result_count = None

                    break

        if " unusual" in lowered or lowered.endswith("unusual"):
            anomalies.append(stripped)
            status = "attention_required"

        if "suspicious" in lowered:
            anomalies.append(stripped)
            status = "attention_required"

        if "fail" in lowered or "failed" in lowered:
            anomalies.append(stripped)
            status = "failed"

    return {
        "version": version,
        "tested_length": tested_length,
        "test_result_count": test_result_count,
        "status": status,
        "anomalies": anomalies,
    }

################
def RunPractRand(
    manifest_path: Path,
    rng_test: str,
    max_size: str,
    raw_log_path: Path,
    summary_path: Path,
    force: bool,
) -> None:
    manifest = LoadJSON(manifest_path)
    stream_path = Path(manifest["output"]["stream_path"])

    if not stream_path.exists():
        raise FileNotFoundError(f"Stream file not found: {stream_path}")

    requested_max_bytes = ParseByteSize(max_size)
    available_stream_bytes = GetManifestStreamByteCount(manifest, stream_path)

    actual_file_size = stream_path.stat().st_size

    if actual_file_size != available_stream_bytes:
        raise RuntimeError(
            "Manifest stream byte count does not match actual file size:\n"
            f"  manifest: {available_stream_bytes}\n"
            f"  file    : {actual_file_size}"
        )

    if requested_max_bytes > available_stream_bytes:
        raise RuntimeError(
            "Requested PractRand max-size is larger than the available stream:\n"
            f"  requested: {requested_max_bytes} bytes ({max_size})\n"
            f"  available: {available_stream_bytes} bytes\n"
            "Generate a larger stream or reduce --max-size."
        )

    EnsureWritable(raw_log_path, force)

    rng_test_path = ResolveExecutable(rng_test)

    command = [
        rng_test_path,
        "stdin32",
        "-tlmax",
        max_size,
    ]

    print("GGEMS PractRand validation")
    print(f"Manifest : {manifest_path}")
    print(f"Stream   : {stream_path}")
    print(f"RNG_test : {rng_test_path}")
    print(f"Max size : {max_size}")

    with stream_path.open("rb") as input_stream:
        completed = subprocess.run(
            command,
            stdin=input_stream,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
        )

    raw_log_path.write_text(completed.stdout, encoding="utf-8")

    parsed_log = ParsePractRandLog(completed.stdout)
    status = parsed_log["status"]

    summary = {
        "schema_version": 1,
        "tool": {
            "name": "PractRand",
            "version": parsed_log["version"],
            "executable": rng_test_path,
            "command": command,
            "return_code": completed.returncode,
            "max_size": max_size,
            "requested_max_bytes": requested_max_bytes,
        },
        "input": {
            "manifest_path": str(manifest_path),
            "stream_path": str(stream_path),
            "engine": manifest["random"]["engine"],
            "seed": manifest["random"]["seed"],
            "particle_count": manifest["random"]["particle_count"],
            "words_per_particle": manifest["random"]["words_per_particle"],
            "total_words": manifest["random"]["total_words"],
            "stream_byte_count": available_stream_bytes,
            "sha256": manifest["integrity"]["value"],
        },
        "output": {
            "raw_log_path": str(raw_log_path),
            "status": status,
            "tested_length": parsed_log["tested_length"],
            "test_result_count": parsed_log["test_result_count"],
            "anomalies": parsed_log["anomalies"],
        },
        "generated_at_utc": dt.datetime.now(dt.UTC).isoformat(),
    }

    WriteJSON(summary_path, summary, force)

    print(f"Raw log  : {raw_log_path}")
    print(f"Summary  : {summary_path}")
    print(f"Status   : {status}")
    print(f"Tested   : {parsed_log['tested_length']}")
    print(f"Results  : {parsed_log['test_result_count']}")

    if completed.returncode != 0:
        raise RuntimeError(f"PractRand returned non-zero code: {completed.returncode}")

################
def main() -> None:
    parser = argparse.ArgumentParser(
        description="Run PractRand on a GGEMS random validation stream."
    )

    parser.add_argument(
        "--manifest",
        required=True,
        type=Path,
        help="Path to the GGEMS random stream manifest.",
    )

    parser.add_argument(
        "--rng-test",
        default="RNG_test",
        help="Path to the PractRand RNG_test executable.",
    )

    parser.add_argument(
        "--max-size",
        default="4MB",
        help="PractRand maximum test length, for example 4MB, 1GB, or 16GB.",
    )

    parser.add_argument(
        "--raw-log",
        required=True,
        type=Path,
        help="Path to the raw PractRand log.",
    )

    parser.add_argument(
        "--summary",
        required=True,
        type=Path,
        help="Path to the PractRand summary JSON.",
    )

    parser.add_argument(
        "--force",
        action="store_true",
        help="Overwrite existing output files.",
    )

    args = parser.parse_args()

    RunPractRand(
        manifest_path=args.manifest,
        rng_test=args.rng_test,
        max_size=args.max_size,
        raw_log_path=args.raw_log,
        summary_path=args.summary,
        force=args.force,
    )

################
if __name__ == "__main__":
    main()
