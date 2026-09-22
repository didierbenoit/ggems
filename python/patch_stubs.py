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
# Julien BERT <julien.bert@univ-brest.fr>
# Didier BENOIT <didier.benoit@inserm.fr>

"""Patch GGEMS type stubs where pybind11-stubgen cannot infer precise types."""

from __future__ import annotations

import re
import sys
from pathlib import Path


_AVAILABLE_SIGNATURE = re.compile(
    r"^(?P<indent>[ \t]*)def available\(\) -> [^:\n]+:",
    re.MULTILINE,
)

_EXPECTED_AVAILABLE_SIGNATURE = "def available() -> tuple[str, ...]:"


def _find_radionuclide_stub(stub_root: Path) -> tuple[Path, str]:
    candidates: list[tuple[Path, str]] = []

    for path in stub_root.rglob("*.pyi"):
        text = path.read_text(encoding="utf-8")

        if "RadionuclideDefinition" in text and "def available(" in text:
            candidates.append((path, text))

    if len(candidates) != 1:
        raise RuntimeError(
            f"Expected exactly one generated radionuclide stub containing RadionuclideDefinition and available(), found {len(candidates)}."
        )

    return candidates[0]


def _patch_radionuclide_available(stub_root: Path) -> None:
    path, text = _find_radionuclide_stub(stub_root)

    if _EXPECTED_AVAILABLE_SIGNATURE in text:
        print(f"GGEMS stub already precise: {path}")
        return

    matches = list(_AVAILABLE_SIGNATURE.finditer(text))

    if len(matches) != 1:
        raise RuntimeError(
            f"Expected exactly one available() signature in the generated radionuclide stub '{path}', found {len(matches)}."
        )

    match = matches[0]
    replacement = f"{match.group('indent')}{_EXPECTED_AVAILABLE_SIGNATURE}"

    patched = text[: match.start()] + replacement + text[match.end() :]
    _ = path.write_text(patched, encoding="utf-8")

    print(f"Patched GGEMS stub: {path}")
    print("  available() -> tuple[str, ...]")


def main() -> int:
    if len(sys.argv) != 2:
        print("Usage: patch_stubs.py <stub-root>", file=sys.stderr)
        return 2

    stub_root = Path(sys.argv[1]).resolve()

    if not stub_root.is_dir():
        print(f"Stub root does not exist: {stub_root}", file=sys.stderr)
        return 2

    try:
        _patch_radionuclide_available(stub_root)
    except RuntimeError as error:
        print(f"GGEMS stub patch failed: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
