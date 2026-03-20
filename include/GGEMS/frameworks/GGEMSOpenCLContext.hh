#pragma once
// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSOpenCLContext.hh
 * \brief Declaration of GGEMSOpenCLContext providing creation and management
 *        of OpenCL contexts, command queues, SVM capabilities, and future
 *        interoperability features (OpenCL ↔ Vulkan).
 *
 * This class encapsulates all OpenCL context-level operations used by GGEMS.
 * It creates and manages the native cl::Context, command queue, and SVM
 * capability detection. The context serves as the central point of memory
 * allocation, queue submission, kernel setup, and (in future versions)
 * interoperability with external APIs such as Vulkan.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-12-08
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

namespace ggems::ocl {
using units::operator""_B;

/*!
 * \enum SVMMemoryKind
 * \brief Specifies the type of Shared Virtual Memory (SVM) desired for
 * allocation.
 *
 * The context uses the device's SVM capabilities to determine which allocation
 * is possible or optimal. The \c Auto mode selects the best supported type.
 */
enum class SVMMemoryKind {
  None,              /*!< No SVM support available. */
  Auto,              /*!< Automatically choose best-supported SVM mode. */
  CoarseGrainBuffer, /*!< Coarse-grain buffer SVM. */
  FineGrainBuffer,   /*!< Fine-grain buffer SVM. */
  FineGrainSystem    /*!< Fine-grain system-wide SVM. */
};

/*!
 * \struct SVMSupport
 * \brief Describes SVM features supported by the device.
 *
 * This structure groups coarse-grain, fine-grain, and atomic support flags.
 * Utility helpers provide quick detection of available modes and selection
 * of the best default memory kind.
 */
struct SVMSupport {
  bool coarse_grain_buffer{false}; /*!< Coarse-grain buffer SVM support. */
  bool fine_grain_buffer{false};   /*!< Fine-grain buffer SVM support. */
  bool fine_grain_system{false};   /*!< Fine-grain system SVM support. */
  bool atomics{false};             /*!< Atomic operations permitted. */

  /*!
   * \brief Returns the best SVM mode supported by the device.
   * \return Preferred SVM memory kind.
   */
  [[nodiscard]] SVMMemoryKind DefaultKind() const noexcept {
    if (fine_grain_system)
      return SVMMemoryKind::FineGrainSystem;
    if (fine_grain_buffer)
      return SVMMemoryKind::FineGrainBuffer;
    if (coarse_grain_buffer)
      return SVMMemoryKind::CoarseGrainBuffer;
    return SVMMemoryKind::None;
  }

  /*!
   * \brief Indicates whether any SVM capability is supported.
   * \return True if at least one SVM mode is available.
   */
  [[nodiscard]] bool HasAny() const noexcept {
    return coarse_grain_buffer || fine_grain_buffer || fine_grain_system;
  }
};

struct VRAMUsage {
  units::Bytes total{0_B};
  units::Bytes allocated{0_B};
  units::Bytes available{0_B};
  units::Bytes peak{0_B};
  std::size_t allocation_count{0};

  [[nodiscard]] std::uint8_t GetPercent() const noexcept {
    if (total.value == 0LL) {
      return 0U;
    }

    return static_cast<std::uint8_t>((100ULL * allocated.value) / total.value);
  }
};

/*!
 * \class GGEMSOpenCLContext
 * \brief High-level wrapper for an OpenCL execution context.
 *
 * This class creates and manages an OpenCL context, command queue, and SVM
 * support detection. It serves as the allocator for SVM buffers and performs
 * queue operations (map/unmap, pointer binding).
 *
 * Future versions of GGEMS will extend this class to include interoperability
 * with GPU graphics APIs (notably Vulkan) to enable shared buffers,
 * zero-copy compute, and unified graphics/compute workflows.
 */
class GGEMSOpenCLContext {
public:
  /*!
   * \brief Constructs an OpenCL context from a given device.
   *
   * This loads the native context, creates a command queue, and probes the
   * SVM capabilities of the target device. All memory operations and command
   * dispatch occur through this context instance.
   *
   * \param device The device used to initialise the context.
   */
  explicit GGEMSOpenCLContext(GGEMSOpenCLDevice const &device);

  /*!
   * \brief Default destructor.
   *
   * All resources are released automatically by RAII wrappers from cl.hpp.
   */
  ~GGEMSOpenCLContext() = default;

  /*!
   * \brief Copy constructor.
   *
   * Performs a shallow copy of the context, command queue, and SVM capability
   * information. All underlying OpenCL objects are reference-counted and remain
   * valid across copies.
   */
  GGEMSOpenCLContext(GGEMSOpenCLContext const &) = default;

  /*!
   * \brief Move constructor.
   *
   * Transfers ownership of the underlying OpenCL context, command queue, and
   * SVM metadata from \p other. The moved-from object is left in a valid but
   * unspecified state.
   */
  GGEMSOpenCLContext(GGEMSOpenCLContext &&) = default;

  GGEMSOpenCLContext &operator=(GGEMSOpenCLContext const &) = delete;
  GGEMSOpenCLContext &operator=(GGEMSOpenCLContext &&) = delete;

public:
  /*!
   * \brief Returns the native OpenCL context object.
   * \return cl::Context reference.
   */
  [[nodiscard]] cl::Context const &GetContextNative() const noexcept {
    return context_;
  }

  /*!
   * \brief Returns the device that owns this context.
   * \return Reference to the GGEMS device abstraction.
   */
  [[nodiscard]] GGEMSOpenCLDevice const &GetDevice() const noexcept {
    return device_;
  }

  /*!
   * \brief Returns the native OpenCL command queue.
   * \return cl::CommandQueue reference.
   */
  [[nodiscard]] cl::CommandQueue const &GetCommandQueueNative() const noexcept {
    return command_queue_;
  }

  /*!
   * \brief Returns SVM capability description for the device.
   * \return Reference to SVMSupport.
   */
  [[nodiscard]] SVMSupport const &GetSVMSupport() const noexcept {
    return svm_support_;
  }

  void RegisterSVMAllocation(units::Bytes size) noexcept;
  void RegisterSVMRelease(units::Bytes size) noexcept;

  [[nodiscard]] VRAMUsage const &GetVRAMUsage() const noexcept {
    return vram_usage_;
  }

  [[nodiscard]] units::Bytes GetTotalVRAM() const noexcept {
    return vram_usage_.total;
  }

  [[nodiscard]] units::Bytes GetAllocatedVRAM() const noexcept {
    return vram_usage_.allocated;
  }

  [[nodiscard]] units::Bytes GetAvailableVRAM() const noexcept {
    return vram_usage_.available;
  }

  [[nodiscard]] std::size_t GetPeakVRAM() const noexcept {
    return vram_usage_.allocation_count;
  }

  /*!
   * \brief Allocates an SVM buffer with the requested size and memory kind.
   *
   * \param size Size of the SVM allocation.
   * \param kind Requested SVM mode, defaults to automatic selection.
   * \param alignment Required alignment in bytes (0 = implementation default).
   * \return Newly created GGEMSOpenCLSVMBuffer.
   */
  [[nodiscard]] GGEMSOpenCLSVMBuffer
  CreateSVMBuffer(units::Bytes size, SVMMemoryKind kind = SVMMemoryKind::Auto,
                  units::Bytes alignment = 0_B);

  /*!
   * \brief Enqueues an SVM map operation for coarse-grain SVM hardware.
   *
   * \param ptr Pointer to SVM memory.
   * \param size Size of the mapped region.
   * \param flags Mapping flags (read/write).
   */
  void EnqueueSVMMap(void *ptr, units::Bytes size,
                     cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE) const;

  /*!
   * \brief Enqueues an SVM unmap operation for coarse-grain SVM devices.
   *
   * \param ptr Pointer to SVM memory.
   */
  void EnqueueSVMUnmap(void *ptr) const;

  /*!
   * \brief Assigns an SVM pointer to a kernel argument.
   *
   * \param kernel Target kernel.
   * \param index  Argument index.
   * \param ptr    Pointer to SVM memory.
   */
  void SetSVMPointer(cl::Kernel &kernel, cl_uint index, void *ptr) const;

  // ----- Context -----------------------------------

  /*!
   * \brief Returns the reference count of the native OpenCL context.
   * \return Reference count.
   */
  [[nodiscard]] cl_uint GetReferenceCount() const;

  /*!
   * \brief Returns the number of devices associated with the context.
   * \return Device count.
   */
  [[nodiscard]] cl_uint GetNumDevices() const;

  /*!
   * \brief Returns the native devices attached to this context.
   * \return Vector of cl::Device.
   */
  [[nodiscard]] std::vector<cl::Device> GetNativeDevices() const;

  /*!
   * \brief Returns the list of context properties used during creation.
   * \return Vector of cl_context_properties.
   */
  [[nodiscard]] std::vector<cl_context_properties> GetProperties() const;

  /*!
   * \brief Prints human-readable information about the context.
   */
  void PrintContext() const;

  /*!
   * \brief Prints details about the command queue associated with the context.
   */
  void PrintCommandQueue() const;

  // ----- Command Queue -----------------------------

  /*!
   * \brief Returns the native context used by the command queue.
   * \return cl::Context.
   */
  [[nodiscard]] cl::Context GetQueueContext() const;

  /*!
   * \brief Returns the device bound to the command queue.
   * \return cl::Device.
   */
  [[nodiscard]] cl::Device GetQueueDevice() const;

  /*!
   * \brief Returns the reference count of the command queue.
   * \return Reference count.
   */
  [[nodiscard]] cl_uint GetQueueReferenceCount() const;

  /*!
   * \brief Returns queue property bitfields.
   * \return Queue properties.
   */
  [[nodiscard]] cl_command_queue_properties GetQueueProperties() const;

  /*!
   * \brief Returns the queue properties as an array.
   * \return Vector of cl_queue_properties.
   */
  [[nodiscard]] std::vector<cl_queue_properties>
  GetQueuePropertiesArray() const;

  /*!
   * \brief Returns the queue size when applicable (profiling/OOO modes).
   * \return Queue size.
   */
  [[nodiscard]] cl_uint GetQueueSize() const;

private:
  /*!
   * \brief Creates the underlying OpenCL context.
   *
   * Extracts platform/device identifiers, builds the context, and prepares
   * for command queue creation.
   */
  void CreateContext();

  /*!
   * \brief Creates a context suitable for OpenGL sharing.
   *
   * This function acts as a placeholder for future shared interop (e.g.
   * Vulkan).
   */
  void CreateGLSharedContext();

  /*!
   * \brief Creates the OpenCL command queue associated with the context.
   */
  void CreateCommandQueue();

  /*!
   * \brief Determines the SVM capabilities for this device and fills
   * svm_support_.
   */
  void InitSVMSupport();

  void InitVRAMUsage();
  void UpdateVRAMUsage() noexcept;

private:
  GGEMSOpenCLDevice const &device_; /*!< Owning OpenCL device abstraction. */
  cl::Context context_;             /*!< Native OpenCL context. */
  cl::CommandQueue command_queue_;  /*!< Primary command queue. */
  SVMSupport svm_support_{};        /*!< SVM capability description. */
  VRAMUsage vram_usage_{};
};
} // namespace ggems::ocl
