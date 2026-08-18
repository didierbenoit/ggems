import os as _os
from pathlib import Path as _Path

_dll_directory_handles = []

try:
    from . import ggems
except ImportError:
    if _os.name != "nt":
        raise

    _runtime_dirs = []

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

logging = ggems.logging
opencl = ggems.opencl
rndm = ggems.rndm
source = ggems.source
run = ggems.run
observer = ggems.observer
radionuclide = ggems.radionuclide

__all__ = ["logging", "opencl", "rndm", "source", "run", "observer", "radionuclide"]

if hasattr(ggems, "gui"):
    gui = ggems.gui
    __all__.append("gui")

# Package metadata
__version__ = "2.0.0"
__author__ = (
    "Didier Benoit <didier.benoit@inserm.fr>",
    "Julien Bert <julien.bert@univ-brest.fr>",
)
__license__ = "GPLv3"
__description__ = "GPU Geant4-based Monte Carlo Simulations (GGEMS)"
