#pragma once

/// \cond
#include <variant>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

namespace ggems::ocl {
class GGEMSOpenCLKernel {
public:
  GGEMSOpenCLKernel(GGEMSOpenCLContext &ctx, cl::Kernel kernel,
                    std::string kernel_name);

  ~GGEMSOpenCLKernel() noexcept;

  GGEMSOpenCLKernel(GGEMSOpenCLKernel const &) = delete;
  GGEMSOpenCLKernel &operator=(GGEMSOpenCLKernel const &) = delete;

  GGEMSOpenCLKernel(GGEMSOpenCLKernel &&) noexcept = delete;
  GGEMSOpenCLKernel &operator=(GGEMSOpenCLKernel &&) noexcept = delete;

public:
  // Ajout d'arguments
  template <typename T> void SetArg(cl_uint index, T const &value);

  void SetArgSVMPointer(cl_uint index, void *ptr, GGEMSOpenCLSVMBuffer *owner);

  // Exécution simple (1D pour l’instant)
  void Run(std::array<size_t, 1> global_size, std::array<size_t, 1> local_size);

private:
  // void ApplyArgs();
  // void MapSVMsIfNeeded();
  // void UnmapSVMsIfNeeded();
  void LogExecution(double ms);

private:
  struct ArgInfo {
    cl_uint index{};
    std::variant<void *,                           // SVM pointer
                 int, float, double, unsigned int, // scalaires classiques
                 cl_mem                            // buffers OpenCL classiques
                 >
        value;

    GGEMSOpenCLSVMBuffer *svm_owner = nullptr; // pour auto Map/Unmap
    bool is_svm = false;
  };

private:
  GGEMSOpenCLContext &context_;
  cl::Kernel kernel_;
  std::string kernel_name_;
  std::vector<ArgInfo> args_;
};
} // namespace ggems::ocl
