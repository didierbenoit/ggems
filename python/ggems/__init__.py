# ============================================================================
#  @file      __init__.py
#  @brief     GGEMS Python package initialisation file.
#  @details   Provides the high-level Python entry point for the GGEMS
#             simulation framework (GPU Geant4-based Monte Carlo Simulations).
#             This package wraps the C++ core built with Pybind11 and exposes
#             its functionalities in a Pythonic interface.
#
#             GGEMS modules currently available:
#               - core      : Logging, exceptions, and system helpers
#               - opencl    : Platform and device management
#               - runtime   : Kernel and program execution (future)
#               - ui        : Future Vulkan/ImGui graphical interface
# ============================================================================

# Load the compiled extension module
from .ggems import *

__all__ = [name for name in dir() if not name.startswith("_")]

# Package metadata
__version__ = "2.0.0"
__author__ = "Didier Benoit"
__license__ = "GPLv3"
__description__ = "GPU Geant4-based Monte Carlo Simulations (GGEMS)"

