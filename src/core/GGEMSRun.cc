#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

namespace ggems::core {

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSRun::GGEMSRun() {
  GGEMS_INFOEX("Core", 3, "GGEMSRun created.");
  Banner();
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSRun::~GGEMSRun() { GGEMS_INFOEX("Core", 3, "GGEMSRun destroyed."); }

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSRun::Banner() const {
  constexpr std::string_view GGEMS_BANNER = R"(

╭──────────────────────────────────────────────────╮
|   ██████╗  ██████╗ ███████╗███╗   ███╗███████╗   |
|  ██╔════╝ ██╔════╝ ██╔════╝████╗ ████║██╔════╝   |
|  ██║  ███╗██║  ███╗█████╗  ██╔████╔██║███████╗   |
|  ██║   ██║██║   ██║██╔══╝  ██║╚██╔╝██║╚════██║   |
|  ╚██████╔╝╚██████╔╝███████╗██║ ╚═╝ ██║███████║   |
|   ╚═════╝  ╚═════╝ ╚══════╝╚═╝     ╚═╝╚══════╝   |
|                                                  |
|    GPU Geant4-based Monte Carlo Simulations      |
|   Version 2.0 • GGEMS Team • https://ggems.fr    |
|     Authors: Julien Bert  &  Didier Benoit       |
|  Copyright © 2025  Licensed under GNU GPL v3.0   |
╰──────────────────────────────────────────────────╯
)";
  GGEMS_INFO("Core", "{}", GGEMS_BANNER);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSRun::Initialise() {
  GGEMS_INFO("Core", "Initialising GGEMSRun...");

  GGEMS_INFO("Core", "GGEMSRun initialised.");
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSRun::Run() {
  auto &opencl = ocl::GGEMSOpenCL::GetInstance();

  auto const &contexts = opencl.GetContext();

  auto context = contexts.front();

  auto svmA = context.CreateSVMBuffer(sizeof(float) * 1024);
  float *A = (float *)svmA.Data();
}

} // namespace ggems::core
