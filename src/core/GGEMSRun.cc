#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProgram.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

using namespace ggems::units;

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
  using ocl::GGEMSOpenCLKernel;
  using ocl::GGEMSOpenCLProgram;
  using ocl::GGEMSOpenCLSVMBuffer;

  auto &opencl = ocl::GGEMSOpenCL::GetInstance();
  auto &contexts = opencl.GetContext();
  auto &context = contexts.front();

  GGEMS_INFO("Core", "Starting SVM vec_add_svm test on...");

  std::size_t const n = 67'108'864;
  Bytes const bytes = Bytes{static_cast<std::uint64_t>(n) * 4ULL};

  auto svmA = context.CreateSVMBuffer(bytes, ocl::SVMMemoryKind::Auto, 0);
  auto svmB = context.CreateSVMBuffer(bytes, ocl::SVMMemoryKind::Auto, 0);
  auto svmC = context.CreateSVMBuffer(bytes, ocl::SVMMemoryKind::Auto, 0);

  auto *A = static_cast<float *>(svmA.Data());
  auto *B = static_cast<float *>(svmB.Data());
  auto *C = static_cast<float *>(svmC.Data());

  svmA.Map();
  svmB.Map();
  svmC.Map();

  for (std::size_t i = 0; i < n; ++i) {
    A[i] = static_cast<float>(i);
    B[i] = static_cast<float>(2 * i);
    C[i] = 0.0f;
  }

  svmA.Unmap();
  svmB.Unmap();
  svmC.Unmap();

  std::filesystem::path kernel_root = "ggems/kernels";
  std::string kernel_name = "vec_add_svm";

  auto &prog = opencl.GetOrCreateProgram(context, kernel_root, kernel_name, "");
  cl::Kernel raw_kernel = prog.CreateKernel(kernel_name);
  GGEMSOpenCLKernel kernel{context, std::move(raw_kernel), kernel_name};

  kernel.SetArgSVMPointer(0, A, &svmA);
  kernel.SetArgSVMPointer(1, B, &svmB);
  kernel.SetArgSVMPointer(2, C, &svmC);

  kernel.SetArg(3, static_cast<unsigned int>(n));

  std::array<std::size_t, 1> global{n};
  std::array<std::size_t, 1> local{256};

  // kernel.Run(global, local);
  kernel.ProfiledEnqueue(global, local, 3 * bytes);

  svmC.Map();
  bool ok = true;
  for (std::size_t i = 0; i < n; ++i) {
    float expected = 3.0f * static_cast<float>(i);
    if (std::fabs(C[i] - expected) > 1e-5f) {
      GGEMS_ERROR("Run", "Mismatch at i = {}: got {}, expected {}", i, C[i],
                  expected);
      ok = false;
      break;
    }
  }
  svmC.Unmap();

  if (ok) {
    GGEMS_INFO("Run", "(TestSVMSimpleVecAdd) Result is correct.");
  } else {
    GGEMS_ERROR("Run", "(TestSVMSimpleVecAdd) Result is WRONG.");
  }
}

} // namespace ggems::core
