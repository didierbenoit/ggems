#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"

namespace ggems::ocl {
class GGEMSOpenCLContext {
public:
  //! Construct a context from a device.
  explicit GGEMSOpenCLContext(GGEMSOpenCLDevice const &device);

  //! Destructor.
  ~GGEMSOpenCLContext();

  //! No copy (contexts are unique).
  GGEMSOpenCLContext(GGEMSOpenCLContext const &) = delete;
  GGEMSOpenCLContext &operator=(GGEMSOpenCLContext const &) = delete;

  //! Allow move.
  GGEMSOpenCLContext(GGEMSOpenCLContext &&) noexcept = default;
  GGEMSOpenCLContext &operator=(GGEMSOpenCLContext &&) noexcept = default;

private:
  void CreateContext();
  void CreateGLSharedContext();
  void CreateCommandQueue();

private:
  GGEMSOpenCLDevice device_;
  cl::Context context_;
  cl::CommandQueue command_queue_;
};
} // namespace ggems::ocl
