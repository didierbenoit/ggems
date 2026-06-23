#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

################
def ComputeSHA256(path: Path) -> str:
    digest = hashlib.sha256()

    with path.open("rb") as stream:
        while True:
            chunk = stream.read(1024 * 1024)

            if not chunk:
                break

            digest.update(chunk)

    return digest.hexdigest().upper()

################
def LoadJSON(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)

################
def CheckManifest(manifest_path: Path) -> None:
    manifest = LoadJSON(manifest_path)

    stream_path = Path(manifest["output"]["stream_path"])

    if not stream_path.exists():
        raise FileNotFoundError(f"Stream file not found: {stream_path}")

    expected_byte_count = int(manifest["random"]["byte_count"])
    actual_byte_count = stream_path.stat().st_size

    if actual_byte_count != expected_byte_count:
        raise RuntimeError(
            "Invalid stream byte count:\n"
            f"  expected: {expected_byte_count}\n"
            f"  actual  : {actual_byte_count}"
        )

    expected_sha256 = str(manifest["integrity"]["value"]).upper()
    actual_sha256 = ComputeSHA256(stream_path)

    if actual_sha256 != expected_sha256:
        raise RuntimeError(
            "Invalid stream SHA-256:\n"
            f"  expected: {expected_sha256}\n"
            f"  actual  : {actual_sha256}"
        )

    print("GGEMS random manifest check: OK")
    print(f"Manifest : {manifest_path}")
    print(f"Stream   : {stream_path}")
    print(f"Bytes    : {actual_byte_count}")
    print(f"SHA-256  : {actual_sha256}")

################
def main() -> None:
    parser = argparse.ArgumentParser(
        description="Check a GGEMS random validation manifest."
    )

    parser.add_argument(
        "--manifest",
        required=True,
        type=Path,
        help="Path to the random validation manifest.",
    )

    args = parser.parse_args()
    CheckManifest(args.manifest)

################
if __name__ == "__main__":
    main()
