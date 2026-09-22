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

"""GGEMS Python package.

GGEMS - GPU Geant4-based Monte Carlo Simulations.

This package exposes the GGEMS C++ engine through Python bindings and provides
access to the main runtime, OpenCL, random, materials, production cuts, source,
radionuclide, observer, run, and optional graphical-interface modules.

Top-level functions control GGEMS logging and output runtime behavior.
"""

import os as _os
from pathlib import Path as _Path

_dll_directory_handles: list[object] = []

try:
    from . import ggems
except ImportError:
    if _os.name != "nt":
        raise

    _runtime_dirs: list[_Path] = []

    _oneapi_root = _os.environ.get("ONEAPI_ROOT")
    if _oneapi_root:
        _runtime_dirs.extend(
            [
                _Path(_oneapi_root) / "compiler" / "latest" / "bin",
                _Path(_oneapi_root) / "bin",
            ]
        )

    _program_files_x86 = _os.environ.get("ProgramFiles(x86)")
    if _program_files_x86:
        _runtime_dirs.append(
            _Path(_program_files_x86)
            / "Intel"
            / "oneAPI"
            / "compiler"
            / "latest"
            / "bin"
        )

    for _runtime_dir in dict.fromkeys(_runtime_dirs):
        if any(
            (_runtime_dir / _dll).is_file() for _dll in ("libmmd.dll", "libmmdd.dll")
        ):
            _dll_directory_handles.append(_os.add_dll_directory(str(_runtime_dir)))

    if not _dll_directory_handles:
        raise

    from . import ggems


def start(mode: str | None = None) -> None:
    """Start GGEMS output.

    Terminal output is used by default when no mode is specified.

    Args:
        mode: Optional output mode. Accepted values are "term" and "gui".
    """
    if mode is None:
        ggems.start()
    else:
        ggems.start(mode)


def stop() -> None:
    """Stop GGEMS output.

    The selected output mode and configured sinks are retained and can be reused
    by a later call to start().
    """
    ggems.stop()


def is_started() -> bool:
    """Return whether GGEMS output is currently started.

    Returns:
        bool: True when GGEMS output is started.
    """
    return ggems.is_started()


def set_detail_level(detail: int = 1) -> None:
    """Set the maximum GGEMS informational detail depth.

    Args:
        detail: Maximum accepted logging depth.
    """
    ggems.set_detail_level(detail)


def set_output_file(path: str) -> None:
    """Enable an optional plain-text GGEMS log file.

    The output file can only be changed while GGEMS output is stopped.

    Args:
        path: Destination log-file path.
    """
    ggems.set_output_file(path)


def clear_output_file() -> None:
    """Disable the optional GGEMS log file.

    This operation is only allowed while GGEMS output is stopped.
    """
    ggems.clear_output_file()


opencl = ggems.opencl
rndm = ggems.rndm
source = ggems.source
run = ggems.run
observer = ggems.observer
radionuclide = ggems.radionuclide
cuts = ggems.cuts

from . import materials  # noqa: E402

__all__ = [
    "start",
    "stop",
    "is_started",
    "set_detail_level",
    "set_output_file",
    "clear_output_file",
    "opencl",
    "rndm",
    "materials",
    "cuts",
    "source",
    "run",
    "observer",
    "radionuclide",
]

if hasattr(ggems, "gui"):
    gui = ggems.gui
    __all__.append("gui")

# Package metadata
__version__ = "2.0.0"
__author__ = (
    "Didier Benoit <didier.benoit@inserm.fr>",
    "Julien Bert <julien.bert@univ-brest.fr>",
)
__license__ = "GPL-3.0-or-later"
__description__ = "GPU Geant4-based Monte Carlo Simulations (GGEMS)"
