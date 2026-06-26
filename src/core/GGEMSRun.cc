#include <filesystem>
#include <memory>

#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"

using namespace ggems::units;

namespace ggems::core {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSRun::GGEMSRun() { GGEMS_INFOEX("Core", 3, "GGEMSRun instance created."); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSRun::SetRandom(std::shared_ptr<random::GGEMSRandom> random) {
  GGEMS_CHECK_RECOVERABLE(random != nullptr,
                          "Cannot attach a null GGEMSRandom to GGEMSRun.");

  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot change GGEMSRandom after Initialise.");

  random_ = std::move(random);

  GGEMS_INFO("Random", "GGEMSRun random engine set to '{}'.",
             random_->GetEngineName());
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSRun::SetPrimaryCount(std::uint32_t primary_count) {
  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot change primary count after Initialise.");

  primary_stream_.SetPrimaryCount(primary_count);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSRun::SetWorkerCount(std::uint32_t worker_count) {
  GGEMS_CHECK_RECOVERABLE(worker_count > 0ULL,
                          "GGEMSRun worker count must be non-zero.");

  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot change worker count after Initialise.");

  worker_count_ = worker_count;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSRun::Initialise() {
  GGEMS_CHECK_RECOVERABLE(
      random_ != nullptr,
      "GGEMSRun cannot be initialised without a GGEMSRandom. "
      "Create a ggems.rndm GGEMSRandom object and attach it with "
      "GGEMSRun::SetRandom before calling Initialise.");

  auto &opencl = ocl::GGEMSOpenCL::GetInstance();

  GGEMS_CHECK_RECOVERABLE(!opencl.GetContext().empty(),
                          "GGEMSRun requires initialised OpenCL contexts.");

  GGEMS_INFO("Core", "Initialising GGEMSRun stable state...");

  GGEMS_INFO("Random", "Random engine ready: {} with seed {}.",
             random_->GetEngineName(), random_->GetSeed());

  primary_stream_.Initialise();

  std::filesystem::path kernel_root{GGEMS_KERNEL_ROOT};
  dummy_transport_ = std::make_unique<transport::GGEMSDummyTransportWorkload>(
      opencl.GetContext().front(), kernel_root, *random_, worker_count_);

  next_run_id_ = 0ULL;
  initialised_ = true;

  GGEMS_INFO("Core", "GGEMSRun Initialised.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSRun::Run() {
  GGEMS_CHECK_RECOVERABLE(initialised_,
                          "GGEMSRun::Run called before Initialise.");

  GGEMS_CHECK_RECOVERABLE(!running_.exchange(true),
                          "GGEMSRun is already running.");

  std::uint64_t run_id = next_run_id_++;
  auto primary_view = primary_stream_.PrepareRun(run_id);

  GGEMS_INFO("Core", "GGEMSRun projection {} started.", run_id);

  GGEMS_INFOEX("Core", 1,
               "Projection {} primary stream: {} primaries, global history "
               "offset {}.",
               run_id, primary_view.source_primary_count,
               primary_view.global_history_offset);

  GGEMS_INFOEX("Core", 1, "Projection {} worker count: {}.", run_id,
               worker_count_);

  transport::GGEMSDummyTransportRunConfig config{};
  config.total_primary_count = primary_count_;

  dummy_transport_->Run(config);

  auto counters = dummy_transport_->ReadCountersOnHost();

  GGEMS_INFO("Core",
             "Projection {} transport report: primaries={}, histories={}, "
             "secondaries={}, terminal_particles={}, fake_steps={}, "
             "max_stack_depth={}, overflow={}.",
             run_id, counters.consumed_primary_count,
             counters.completed_history_count, counters.created_secondary_count,
             counters.terminal_particle_count, counters.total_fake_step_count,
             counters.max_stack_depth, counters.overflow_count);

  GGEMS_INFO("Core", "GGEMSRun projection {} completed.", run_id);

  running_.store(false);

  /*GGEMS_INFO("Core", "GGEMS starting...");

  Angle a = 90.0_deg;
  Angle b = 1.5707963267948966_rad;

  GGEMS_DEBUG("Core", "{}", HumanReadable(a));
  GGEMS_DEBUG("Core", "{}", HumanReadable(b));

  Frequency f = 3874364_Hz;
  GGEMS_DEBUG("Core", "{}", HumanReadable(f, 1, 5));
  GGEMS_WARN("Core", "Test");
  GGEMS_ERROR("Core", "Test");

  render::GGEMSProgressBar *progress_bar =
      core::IsProgressBarAvailable() ? &core::GetProgressBar() : nullptr;

  auto &opencl = ocl::GGEMSOpenCL::GetInstance();
  auto &contexts = opencl.GetContext();

  std::vector<ocl::GGEMSOpenCLSVMBuffer> buffers;

  if (progress_bar) {
    progress_bar->Clear();
  }

  // Filling slots
  for (auto &ctx : contexts) {
    buffers.push_back(ctx.CreateSVMBuffer(1024_B));

    if (progress_bar) {
      auto &dev = ctx.GetDevice();
      auto device_name = dev.GetName();
      auto device_type = dev.GetType();
      auto device_luid = dev.GetLUIDKhr();
      progress_bar
          ->AddSlot(device_name,
                    (device_type == CL_DEVICE_TYPE_GPU) ? true : false,
                    device_luid)
          .SetKernelName("vec_add")
          .SetBatchesDone(0ULL)
          .SetBatchesTotal(100ULL)
          .SetStatus(render::GGEMSProgressBar::Slot::Status::Pending)
          .SetParticleType(
              render::GGEMSProgressBar::Slot::ParticleType::Aionino)
          .SetETAPicoseconds(0ULL)
          .SetPercentVRAM(ctx.GetPercentVRAM())
          .SetTotalVRAM(ctx.GetTotalVRAM().value)
          .SetAllocatedVRAM(ctx.GetAllocatedVRAM().value)
          .SetAllocationCountVRAM(ctx.GetAllocationCountVRAM());
    }
  }

  workers_.clear();
  workers_.reserve(contexts.size());

  std::atomic<std::size_t> finished_workers{0};

  for (std::size_t i = 0; i < contexts.size(); ++i) {
    if (progress_bar) {
      auto &slot = progress_bar->GetSlot(i);
      workers_.emplace_back([&slot, &finished_workers]() {
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

          if (ratio > 0.0L) {
            long double estimated_total_ps =
                static_cast<long double>(elapsed_ps) / ratio;
            long double remaining_ps =
                estimated_total_ps - static_cast<long double>(elapsed_ps);

            slot.SetETAPicoseconds((remaining_ps > 0.0L)
                                       ? static_cast<uint64_t>(remaining_ps)
                                       : 0ULL);
          } else {
            slot.SetETAPicoseconds(0ULL);
          }

          slot.SetStatus(render::GGEMSProgressBar::Slot::Status::Running)
              .SetBatchesDone(p);
        }
        slot.SetStatus(render::GGEMSProgressBar::Slot::Status::Finished);
        finished_workers.fetch_add(1, std::memory_order_relaxed);
      });
    } else {
      workers_.emplace_back([&finished_workers]() {
        std::uint64_t p = 0ULL;
        std::uint64_t nbatch = 100ULL;

        while (p < nbatch) {
          ++p;
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        finished_workers.fetch_add(1, std::memory_order_relaxed);
      });
    }
  }

  while (finished_workers.load(std::memory_order_relaxed) < workers_.size()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(33));
  }

  for (auto &t : workers_) {
    if (t.joinable()) {
      t.join();
    }
  }
  workers_.clear();

  GGEMS_INFO("Core", "GGEMS run completed.");
*/
  /*  auto &opencl = ocl::GGEMSOpenCL::GetInstance();
    auto &contexts = opencl.GetContext();

    render::GGEMSProgressBar *progress_bar =
        core::IsProgressBarAvailable() ? &core::GetProgressBar() : nullptr;

    std::size_t const n = 33'554'432;
    Bytes const bytes = Bytes{static_cast<std::uint64_t>(n) * 4ULL};
    std::uint64_t n_vec_add_repeat = 1000ULL;

    // Filling slots
    for (auto &ctx : contexts) {
      auto &dev = ctx.GetDevice();
      auto const device_name = dev.GetName();
      auto const device_type = dev.GetType();
      auto const device_luid = dev.GetLUIDKhr();
      progress_bar
          ->AddSlot(device_name,
                    (device_type == CL_DEVICE_TYPE_GPU) ? true : false,
                    device_luid)
          .SetKernelName("vec_add")
          .SetBatchesDone(0ULL)
          .SetBatchesTotal(n_vec_add_repeat)
          .SetStatus(render::GGEMSProgressBar::Slot::Status::Pending)
          .SetParticleType(render::GGEMSProgressBar::Slot::ParticleType::Gamma)
          .SetETAPicoseconds(0ULL);
      //.SetBandwidthBytesPerPicosecond(0.0);
    }

    workers_.clear();
    workers_.reserve(contexts.size());
    // progress_bar->Start();

    for (std::size_t i = 0; i < contexts.size(); ++i) {
      auto &ctx = contexts[i];
      auto &slot = progress_bar->GetSlot(i);
      workers_.emplace_back([&opencl, &ctx, &slot, bytes, n_vec_add_repeat]() {
        auto svmA = ctx.CreateSVMBuffer(bytes);
        auto svmB = ctx.CreateSVMBuffer(bytes);
        auto svmC = ctx.CreateSVMBuffer(bytes);

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

          slot.SetStatus(render::GGEMSProgressBar::Slot::Status::Running)
              .SetBatchesDone(p);
        }
        slot.SetStatus(render::GGEMSProgressBar::Slot::Status::Finished);
      });
    }

    for (auto &t : workers_) {
      if (t.joinable()) {
        t.join();
      }
    }
    workers_.clear();
  */
  // progress_bar_.Stop();

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

    kernel.Run(global, local);

    kernel.ProfiledEnqueue(global, local, 3 * bytes);
    kernel.ProfileWorkGroups(n, 3 * bytes);*/
  /*
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

    std::string fname = kernel.GetFunctionName();
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
