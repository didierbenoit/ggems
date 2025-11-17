#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProfiler.hh"
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
  using ocl::GGEMSOpenCLProfiler;
  using ocl::GGEMSOpenCLProgram;
  using ocl::GGEMSOpenCLSVMBuffer;

  auto &opencl = ocl::GGEMSOpenCL::GetInstance();
  auto &contexts = opencl.GetContext();
  auto &context = contexts.front();

  GGEMS_INFO("Core", "Starting SVM vec_add_svm test on...");

  std::size_t const n = 16'777'216;
  Bytes const bytes = Bytes{static_cast<std::uint64_t>(n) * 4ULL};

  auto svmA = context.CreateSVMBuffer(bytes);
  auto svmB = context.CreateSVMBuffer(bytes);
  auto svmC = context.CreateSVMBuffer(bytes);

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

  kernel.SetArgSVMPointer(0, A);
  kernel.SetArgSVMPointer(1, B);
  kernel.SetArgSVMPointer(2, C);

  kernel.SetArg(3, static_cast<unsigned int>(n));

  std::array<std::size_t, 1> global{n};
  std::array<std::size_t, 1> local{256};

  // kernel.Run(global, local);
  // kernel.ProfiledEnqueue(global, local, 3 * bytes);
  //  kernel.ProfileWorkGroups(n, 3 * bytes);

  GGEMSOpenCLProfiler::Options opts{{256, 512, 1024, 2048, 4096, 8192, 16384,
                                     32768, 65536, 131072, 262144, 524288,
                                     1'048'576, 2'097'152, 4'194'304, 8'388'608,
                                     16'777'216, 33'554'432, 67'108'864},
                                    3 * 4_B,
                                    true,
                                    true,
                                    true};

  GGEMSOpenCLProfiler profiler{};
  profiler.ProfileKernel(kernel, opts);
  profiler.PrintStaticInfo();

  /*std::vector<std::size_t> elements{
      256,       512,        1024,       2048,      4096,
      8192,      16384,      32768,      65536,     131072,
      262144,    524288,     1'048'576,  2'097'152, 4'194'304,
      8'388'608, 16'777'216, 33'554'432, 67'108'864};
  kernel.ProfileBandwidthSweep(elements, 3 * 4_B);*/

  //  Time tover = kernel.ProfileDriverOverhead();
  //  GGEMS_DEBUG("OpenCL", "Driver overhead {}", HumanReadable(tover);

  /*  std::string fname = kernel.GetFunctionName();
    cl_uint nargs = kernel.GetNumArgs();
    cl_uint refcount = kernel.GetReferenceCount();
    std::string attributes = kernel.GetAttributes();

    GGEMS_DEBUG("OpenCL", "fname: {}", fname);
    GGEMS_DEBUG("OpenCL", "nargs: {}", nargs);
    GGEMS_DEBUG("OpenCL", "refcount: {}", refcount);
    GGEMS_DEBUG("OpenCL", "attributes: {}", attributes);

    std::size_t wg = kernel.GetWorkGroupSize();
    std::size_t pwg = kernel.GetPreferredWorkGroupSizeMultiple();
    std::array<std::size_t, 3> cwg = kernel.GetCompileWorkGroupSize();
    Bytes lmem = kernel.GetLocalMemSize() * 1_B;
    Bytes pmem = kernel.GetPrivateMemSize() * 1_B;

    GGEMS_DEBUG("OpenCL", "work group size: {}", wg);
    GGEMS_DEBUG("OpenCL", "Preferred WG size multiple: {}", pwg);
    GGEMS_DEBUG("OpenCL", "Compile WG size: {}", cwg);
    GGEMS_DEBUG("OpenCL", "Local mem size: {}", HumanReadable(lmem));
    GGEMS_DEBUG("OpenCL", "Private mem size: {}", HumanReadable(pmem));

    for (cl_uint a = 0; a < nargs; ++a) {
      std::string argaddr = kernel.GetArgAddressQualifier(a);
      std::string argname = kernel.GetArgName(a);
      std::string argtypename = kernel.GetArgTypeName(a);
      std::string argacc = kernel.GetArgAccessQualifier(a);
      std::string argtype = kernel.GetArgTypeQualifier(a);
      GGEMS_DEBUG("OpenCL", "-------------");
      GGEMS_DEBUG("OpenCL", "  arg name {}: {}", a, argname);
      GGEMS_DEBUG("OpenCL", "  arg type name {}: {}", a, argtypename);
      GGEMS_DEBUG("OpenCL", "  arg address qualifier {}: {}", a, argaddr);
      GGEMS_DEBUG("OpenCL", "  arg address qualifier {}: {}", a, argtype);
      GGEMS_DEBUG("OpenCL", "  arg address qualifier {}: {}", a, argacc);
    }*/
}

} // namespace ggems::core
