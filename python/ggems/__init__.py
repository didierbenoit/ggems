from . import ggems

core = ggems.core
opencl = ggems.opencl
rndm = ggems.rndm
run = ggems.run

__all__ = ["core", "opencl", "rndm", "run"]

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

