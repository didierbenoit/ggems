// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Implements the Apple OpenCL SVM compatibility layer.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#define GGEMS_OPENCL_C_API_ONLY
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#undef GGEMS_OPENCL_C_API_ONLY

/// \cond
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include <unistd.h>
/// \endcond

namespace {

/*!
 * \brief Maximum alignment accepted by the Apple SVM compatibility layer.
 */
constexpr std::size_t max_requested_alignment = 16U * sizeof(cl_long);

/*!
 * \brief OpenCL full-profile long16 size used to verify the alignment limit.
 */
constexpr std::size_t full_profile_long16_size = 128U;

static_assert(max_requested_alignment == full_profile_long16_size);

/*!
 * \brief Stores one public SVM pointer and its backing mapped pointer.
 */
struct Mapping {
  void *public_pointer{}; /*!< Public pointer supplied to the SVM API. */
  void *actual_pointer{}; /*!< Pointer returned by the backing buffer map. */
};

/*!
 * \brief Owns resources and mapping state for one emulated SVM allocation.
 */
struct AllocationState {
  /*!
   * \brief Constructs an allocation state.
   *
   * \param[in] allocation_context OpenCL context owning the allocation.
   * \param[in] allocation_buffer Backing OpenCL buffer.
   * \param[in] allocation_pointer Public host pointer.
   * \param[in] allocation_size Allocation size in bytes.
   */
  AllocationState(cl_context allocation_context, cl_mem allocation_buffer,
                  void *allocation_pointer,
                  std::size_t allocation_size) noexcept
      : context(allocation_context), buffer(allocation_buffer),
        base_pointer(allocation_pointer), size(allocation_size) {}

  /*!
   * \brief Releases the backing OpenCL buffer when ownership remains enabled.
   */
  ~AllocationState() {
    if (release_buffer.load(std::memory_order_relaxed) && buffer != nullptr) {
      // clSVMFree has no error return. If this release fails, retaining the
      // buffer and its destructor-owned host storage is safer than freeing the
      // CL_MEM_USE_HOST_PTR backing while OpenCL may still reference it.
      (void)clReleaseMemObject(buffer);
    }
  }

  /*!
   * \brief Disables copy construction.
   */
  AllocationState(AllocationState const &) = delete;

  /*!
   * \brief Disables copy assignment.
   */
  auto operator=(AllocationState const &) -> AllocationState & = delete;

  /*!
   * \brief Disables move construction.
   */
  AllocationState(AllocationState &&) = delete;

  /*!
   * \brief Disables move assignment.
   */
  auto operator=(AllocationState &&) -> AllocationState & = delete;

  /*!
   * \brief Marks the allocation unusable and optionally preserves its resources.
   *
   * \param[in] preserve_resources Whether destruction must retain the backing buffer.
   */
  auto Poison(bool preserve_resources) noexcept -> void {
    usable.store(false, std::memory_order_release);

    if (preserve_resources) {
      release_buffer.store(false, std::memory_order_release);
    }
  }

  cl_context context{}; /*!< OpenCL context owning the allocation. */
  cl_mem buffer{};     /*!< Backing OpenCL buffer. */
  void *base_pointer{}; /*!< Public host pointer exposed as the SVM allocation. */
  std::size_t size{};   /*!< Allocation size in bytes. */
  std::mutex mappings_mutex;              /*!< Mutex protecting mapping records. */
  std::list<Mapping> mappings;              /*!< Active mapped regions. */
  std::atomic<bool> usable{true};            /*!< Whether the allocation may be used. */
  std::atomic<bool> release_buffer{true};    /*!< Whether destruction releases the buffer. */
};

/*!
 * \brief Stores an allocation match and byte offset for a pointer lookup.
 */
struct AllocationMatch {
  std::shared_ptr<AllocationState> allocation; /*!< Matched allocation state. */
  std::size_t offset{};                        /*!< Byte offset from the allocation base. */
};

/*!
 * \brief Owns one temporary OpenCL event.
 */
class EventHandle {
public:
  /*!
   * \brief Constructs an empty event handle.
   */
  EventHandle() = default;

  /*!
   * \brief Releases the owned OpenCL event.
   */
  ~EventHandle() {
    if (event_ != nullptr) {
      (void)clReleaseEvent(event_);
    }
  }

  /*!
   * \brief Disables copy construction.
   */
  EventHandle(EventHandle const &) = delete;

  /*!
   * \brief Disables copy assignment.
   */
  auto operator=(EventHandle const &) -> EventHandle & = delete;

  /*!
   * \brief Disables move construction.
   */
  EventHandle(EventHandle &&) = delete;

  /*!
   * \brief Disables move assignment.
   */
  auto operator=(EventHandle &&) -> EventHandle & = delete;

  /*!
   * \brief Returns the address used to receive an OpenCL event.
   *
   * \return Address of the owned event handle.
   */
  [[nodiscard]] auto Address() noexcept -> cl_event * { return &event_; }

  /*!
   * \brief Returns the owned OpenCL event.
   *
   * \return Owned OpenCL event, or null when empty.
   */
  [[nodiscard]] auto Get() const noexcept -> cl_event { return event_; }

  /*!
   * \brief Releases ownership of the OpenCL event.
   *
   * \return Previously owned OpenCL event.
   */
  [[nodiscard]] auto ReleaseOwnership() noexcept -> cl_event {
    return std::exchange(event_, nullptr);
  }

private:
  cl_event event_{}; /*!< Owned OpenCL event. */
};

/*!
 * \brief Owns one temporary OpenCL memory object.
 */
class MemObjectHandle {
public:
  /*!
   * \brief Constructs a memory-object handle.
   *
   * \param[in] memory_object OpenCL memory object to own.
   */
  explicit MemObjectHandle(cl_mem memory_object) noexcept
      : memory_object_(memory_object) {}

  /*!
   * \brief Releases the owned OpenCL memory object.
   */
  ~MemObjectHandle() {
    if (memory_object_ != nullptr) {
      (void)clReleaseMemObject(memory_object_);
    }
  }

  /*!
   * \brief Disables copy construction.
   */
  MemObjectHandle(MemObjectHandle const &) = delete;

  /*!
   * \brief Disables copy assignment.
   */
  auto operator=(MemObjectHandle const &) -> MemObjectHandle & = delete;

  /*!
   * \brief Disables move construction.
   */
  MemObjectHandle(MemObjectHandle &&) = delete;

  /*!
   * \brief Disables move assignment.
   */
  auto operator=(MemObjectHandle &&) -> MemObjectHandle & = delete;

  /*!
   * \brief Returns the owned OpenCL memory object.
   *
   * \return Owned OpenCL memory object.
   */
  [[nodiscard]] auto Get() const noexcept -> cl_mem { return memory_object_; }

  /*!
   * \brief Releases ownership of the OpenCL memory object.
   *
   * \return Previously owned OpenCL memory object.
   */
  [[nodiscard]] auto ReleaseOwnership() noexcept -> cl_mem {
    return std::exchange(memory_object_, nullptr);
  }

private:
  cl_mem memory_object_{}; /*!< Owned OpenCL memory object. */
};

/*!
 * \brief Protects the process-wide emulated SVM allocation registry.
 */
std::mutex allocations_mutex;

/*!
 * \brief Maps public allocation base addresses to their allocation state.
 */
std::map<std::uintptr_t, std::shared_ptr<AllocationState>> allocations;

/*!
 * \brief Releases host storage when the backing OpenCL buffer is destroyed.
 *
 * \param[in] memory OpenCL memory object being destroyed.
 * \param[in] user_data Host pointer registered for destruction.
 */
void CL_CALLBACK FreeHostPointer(cl_mem memory, void *user_data) noexcept {
  (void)memory;
  std::free(user_data);
}

/*!
 * \brief Checks whether a size is a nonzero power of two.
 *
 * \param[in] value Value to test.
 * \return True when the value is a power of two.
 */
[[nodiscard]] auto IsPowerOfTwo(std::size_t value) noexcept -> bool {
  return value != 0 && (value & (value - 1U)) == 0;
}

/*!
 * \brief Returns a usable system page size or the compatibility fallback.
 *
 * \return Effective page size in bytes.
 */
[[nodiscard]] auto GetPageSize() noexcept -> std::size_t {
  auto const fallback_alignment =
      std::max(max_requested_alignment, alignof(std::max_align_t));
  auto const system_page_size = ::sysconf(_SC_PAGESIZE);

  if (system_page_size <= 0) {
    return fallback_alignment;
  }

  auto const page_size = static_cast<std::size_t>(system_page_size);

  if (!IsPowerOfTwo(page_size) || page_size % sizeof(void *) != 0) {
    return fallback_alignment;
  }

  return page_size;
}

/*!
 * \brief Computes the host allocation alignment for an SVM request.
 *
 * \param[in] requested_alignment Requested OpenCL SVM alignment.
 * \return Effective host alignment in bytes.
 */
[[nodiscard]] auto GetEffectiveAlignment(cl_uint requested_alignment) noexcept
    -> std::size_t {
  auto const svm_alignment =
      requested_alignment == 0 ? max_requested_alignment
                               : static_cast<std::size_t>(requested_alignment);

  return std::max(GetPageSize(), svm_alignment);
}

/*!
 * \brief Translates supported SVM flags to backing-buffer flags.
 *
 * \param[in] svm_flags Requested SVM allocation flags.
 * \param[out] buffer_flags Backing OpenCL buffer flags.
 * \return True when the requested flag combination is supported.
 */
[[nodiscard]] auto GetBufferFlags(cl_svm_mem_flags svm_flags,
                                  cl_mem_flags &buffer_flags) noexcept -> bool {
  constexpr cl_svm_mem_flags supported_flags =
      CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY;

  if ((svm_flags & ~supported_flags) != 0) {
    return false;
  }

  auto const read_write = (svm_flags & CL_MEM_READ_WRITE) != 0;
  auto const write_only = (svm_flags & CL_MEM_WRITE_ONLY) != 0;
  auto const read_only = (svm_flags & CL_MEM_READ_ONLY) != 0;
  auto const access_flag_count = static_cast<unsigned int>(read_write) +
                                 static_cast<unsigned int>(write_only) +
                                 static_cast<unsigned int>(read_only);

  if (access_flag_count > 1U) {
    return false;
  }

  buffer_flags = static_cast<cl_mem_flags>(svm_flags & supported_flags);
  if (buffer_flags == 0) {
    buffer_flags = CL_MEM_READ_WRITE;
  }

  buffer_flags |= CL_MEM_USE_HOST_PTR;
  return true;
}

/*!
 * \brief Finds the smallest maximum allocation size across a context's devices.
 *
 * \param[in] context OpenCL context to inspect.
 * \param[out] max_allocation_size Smallest device allocation limit in bytes.
 * \return True when the context and device limits were queried successfully.
 */
[[nodiscard]] auto
GetContextMaxAllocationSize(cl_context context,
                            std::size_t &max_allocation_size) noexcept -> bool {
  try {
    std::size_t device_list_size = 0;
    auto error = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, nullptr,
                                  &device_list_size);

    if (error != CL_SUCCESS || device_list_size == 0 ||
        device_list_size % sizeof(cl_device_id) != 0) {
      return false;
    }

    std::vector<cl_device_id> devices(device_list_size / sizeof(cl_device_id));

    error = clGetContextInfo(context, CL_CONTEXT_DEVICES, device_list_size,
                             static_cast<void *>(devices.data()), nullptr);

    if (error != CL_SUCCESS) {
      return false;
    }

    auto smallest_limit = std::numeric_limits<std::size_t>::max();

    for (auto *const device : devices) {
      cl_ulong device_limit = 0;
      error = clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                              sizeof(device_limit), &device_limit, nullptr);

      if (error != CL_SUCCESS) {
        return false;
      }

      auto const representable_limit =
          device_limit > std::numeric_limits<std::size_t>::max()
              ? std::numeric_limits<std::size_t>::max()
              : static_cast<std::size_t>(device_limit);

      smallest_limit = std::min(smallest_limit, representable_limit);
    }

    max_allocation_size = smallest_limit;
    return true;
  } catch (...) {
    return false;
  }
}

/*!
 * \brief Releases an unregistered backing buffer and host pointer when safe.
 *
 * \param[in] buffer Backing OpenCL buffer, or null.
 * \param[in] host_pointer Host allocation backing the buffer.
 */
auto ReleaseUnregisteredBufferAndHostPointer(cl_mem buffer,
                                             void *host_pointer) noexcept
    -> void {
  if (buffer == nullptr) {
    std::free(host_pointer);
    return;
  }

  if (clReleaseMemObject(buffer) == CL_SUCCESS) {
    std::free(host_pointer);
  }
  // If release fails, intentionally preserve both resources. Freeing the host
  // pointer while the CL_MEM_USE_HOST_PTR object may survive would be unsafe.
}

/*!
 * \brief Validates an OpenCL event wait-list and output-event combination.
 *
 * \param[in] num_events_in_wait_list Number of wait-list events.
 * \param[in] event_wait_list Input event wait list.
 * \param[out] event Optional output event location.
 * \return OpenCL status code.
 */
[[nodiscard]] auto ValidateEventArguments(cl_uint num_events_in_wait_list,
                                          cl_event const *event_wait_list,
                                          cl_event *event) noexcept -> cl_int {
  if (event_wait_list == nullptr) {
    return num_events_in_wait_list == 0 ? CL_SUCCESS
                                        : CL_INVALID_EVENT_WAIT_LIST;
  }

  if (num_events_in_wait_list == 0) {
    return CL_INVALID_EVENT_WAIT_LIST;
  }

  if (event == nullptr) {
    return CL_SUCCESS;
  }

  auto const wait_list_begin =
      reinterpret_cast<std::uintptr_t>(event_wait_list);
  auto const event_begin = reinterpret_cast<std::uintptr_t>(event);
  auto const max_address = std::numeric_limits<std::uintptr_t>::max();
  auto const event_size = sizeof(cl_event);

  if (num_events_in_wait_list > (max_address - wait_list_begin) / event_size ||
      event_begin > max_address - event_size) {
    return CL_INVALID_EVENT_WAIT_LIST;
  }

  auto const wait_list_end =
      wait_list_begin +
      (static_cast<std::uintptr_t>(num_events_in_wait_list) * event_size);
  auto const event_end = event_begin + event_size;

  if (event_begin < wait_list_end && wait_list_begin < event_end) {
    return CL_INVALID_EVENT_WAIT_LIST;
  }

  return CL_SUCCESS;
}

/*!
 * \brief Finds the emulated SVM allocation containing a pointer range.
 *
 * \param[in] pointer First byte of the requested range.
 * \param[in] range_size Requested range size in bytes.
 * \return Matching allocation and offset, or an empty match.
 */
[[nodiscard]] auto FindAllocationContaining(void *pointer,
                                            std::size_t range_size)
    -> AllocationMatch {
  auto const address = reinterpret_cast<std::uintptr_t>(pointer);
  AllocationMatch match;

  {
    std::scoped_lock lock(allocations_mutex);
    auto iterator = allocations.upper_bound(address);

    if (iterator == allocations.begin()) {
      return {};
    }

    --iterator;
    auto const base_address = iterator->first;
    auto const &allocation = iterator->second;
    auto const offset = address - base_address;

    if (offset >= allocation->size || range_size > allocation->size - offset) {
      return {};
    }

    match.allocation = allocation;
    match.offset = static_cast<std::size_t>(offset);
  }

  return match;
}

/*!
 * \brief Maps backing-buffer kernel-argument errors to public SVM errors.
 *
 * \param[in] error Backing OpenCL error code.
 * \return Public SVM-compatible error code.
 */
[[nodiscard]] auto NormalizeKernelArgumentError(cl_int error) noexcept
    -> cl_int {
  switch (error) {
  case CL_SUCCESS:
  case CL_INVALID_KERNEL:
  case CL_INVALID_ARG_INDEX:
  case CL_INVALID_ARG_VALUE:
  case CL_OUT_OF_RESOURCES:
  case CL_OUT_OF_HOST_MEMORY:
    return error;
  default:
    return CL_INVALID_ARG_VALUE;
  }
}

/*!
 * \brief Maps backing-buffer map errors to public SVM-map errors.
 *
 * \param[in] error Backing OpenCL error code.
 * \return Public SVM-compatible error code.
 */
[[nodiscard]] auto NormalizeMapError(cl_int error) noexcept -> cl_int {
  switch (error) {
  case CL_SUCCESS:
  case CL_INVALID_COMMAND_QUEUE:
  case CL_INVALID_OPERATION:
  case CL_INVALID_CONTEXT:
  case CL_INVALID_VALUE:
  case CL_INVALID_EVENT_WAIT_LIST:
  case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
  case CL_OUT_OF_RESOURCES:
  case CL_OUT_OF_HOST_MEMORY:
    return error;
  case CL_MEM_OBJECT_ALLOCATION_FAILURE:
  case CL_MAP_FAILURE:
  case CL_INVALID_MEM_OBJECT:
  case CL_MISALIGNED_SUB_BUFFER_OFFSET:
  default:
    // These are buffer-backend failures with no public SVM-map equivalent.
    return CL_OUT_OF_RESOURCES;
  }
}

/*!
 * \brief Maps backing-buffer unmap errors to public SVM-unmap errors.
 *
 * \param[in] error Backing OpenCL error code.
 * \return Public SVM-compatible error code.
 */
[[nodiscard]] auto NormalizeUnmapError(cl_int error) noexcept -> cl_int {
  switch (error) {
  case CL_SUCCESS:
  case CL_INVALID_COMMAND_QUEUE:
  case CL_INVALID_OPERATION:
  case CL_INVALID_CONTEXT:
  case CL_INVALID_EVENT_WAIT_LIST:
  case CL_OUT_OF_RESOURCES:
  case CL_OUT_OF_HOST_MEMORY:
    return error;
  case CL_INVALID_MEM_OBJECT:
  case CL_INVALID_VALUE:
    // The public pointer and its mapped-pointer record were validated before
    // entering OpenCL, so either code denotes an internal backend mismatch.
    return CL_INVALID_OPERATION;
  case CL_MEM_OBJECT_ALLOCATION_FAILURE:
  default:
    return CL_OUT_OF_RESOURCES;
  }
}

/*!
 * \brief Enqueues a best-effort unmap after a rejected map result.
 *
 * \param[in] command_queue OpenCL command queue.
 * \param[in] buffer Backing OpenCL buffer.
 * \param[in] mapped_pointer Pointer returned by the backing map.
 * \param[in] map_event Event produced by the backing map.
 * \return True when cleanup was enqueued successfully.
 */
[[nodiscard]] auto EnqueueMapCleanup(cl_command_queue command_queue,
                                     cl_mem buffer, void *mapped_pointer,
                                     cl_event map_event) noexcept -> bool {
  if (map_event == nullptr) {
    return false;
  }

  EventHandle cleanup_event;
  auto const error =
      clEnqueueUnmapMemObject(command_queue, buffer, mapped_pointer, 1,
                              &map_event, cleanup_event.Address());

  return error == CL_SUCCESS;
}

/*!
 * \brief Poisons an allocation and attempts cleanup after a map invariant fails.
 *
 * \param[in] allocation Allocation whose mapping was rejected.
 * \param[in] command_queue OpenCL command queue.
 * \param[in] mapped_pointer Pointer returned by the backing map.
 * \param[in] map_event Event produced by the backing map.
 * \param[in] public_error Error returned through the public SVM API.
 * \return The supplied public error code.
 */
[[nodiscard]] auto
RejectMappedRegion(std::shared_ptr<AllocationState> const &allocation,
                   cl_command_queue command_queue, void *mapped_pointer,
                   cl_event map_event, cl_int public_error) noexcept -> cl_int {
  // A post-map invariant failure means that this backend is no longer safe to
  // use. Even a successfully enqueued cleanup can fail asynchronously, after
  // its event has been released, so permanently preserve the cl_mem and its
  // destructor-owned host pointer. The best-effort unmap below limits runtime
  // damage but is deliberately not treated as proof that P may be released.
  allocation->Poison(true);
  (void)EnqueueMapCleanup(command_queue, allocation->buffer, mapped_pointer,
                          map_event);

  return public_error;
}

/*!
 * \brief Allocates host-backed storage and a buffer for emulated SVM.
 *
 * \param[in] context OpenCL context owning the allocation.
 * \param[in] flags Requested SVM allocation flags.
 * \param[in] size Allocation size in bytes.
 * \param[in] alignment Requested alignment in bytes.
 * \return Public SVM pointer, or null when allocation fails.
 */
[[nodiscard]] auto SVMAllocImpl(cl_context context, cl_svm_mem_flags flags,
                                std::size_t size, cl_uint alignment) -> void * {
  if (context == nullptr || size == 0) {
    return nullptr;
  }

  if (alignment != 0 &&
      (!IsPowerOfTwo(alignment) || alignment > max_requested_alignment)) {
    return nullptr;
  }

  cl_mem_flags buffer_flags{};

  if (!GetBufferFlags(flags, buffer_flags)) {
    return nullptr;
  }

  std::size_t max_allocation_size = 0;

  if (!GetContextMaxAllocationSize(context, max_allocation_size) ||
      size > max_allocation_size) {
    return nullptr;
  }

  void *pointer = nullptr;
  auto const effective_alignment = GetEffectiveAlignment(alignment);

  if (::posix_memalign(&pointer, effective_alignment, size) != 0) {
    return nullptr;
  }

  cl_int error = CL_SUCCESS;
  auto *const buffer =
      clCreateBuffer(context, buffer_flags, size, pointer, &error);

  if (error != CL_SUCCESS || buffer == nullptr) {
    ReleaseUnregisteredBufferAndHostPointer(buffer, pointer);
    return nullptr;
  }

  error = clSetMemObjectDestructorCallback(buffer, FreeHostPointer, pointer);

  if (error != CL_SUCCESS) {
    ReleaseUnregisteredBufferAndHostPointer(buffer, pointer);
    return nullptr;
  }

  MemObjectHandle buffer_owner(buffer);
  auto allocation = std::make_shared<AllocationState>(
      context, buffer_owner.Get(), pointer, size);
  (void)buffer_owner.ReleaseOwnership();

  bool inserted = false;

  {
    std::scoped_lock lock(allocations_mutex);
    inserted =
        allocations
            .emplace(reinterpret_cast<std::uintptr_t>(pointer), allocation)
            .second;
  }

  if (!inserted) {
    allocation.reset();
    return nullptr;
  }

  return pointer;
}

/*!
 * \brief Releases an emulated SVM allocation registered for a context.
 *
 * \param[in] context OpenCL context owning the allocation.
 * \param[in] svm_pointer Allocation base pointer to release.
 */
auto SVMFreeImpl(cl_context context, void *svm_pointer) -> void {
  if (svm_pointer == nullptr) {
    return;
  }

  std::shared_ptr<AllocationState> allocation;

  {
    std::scoped_lock lock(allocations_mutex);
    auto const address = reinterpret_cast<std::uintptr_t>(svm_pointer);
    auto const iterator = allocations.find(address);

    if (iterator == allocations.end() || iterator->second->context != context) {
      return;
    }

    allocation = std::move(iterator->second);
    allocations.erase(iterator);
  }

  // Ensure clReleaseMemObject and the host-pointer destructor callback are not
  // invoked while allocations_mutex is held.
  allocation.reset();
}

/*!
 * \brief Maps an emulated SVM range through its backing OpenCL buffer.
 *
 * \param[in] command_queue OpenCL command queue.
 * \param[in] blocking_map Whether the map must complete before returning.
 * \param[in] map_flags OpenCL mapping flags.
 * \param[in,out] svm_pointer Public SVM pointer to map.
 * \param[in] size Mapped byte count.
 * \param[in] num_events_in_wait_list Number of wait-list events.
 * \param[in] event_wait_list Input event wait list.
 * \param[out] event Optional output event.
 * \return OpenCL status code.
 */
[[nodiscard]] auto
EnqueueSVMMapImpl(cl_command_queue command_queue, cl_bool blocking_map,
                  cl_map_flags map_flags, void *svm_pointer, std::size_t size,
                  cl_uint num_events_in_wait_list,
                  cl_event const *event_wait_list, cl_event *event) -> cl_int {
  if (blocking_map != CL_TRUE && blocking_map != CL_FALSE) {
    return CL_INVALID_VALUE;
  }

  if (svm_pointer == nullptr || size == 0) {
    return CL_INVALID_VALUE;
  }

  auto const match = FindAllocationContaining(svm_pointer, size);

  if (match.allocation == nullptr) {
    return CL_INVALID_VALUE;
  }

  auto const &allocation = match.allocation;

  if (!allocation->usable.load(std::memory_order_acquire)) {
    return CL_INVALID_OPERATION;
  }

  // Allocate the list node before the OpenCL map succeeds. Recording the map
  // after enqueue then uses no host allocation and list::splice cannot throw.
  std::list<Mapping> prepared_mapping;
  prepared_mapping.push_back(Mapping{
      .public_pointer = svm_pointer,
      .actual_pointer = nullptr,
  });

  EventHandle map_event;
  cl_int error = CL_SUCCESS;
  auto *const mapped_pointer =
      clEnqueueMapBuffer(command_queue, allocation->buffer, blocking_map,
                         map_flags, match.offset, size, num_events_in_wait_list,
                         event_wait_list, map_event.Address(), &error);

  if (error != CL_SUCCESS) {
    return NormalizeMapError(error);
  }

  auto *const expected_pointer =
      static_cast<std::byte *>(allocation->base_pointer) + match.offset;

  if (mapped_pointer != expected_pointer || map_event.Get() == nullptr) {
    return RejectMappedRegion(allocation, command_queue, mapped_pointer,
                              map_event.Get(), CL_INVALID_OPERATION);
  }

  prepared_mapping.front().actual_pointer = mapped_pointer;

  if (!allocation->usable.load(std::memory_order_acquire)) {
    return RejectMappedRegion(allocation, command_queue, mapped_pointer,
                              map_event.Get(), CL_INVALID_OPERATION);
  }

  try {
    std::scoped_lock lock(allocation->mappings_mutex);
    allocation->mappings.splice(allocation->mappings.end(), prepared_mapping);
  } catch (...) {
    return RejectMappedRegion(allocation, command_queue, mapped_pointer,
                              map_event.Get(), CL_OUT_OF_HOST_MEMORY);
  }

  if (event != nullptr) {
    *event = map_event.ReleaseOwnership();
  }

  return CL_SUCCESS;
}

/*!
 * \brief Unmaps an emulated SVM range through its backing OpenCL buffer.
 *
 * \param[in] command_queue OpenCL command queue.
 * \param[in,out] svm_pointer Public SVM pointer to unmap.
 * \param[in] num_events_in_wait_list Number of wait-list events.
 * \param[in] event_wait_list Input event wait list.
 * \param[out] event Optional output event.
 * \return OpenCL status code.
 */
[[nodiscard]] auto EnqueueSVMUnmapImpl(cl_command_queue command_queue,
                                       void *svm_pointer,
                                       cl_uint num_events_in_wait_list,
                                       cl_event const *event_wait_list,
                                       cl_event *event) -> cl_int {
  if (svm_pointer == nullptr) {
    return CL_INVALID_VALUE;
  }

  auto const match = FindAllocationContaining(svm_pointer, 0);

  if (match.allocation == nullptr) {
    return CL_INVALID_VALUE;
  }

  auto const &allocation = match.allocation;
  std::list<Mapping> detached_mapping;

  {
    std::scoped_lock lock(allocation->mappings_mutex);
    auto const mapping_iterator =
        std::find_if(allocation->mappings.begin(), allocation->mappings.end(),
                     [svm_pointer](Mapping const &mapping) -> bool {
                       return mapping.public_pointer == svm_pointer;
                     });

    if (mapping_iterator == allocation->mappings.end()) {
      return CL_INVALID_OPERATION;
    }

    detached_mapping.splice(detached_mapping.end(), allocation->mappings,
                            mapping_iterator);
  }

  EventHandle unmap_event;
  auto const error = clEnqueueUnmapMemObject(
      command_queue, allocation->buffer,
      detached_mapping.front().actual_pointer, num_events_in_wait_list,
      event_wait_list, unmap_event.Address());

  if (error != CL_SUCCESS) {
    try {
      std::scoped_lock lock(allocation->mappings_mutex);
      allocation->mappings.splice(allocation->mappings.end(), detached_mapping);
    } catch (...) {
     // The underlying mapping is still active but its record cannot be
      // restored. Preserve the OpenCL buffer and host backing indefinitely.
      allocation->Poison(true);
    }

    return NormalizeUnmapError(error);
  }

  if (event != nullptr && unmap_event.Get() == nullptr) {
    // The unmap was accepted but the backend did not provide the requested
    // completion object. Its asynchronous completion can no longer be tracked,
    // so preserve the buffer and host backing rather than permit an unsafe
    // free.
    allocation->Poison(true);
    return CL_OUT_OF_RESOURCES;
  }

  if (event != nullptr) {
    *event = unmap_event.ReleaseOwnership();
  }

  return CL_SUCCESS;
}

/*!
 * \brief Binds an emulated SVM allocation as a kernel buffer argument.
 *
 * \param[in] kernel OpenCL kernel.
 * \param[in] argument_index Kernel argument index.
 * \param[in] argument_value Public SVM allocation pointer.
 * \return OpenCL status code.
 */
[[nodiscard]] auto SetKernelArgSVMPointerImpl(cl_kernel kernel,
                                              cl_uint argument_index,
                                              void const *argument_value)
    -> cl_int {
  if (argument_value == nullptr) {
    cl_mem null_buffer = nullptr;
    return NormalizeKernelArgumentError(
        clSetKernelArg(kernel, argument_index, sizeof(cl_mem),
                       static_cast<void const *>(&null_buffer)));
  }

  auto const match =
      FindAllocationContaining(const_cast<void *>(argument_value), 0);

  if (match.allocation == nullptr || match.offset != 0) {
    return CL_INVALID_ARG_VALUE;
  }

  auto const &allocation = match.allocation;

  if (!allocation->usable.load(std::memory_order_acquire)) {
    return CL_INVALID_OPERATION;
  }

  auto *const argument_buffer = allocation->buffer;
  return NormalizeKernelArgumentError(
      clSetKernelArg(kernel, argument_index, sizeof(cl_mem),
                     static_cast<void const *>(&argument_buffer)));
}

} // namespace

extern "C" {

/*!
 * \brief Implements the Apple compatibility entry point for clSVMAlloc.
 *
 * \param[in] context OpenCL context owning the allocation.
 * \param[in] flags Requested SVM allocation flags.
 * \param[in] size Allocation size in bytes.
 * \param[in] alignment Requested alignment in bytes.
 * \return Public SVM pointer, or null when allocation fails.
 */
CL_API_ENTRY void *CL_API_CALL clSVMAlloc(cl_context context,
                                          cl_svm_mem_flags flags, size_t size,
                                          cl_uint alignment) {
  try {
    return SVMAllocImpl(context, flags, size, alignment);
  } catch (...) {
    return nullptr;
  }
}

/*!
 * \brief Implements the Apple compatibility entry point for clSVMFree.
 *
 * \param[in] context OpenCL context owning the allocation.
 * \param[in] svm_pointer Allocation base pointer to release.
 */
CL_API_ENTRY void CL_API_CALL clSVMFree(cl_context context, void *svm_pointer) {
  try {
    SVMFreeImpl(context, svm_pointer);
  } catch (...) {
    // clSVMFree has no error return. Never propagate a C++ exception through
    // the Khronos C ABI.
    return;
  }
}

/*!
 * \brief Implements the Apple compatibility entry point for clEnqueueSVMMap.
 *
 * \param[in] command_queue OpenCL command queue.
 * \param[in] blocking_map Whether the map must complete before returning.
 * \param[in] flags OpenCL mapping flags.
 * \param[in,out] svm_ptr Public SVM pointer to map.
 * \param[in] size Mapped byte count.
 * \param[in] num_events_in_wait_list Number of wait-list events.
 * \param[in] event_wait_list Input event wait list.
 * \param[out] event Optional output event.
 * \return OpenCL status code.
 */
CL_API_ENTRY cl_int CL_API_CALL clEnqueueSVMMap(
    cl_command_queue command_queue, cl_bool blocking_map, cl_map_flags flags,
    void *svm_ptr, size_t size, cl_uint num_events_in_wait_list,
    cl_event const *event_wait_list, cl_event *event) {
  auto const event_error =
      ValidateEventArguments(num_events_in_wait_list, event_wait_list, event);

  if (event_error != CL_SUCCESS) {
    return event_error;
  }

  if (event != nullptr) {
    *event = nullptr;
  }

  try {
    return EnqueueSVMMapImpl(command_queue, blocking_map, flags, svm_ptr, size,
                             num_events_in_wait_list, event_wait_list, event);
  } catch (...) {
    return CL_OUT_OF_HOST_MEMORY;
  }
}

/*!
 * \brief Implements the Apple compatibility entry point for clEnqueueSVMUnmap.
 *
 * \param[in] command_queue OpenCL command queue.
 * \param[in,out] svm_ptr Public SVM pointer to unmap.
 * \param[in] num_events_in_wait_list Number of wait-list events.
 * \param[in] event_wait_list Input event wait list.
 * \param[out] event Optional output event.
 * \return OpenCL status code.
 */
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMUnmap(cl_command_queue command_queue, void *svm_ptr,
                  cl_uint num_events_in_wait_list,
                  cl_event const *event_wait_list, cl_event *event) {
  auto const event_error =
      ValidateEventArguments(num_events_in_wait_list, event_wait_list, event);

  if (event_error != CL_SUCCESS) {
    return event_error;
  }

  if (event != nullptr) {
    *event = nullptr;
  }

  try {
    return EnqueueSVMUnmapImpl(command_queue, svm_ptr, num_events_in_wait_list,
                               event_wait_list, event);
  } catch (...) {
    return CL_OUT_OF_HOST_MEMORY;
  }
}

/*!
 * \brief Implements the Apple compatibility entry point for clSetKernelArgSVMPointer.
 *
 * \param[in] kernel OpenCL kernel.
 * \param[in] arg_index Kernel argument index.
 * \param[in] arg_value Public SVM allocation pointer.
 * \return OpenCL status code.
 */
CL_API_ENTRY cl_int CL_API_CALL clSetKernelArgSVMPointer(
    cl_kernel kernel, cl_uint arg_index, void const *arg_value) {
  try {
    return SetKernelArgSVMPointerImpl(kernel, arg_index, arg_value);
  } catch (...) {
    return CL_OUT_OF_HOST_MEMORY;
  }
}

} // extern "C"
