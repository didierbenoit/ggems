#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

namespace ggems::ocl {

GGEMSOpenCLContext::GGEMSOpenCLContext(GGEMSOpenCLDevice const &device)
    : device_{device} {
  GGEMS_INFOEX("OpenCL", 2, "Allocating GGEMSOpenCLContext: {}",
               device.GetName());

  CreateContext();
  CreateCommandQueue();

  GGEMS_INFOEX("OpenCL", 2, "GGEMSOpenCLContext allocated.");
}

/* ------------------------------------------------------------------------- */

GGEMSOpenCLContext::~GGEMSOpenCLContext() {
  GGEMS_INFOEX("OpenCL", 2, "Destroying OpenCL context for device: {}",
               device_.GetName());
}

void GGEMSOpenCLContext::CreateContext() {
  GGEMS_INFO("OpenCL", "Creating compute OpenCL context for device: {}",
             device_.GetName());

  cl_int err{CL_SUCCESS};

  // --- Standard compute-only context (no OpenGL interop) --------------------
  cl_context_properties props[] = {
      CL_CONTEXT_PLATFORM,
      reinterpret_cast<cl_context_properties>(device_.GetPlatformID()), 0};

  context_ = cl::Context({device_.GetNative()}, // devices
                         props,                 // context properties
                         nullptr,               // notification callback
                         nullptr,               // user data
                         &err);

  CheckCLError(err, "Failed to create OpenCL context");

  GGEMS_INFOEX("OpenCL", 2, "OpenCL context created.");
}

void GGEMSOpenCLContext::CreateCommandQueue() {
  GGEMS_INFO("OpenCL", "Creating command queue...");

  cl_int err{CL_SUCCESS};

  cl_command_queue_properties props = 0;
  props |= CL_QUEUE_PROFILING_ENABLE; // needed for profiling

  command_queue_ = cl::CommandQueue(context_, device_.GetNative(), props, &err);

  CheckCLError(err, "Failed to create command queue.");

  GGEMS_INFOEX("OpenCL", 2, "Command queue created (profiling enabled).");
}

/* ------------------------------------------------------------------------- */
} // namespace ggems::ocl
