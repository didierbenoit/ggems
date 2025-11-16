#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

using namespace ggems::units;

namespace ggems::ocl {

/* ---------------------------------------------------------------------------*/

GGEMSOpenCLKernel::GGEMSOpenCLKernel(GGEMSOpenCLContext &ctx, cl::Kernel kernel,
                                     std::string kernel_name)
    : context_(ctx), kernel_(std::move(kernel)),
      kernel_name_(std::move(kernel_name)) {
  GGEMS_INFOEX("OpenCL", 2, "Created kernel '{}'", kernel_name_);
}

/* -------------------------------------------------------------------------- */

GGEMSOpenCLKernel::~GGEMSOpenCLKernel() noexcept {
  GGEMS_INFOEX("OpenCL", 2, "Destroying kernel '{}'", kernel_name_);
}

/* -------------------------------------------------------------------------- */

void GGEMSOpenCLKernel::SetArgSVMPointer(cl_uint index, void *ptr,
                                         GGEMSOpenCLSVMBuffer *owner) {
  cl_int err = clSetKernelArgSVMPointer(kernel_(), index, ptr);
  GGEMS_OCL_CHECK(err, std::format("Failed to set SVM arg {}", index));
}

/* -------------------------------------------------------------------------- */

void GGEMSOpenCLKernel::Run(std::array<std::size_t, 1> const &global,
                            std::array<std::size_t, 1> const &local) {
  cl::Event evt;

  auto &queue = context_.GetCommandQueueNative();

  cl_int err =
      queue.enqueueNDRangeKernel(kernel_, cl::NullRange, cl::NDRange(global[0]),
                                 cl::NDRange(local[0]), nullptr, &evt);
  GGEMS_OCL_CHECK(err,
                  std::format("Failed to enqueue kernel '{}'", kernel_name_));

  queue.finish();
}

/* -------------------------------------------------------------------------- */

GGEMSKernelExecutionStats
GGEMSOpenCLKernel::ProfiledEnqueue(cl::NDRange global, cl::NDRange local,
                                   Bytes bytes_moved) const {
  cl::Event ev;

  auto &queue = context_.GetCommandQueueNative();

  cl_int err =
      queue.enqueueNDRangeKernel(kernel_, cl::NullRange, cl::NDRange(global),
                                 cl::NDRange(local), nullptr, &ev);
  GGEMS_OCL_CHECK(err, std::format("enqueueNDRangeKernel failed"));

  queue.finish();

  GGEMSKernelExecutionStats stats;
  stats.kernel_name = kernel_name_;
  stats.device_name = context_.GetDevice().GetName();

  stats.global_work_items = global[0];
  stats.local_work_size = (local[0] == 0 ? 1 : local[0]);
  stats.bytes_moved = bytes_moved;

  // --- timestamps ---------------------------------------------------------
  stats.time_queued = ev.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>() * 1_ns;
  stats.time_submit = ev.getProfilingInfo<CL_PROFILING_COMMAND_SUBMIT>() * 1_ns;
  stats.time_start = ev.getProfilingInfo<CL_PROFILING_COMMAND_START>() * 1_ns;
  stats.time_end = ev.getProfilingInfo<CL_PROFILING_COMMAND_END>() * 1_ns;

  stats.kernel_time = stats.time_end - stats.time_start;
  stats.wall_time = stats.time_end - stats.time_queued;

  if (stats.bytes_moved.value > 0 && stats.kernel_time.value > 0) {
    stats.bandwidth = stats.bytes_moved / stats.kernel_time;
  }

  // --- log ---------------------------------------------------------------
  GGEMS_INFO("OpenCL", "Device {}: {} n={} took {} → {}", stats.device_name,
             stats.kernel_name, stats.global_work_items,
             HumanReadable(stats.kernel_time), HumanReadable(stats.bandwidth));

  return stats;
}

/* -------------------------------------------------------------------------- */

std::vector<GGEMSKernelExecutionStats>
GGEMSOpenCLKernel::ProfileWorkGroups(std::size_t global_size,
                                     Bytes bytes_moved) const {
  std::vector<GGEMSKernelExecutionStats> results;
  results.reserve(6);

  // Work-group sizes courants
  std::array<std::size_t, 6> candidates = {1, 32, 64, 128, 256, 512};
  auto const &dev = context_.GetDevice();

  for (auto wg : candidates) {
    if (wg > dev.GetMaxWorkGroupSize())
      continue;

    GGEMSKernelExecutionStats stats =
        ProfiledEnqueue(cl::NDRange(global_size), cl::NDRange(wg), bytes_moved);

    results.push_back(stats);
  }

  return results;
}

/* -------------------------------------------------------------------------- */

std::vector<GGEMSKernelExecutionStats>
GGEMSOpenCLKernel::ProfileBandwidthSweep(std::vector<std::size_t> const &sizes,
                                         Bytes bytes_per_item) const {
  std::vector<GGEMSKernelExecutionStats> results;

  for (auto n : sizes) {
    Bytes bytes = n * bytes_per_item;

    GGEMSKernelExecutionStats stats =
        ProfiledEnqueue(cl::NDRange(n), cl::NDRange(256), bytes);

    results.push_back(stats);
  }

  return results;
}

/* -------------------------------------------------------------------------- */

Time GGEMSOpenCLKernel::ProfileDriverOverhead() const {
  cl::Event ev;

  auto &queue = context_.GetCommandQueueNative();

  cl_int err = queue.enqueueNDRangeKernel(
      kernel_, cl::NullRange, cl::NDRange(1), cl::NDRange(1), nullptr, &ev);
  GGEMS_OCL_CHECK(err, std::format("Enqueue empty kernel failed."));

  queue.finish();

  Time queued = ev.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>() * 1_ns;
  Time start = ev.getProfilingInfo<CL_PROFILING_COMMAND_START>() * 1_ns;

  return (start - queued);
}

} // namespace ggems::ocl
