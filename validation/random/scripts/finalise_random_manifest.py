#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
from pathlib import Path 
from typing import Any

################
def compute_sha256(path: Path) -> str:
    digest = hashlib.sha256()

    with path.open("rb") as stream:
        while True:
            chunk = stream.read(1024*1024)
            if not chunk:
                break

            digest.update(chunk)

    return digest.hexdigest().upper()

################
def load_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)

################
def write_json(path: Path, data: dict[str, Any]) -> None:
    with path.open("w", encoding="utf-8") as stream:
        json.dump(data, stream, indent=2)
        stream.write("\n")

################
def finalise_manifest(manifest_path: Path) -> None:
    manifest = load_json(manifest_path)

    stream_path = Path(manifest["output"]["stream_path"])

    if not stream_path.exists():
        raise FileNotFoundError(f"Stream file not found: {stream_path}")

    sha256 = compute_sha256(stream_path)
    byte_count = stream_path.stat().st_size

    manifest["integrity"] = {
            "algorithm": "SHA-256",
            "value": sha256
            }

    manifest["output"]["byte_count_actual"] = byte_count
    manifest["finalised_at_utc"] = dt.datetime.now(dt.UTC).isoformat()

    write_json(manifest_path, manifest)

    print(f"Manifest : {manifest_path}")
    print(f"Stream   : {stream_path}")
    print(f"SHA-256  : {sha256}")
    print(f"Bytes    : {byte_count}")

################
def main() -> None:
    parser = argparse.ArgumentParser(
            description="Finalise a GGEMS random validation manifest."
            )

    parser.add_argument(
            "--manifest",
            required=True,
            type=Path,
            help="Path to the manifest JSON file."
            )

    args = parser.parse_args()
    finalise_manifest(args.manifest)

################
if __name__ == "__main__":
    main()
