#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include <iterator>

namespace ggems::ocl {

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

GGEMSOpenCLKernel::GGEMSOpenCLKernel(GGEMSOpenCLContext &ctx, cl::Kernel kernel,
                                     std::string kernel_name)
    : context_(ctx), kernel_(std::move(kernel)),
      kernel_name_(std::move(kernel_name)) {
  GGEMS_INFOEX("OpenCL", 2, "Created kernel '{}'", kernel_name_);
}

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

GGEMSOpenCLKernel::~GGEMSOpenCLKernel() noexcept {
  GGEMS_INFOEX("OpenCL", 2, "Destroying kernel '{}'", kernel_name_);
}

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

template <typename T>
void GGEMSOpenCLKernel::SetArg(cl_uint idx, T const &val) {
  kernel_.setArg(idx, val);

  ArgInfo ai{};
  ai.index = idx;
  ai.value = val;
  ai.is_svm = false;

  args_.push_back(ai);
}

template void GGEMSOpenCLKernel::SetArg<int>(cl_uint, int const &);
template void GGEMSOpenCLKernel::SetArg<float>(cl_uint, float const &);
template void GGEMSOpenCLKernel::SetArg<double>(cl_uint, double const &);
template void GGEMSOpenCLKernel::SetArg<unsigned int>(cl_uint,
                                                      unsigned int const &);

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

void GGEMSOpenCLKernel::SetArgSVMPointer(cl_uint index, void *ptr,
                                         GGEMSOpenCLSVMBuffer *owner) {
  cl_int err = clSetKernelArgSVMPointer(kernel_(), index, ptr);
  GGEMS_OCL_CHECK(err, std::format("Failed to set SVM arg {}", index));

  ArgInfo ai{};
  ai.index = index;
  ai.value = ptr;
  ai.svm_owner = owner;
  ai.is_svm = true;

  args_.push_back(ai);
}

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

/*void GGEMSOpenCLKernel::MapSVMsIfNeeded() {
  for (auto &arg : args_) {
    if (!arg.is_svm || !arg.svm_owner)
      continue;

    if (arg.svm_owner->NeedsMap(context_)) {
      arg.svm_owner->Map(context_);
    }
  }
}*/

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

/*void GGEMSOpenCLKernel::UnmapSVMsIfNeeded() {
  for (auto &arg : args_) {
    if (!arg.is_svm || !arg.svm_owner)
      continue;

    if (arg.svm_owner->NeedsMap(context_)) {
      arg.svm_owner->Unmap(context_);
    }
  }
}*/

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

// void GGEMSOpenCLKernel::ApplyArgs() {}

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

void GGEMSOpenCLKernel::Run(std::array<size_t, 1> global_size,
                            std::array<size_t, 1> local_size) {
  cl::Event evt;

  auto &queue = context_.GetCommandQueueNative();

  cl_int err = queue.enqueueNDRangeKernel(
      kernel_, cl::NullRange, cl::NDRange(global_size[0]),
      cl::NDRange(local_size[0]), nullptr, &evt);
  GGEMS_OCL_CHECK(err,
                  std::format("Failed to enqueue kernel '{}'", kernel_name_));

  queue.finish();

  // Profiling
  cl_ulong start = evt.getProfilingInfo<CL_PROFILING_COMMAND_START>();
  cl_ulong end = evt.getProfilingInfo<CL_PROFILING_COMMAND_END>();

  double ms = double(end - start) * 1e-6;
  LogExecution(ms);
}

/* ---------------------------------------------*/
/* ---------------------------------------------*/
/* ---------------------------------------------*/

void GGEMSOpenCLKernel::LogExecution(double ms) {
  double gb = static_cast<double>(3ULL * 1024 * sizeof(float)) / 1e9; // A+B+C
  double bw = gb / (ms / 1e3);                                        // GB/s
  GGEMS_INFOEX("OpenCL", 1,
               "Device {}: vec_add_svm n={} took {:.3f} ms → {:.1f} GB/s",
               context_.GetDevice().GetName(), 1024, ms, bw);
}
} // namespace ggems::ocl
