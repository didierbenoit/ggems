#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"

#include "GGEMS/frameworks/GGEMSOpenCLProfiler.hh"

#include "GGEMS/render/GGEMSTerminalRenderer.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/core/GGEMSOutputMode.hh"

using namespace ggems::units;

namespace ggems::core {

/* --------------------------------*/

GGEMSRun::GGEMSRun() {
  GGEMS_INFOEX("Core", 3, "GGEMSRun created.");
  /*  Encoding encoding = GGEMSLogger::GetInstance().GetEncoding();
    if (encoding == Encoding::Utf32) {
      Banner();
    } else {
      BannerAscii();
    }*/
}

/* --------------------------------*/

GGEMSRun::~GGEMSRun() { ; }

/* --------------------------------*/

/*void GGEMSRun::Banner() const noexcept {
  constexpr std::string_view GGEMS_BANNER = R"(

╔══════════════════════════════════════════════════╗
║                                                  ║
║   ██████╗  ██████╗ ███████╗███╗   ███╗███████╗   ║
║  ██╔════╝ ██╔════╝ ██╔════╝████╗ ████║██╔════╝   ║
║  ██║  ███╗██║  ███╗█████╗  ██╔████╔██║███████╗   ║
║  ██║   ██║██║   ██║██╔══╝  ██║╚██╔╝██║╚════██║   ║
║  ╚██████╔╝╚██████╔╝███████╗██║ ╚═╝ ██║███████║   ║
║   ╚═════╝  ╚═════╝ ╚══════╝╚═╝     ╚═╝╚══════╝   ║
║                                                  ║
╟──────────────────────────────────────────────────╢
║                                                  ║
║    GPU Geant4-based Monte Carlo Simulations      ║
║   Version 2.0 • GGEMS Team • https://ggems.fr    ║
║     Authors: Julien Bert  &  Didier Benoit       ║
║  Copyright © 2025  Licensed under GNU GPL v3.0   ║
║                                                  ║
╚══════════════════════════════════════════════════╝
)";

  GGEMS_INFO("Core", "{}", GGEMS_BANNER);
}*/

/* --------------------------------*/

/*void GGEMSRun::BannerAscii() const noexcept {
  constexpr std::string_view GGEMS_BANNER_ASCII = R"(

+**************************************************+
*                                                  *
*   ######\  ######\ #######\###\   ###\#######\   *
*  ##/----/ ##/----/ ##/----/####\ ####|##/----/   *
*  ##|  ###\##|  ###\#####\  ##/####/##|#######\   *
*  ##|   ##|##|   ##|##/--/  ##|\##//##|\----##|   *
*  \######//\######//#######\##| \-/ ##|#######|   *
*   \-----/  \-----/ \------/\-/     \-/\------/   *
*                                                  *
+--------------------------------------------------+
*                                                  *
*    GPU Geant4-based Monte Carlo Simulations      *
*   Version 2.0 . GGEMS Team . https://ggems.fr    *
*     Authors: Julien Bert  &  Didier Benoit       *
* Copyright (C) 2025  Licensed under GNU GPL v3.0  *
*                                                  *
+**************************************************+
)";

  GGEMS_INFO("Core", "{}", GGEMS_BANNER_ASCII);
}*/

/* --------------------------------*/

void GGEMSRun::Initialise() {
  GGEMS_INFO("Core", "Initialising GGEMSRun...");

  GGEMS_INFO("Core", "GGEMSRun initialised.");
}

/* --------------------------------*/
void GGEMSRun::Run() {
  GGEMS_INFO("Core", "GGEMS starting...");
  auto &state = core::EnsureOutputState();
  render::GGEMSBanner banner;
  render::GGEMSTerminalRenderer renderer(banner, state);
  renderer.Start();
  renderer.RenderOnce();
  std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  renderer.Stop();

  /*  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    auto &opencl = ocl::GGEMSOpenCL::GetInstance();
    auto &contexts = opencl.GetContext();

    // Filling slots
    for (auto &ctx : contexts) {
      auto &dev = ctx.GetDevice();
      auto const device_name = dev.GetName();
      auto const device_type = dev.GetType();
      auto const device_luid = dev.GetLUIDKhr();
      progress_bar_
          .AddSlot(device_name,
                   (device_type == CL_DEVICE_TYPE_GPU) ? true : false,
                   device_luid)
          .SetKernelName("vec_add")
          .SetBatchesDone(0ULL)
          .SetBatchesTotal(100ULL)
          .SetStatus(GGEMSProgressBar::Slot::Status::Pending)
          .SetParticleType(GGEMSProgressBar::Slot::ParticleType::Gamma)
          .SetETAPicoseconds(0ULL)
          .SetBandwidthBytesPerPicosecond(0.0);
    }

    workers_.clear();
    workers_.reserve(contexts.size());
    progress_bar_.Start();
    for (std::size_t i = 0; i < contexts.size(); ++i) {
      // auto &ctx = contexts[i];
      auto &slot = progress_bar_.GetSlot(i);
      workers_.emplace_back([&slot]() {
        uint64_t p = 0ULL;
        auto start = std::chrono::high_resolution_clock::now();

        uint64_t nbatch = 100ULL;
        while (p < nbatch) {
          ++p;

          auto now = std::chrono::high_resolution_clock::now();
          std::this_thread::sleep_for(std::chrono::milliseconds(100));

          int64_t elapsed_ps =
              std::chrono::duration_cast<std::chrono::nanoseconds>(now - start)
                  .count() *
              1000;

          long double ratio =
              static_cast<long double>(p) / static_cast<long double>(nbatch);
          if (ratio > 0.0) {
            long double estimated_total_ps =
                static_cast<long double>(elapsed_ps) / ratio;
            long double remaining_ps =
                estimated_total_ps - static_cast<long double>(elapsed_ps);
            slot.SetETAPicoseconds((remaining_ps > 0.0)
                                       ? static_cast<uint64_t>(remaining_ps)
                                       : 0ULL);
          } else {
            slot.SetETAPicoseconds(0ULL);
          }

          slot.SetStatus(GGEMSProgressBar::Slot::Status::Running)
              .SetBatchesDone(p)
              .SetBandwidthBytesPerPicosecond(5.0 * static_cast<double>(p));
        }
        slot.SetStatus(GGEMSProgressBar::Slot::Status::Finished);
      });
    }

    for (auto &t : workers_) {
      if (t.joinable()) {
        t.join();
      }
    }
    workers_.clear();
    progress_bar_.Stop();
  */
  /*   auto &opencl = ocl::GGEMSOpenCL::GetInstance();
     auto &contexts = opencl.GetContext();

     std::size_t const n = 33'554'432;
     Bytes const bytes = Bytes{static_cast<std::uint64_t>(n) * 4ULL};
     std::uint64_t n_vec_add_repeat = 1000ULL;

     // Filling slots
     for (auto &ctx : contexts) {
       auto &dev = ctx.GetDevice();
       auto const device_name = dev.GetName();
       auto const device_type = dev.GetType();
       auto const device_luid = dev.GetLUIDKhr();
       progress_bar_
           .AddSlot(device_name,
                    (device_type == CL_DEVICE_TYPE_GPU) ? true : false,
                    device_luid)
           .SetKernelName("vec_add")
           .SetBatchesDone(0ULL)
           .SetBatchesTotal(n_vec_add_repeat)
           .SetStatus(GGEMSProgressBar::Slot::Status::Pending)
           .SetParticleType(GGEMSProgressBar::Slot::ParticleType::Gamma)
           .SetETAPicoseconds(0ULL)
           .SetBandwidthBytesPerPicosecond(0.0);
     }

     workers_.clear();
     workers_.reserve(contexts.size());
     progress_bar_.Start();

     for (std::size_t i = 0; i < contexts.size(); ++i) {
       auto &ctx = contexts[i];
       auto &slot = progress_bar_.GetSlot(i);
       workers_.emplace_back([&opencl, &ctx, &slot, bytes, n_vec_add_repeat]() {
         auto svmA = ctx.CreateSVMBuffer(bytes);
         auto svmB = ctx.CreateSVMBuffer(bytes);
         auto svmC = ctx.CreateSVMBuffer(bytes);

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

         auto &prog = opencl.GetOrCreateProgram(ctx, kernel_root, kernel_name,
     ""); cl::Kernel raw_kernel = prog.CreateKernel(kernel_name);
         ocl::GGEMSOpenCLKernel kernel{ctx, std::move(raw_kernel), kernel_name};

         kernel.SetArgSVMPointer(0, A);
         kernel.SetArgSVMPointer(1, B);
         kernel.SetArgSVMPointer(2, C);

         kernel.SetArg(3, static_cast<unsigned int>(n));

         uint64_t p = 0ULL;
         auto start = std::chrono::high_resolution_clock::now();

         while (p < n_vec_add_repeat) {
           ++p;

           auto now = std::chrono::high_resolution_clock::now();

           std::array<std::size_t, 1> global{n};
           std::array<std::size_t, 1> local{256};

           kernel.Run(global, local);

           int64_t elapsed_ps =
               std::chrono::duration_cast<std::chrono::nanoseconds>(now - start)
                   .count() *
               1000;

           long double ratio = static_cast<long double>(p) /
                               static_cast<long double>(n_vec_add_repeat);
           if (ratio > 0.0) {
             long double estimated_total_ps =
                 static_cast<long double>(elapsed_ps) / ratio;
             long double remaining_ps =
                 estimated_total_ps - static_cast<long double>(elapsed_ps);
             slot.SetETAPicoseconds((remaining_ps > 0.0)
                                        ? static_cast<uint64_t>(remaining_ps)
                                        : 0ULL);
           } else {
             slot.SetETAPicoseconds(0ULL);
           }

           slot.SetStatus(GGEMSProgressBar::Slot::Status::Running)
               .SetBatchesDone(p);
         }
         slot.SetStatus(GGEMSProgressBar::Slot::Status::Finished);
       });
     }

     for (auto &t : workers_) {
       if (t.joinable()) {
         t.join();
       }
     }
     workers_.clear();

     progress_bar_.Stop();*/

  /*  auto &opencl = ocl::GGEMSOpenCL::GetInstance();
    auto &contexts = opencl.GetContext();

    auto &context = contexts.front();

    GGEMS_INFO("Core", "Starting SVM vec_add_svm test on...");

    std::size_t const n = 16'777'216;
    Bytes const bytes = Bytes{static_cast<std::uint64_t>(n) * 4ULL};

    auto svmA = context.CreateSVMBuffer(bytes);
    auto svmB = context.CreateSVMBuffer(bytes);
    auto svmC = context.CreateSVMBuffer(bytes);

    auto *A = static_cast<float *>(svmA.GetData());
    auto *B = static_cast<float *>(svmB.GetData());
    auto *C = static_cast<float *>(svmC.GetData());

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

    auto &prog = opencl.GetOrCreateProgram(context, kernel_root, kernel_name,
    ""); cl::Kernel raw_kernel = prog.CreateKernel(kernel_name);
    ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel), kernel_name};

    kernel.SetArgSVMPointer(0, A);
    kernel.SetArgSVMPointer(1, B);
    kernel.SetArgSVMPointer(2, C);

    kernel.SetArg(3, static_cast<unsigned int>(n));

    std::array<std::size_t, 1> global{n};
    std::array<std::size_t, 1> local{256};

    // kernel.Run(global, local);

    // kernel.ProfiledEnqueue(global, local, 3 * bytes);
    // kernel.ProfileWorkGroups(n, 3 * bytes);

    ocl::GGEMSOpenCLProfiler::Options opts{
        {256,       512,        1024,       2048,       4096,
         8192,      16384,      32768,      65536,      131072,
         262144,    524288,     1'048'576,  2'097'152,  4'194'304,
         8'388'608, 16'777'216, 33'554'432, 67'108'864, 134'217'728},
        3 * 4_B,
        true,
        true,
        true};

    ocl::GGEMSOpenCLProfiler profiler{};
    profiler.ProfileKernel(kernel, opts);
    profiler.PrintAllInfo();

    std::vector<std::size_t> elements{
        256,       512,        1024,       2048,      4096,
        8192,      16384,      32768,      65536,     131072,
        262144,    524288,     1'048'576,  2'097'152, 4'194'304,
        8'388'608, 16'777'216, 33'554'432, 67'108'864};
    kernel.ProfileBandwidthSweep(elements, 3 * 4_B);

    Time tover = kernel.ProfileDriverOverhead();
    GGEMS_DEBUG("OpenCL", "Driver overhead {}", HumanReadable(tover));
  */
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

  GGEMS_INFO("Core", "GGEMS run completed.");
}
} // namespace ggems::core
