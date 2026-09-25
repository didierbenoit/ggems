#pragma once

#include <cstdint>

namespace ggems::core::particles {

/*!
 * \brief Describes the global primary identifiers reserved for one run.
 * \details Each primary is one source particle starting a transport history.
 * Its global identifier is global_history_offset plus its zero-based index
 * within this reservation. This value object contains no particle data.
 */
struct GGEMSPrimaryStreamRunView {
  std::uint64_t run_id{0ULL}; /*!< Run label supplied by the caller. */
  std::uint64_t source_primary_count{
    0ULL}; /*!< Number of reserved primaries. */
  std::uint64_t global_history_offset{
    0ULL}; /*!< First reserved global identifier. */
};

/*!
 * \brief Allocates consecutive global primary identifiers across runs.
 * \details A new stream is ready to use and starts at identifier zero.
 * Each successful reservation advances the stream, so its ranges are contiguous
 * and do not overlap. The caller supplies the primary count for each run.
 *
 * The value k_invalid_id_u64 is reserved for invalid identifiers and is never
 * allocated. A rejected reservation leaves the stream unchanged.
 * Run labels do not affect allocation and need not be consecutive or unique.
 * Calls on the same stream must be serialized by the caller.
 */
class GGEMSPrimaryStream {
public:
  /*!
   * \brief Creates a stream whose first reservation starts at identifier zero.
   */
  GGEMSPrimaryStream() = default;

  /*!
   * \brief Destroys the stream.
   */
  ~GGEMSPrimaryStream() = default;

  /*!
   * \brief Prevents copying the identifier allocation state.
   * \param other Stream that cannot be copied.
   */
  GGEMSPrimaryStream(GGEMSPrimaryStream const &other) = delete;

  /*!
   * \brief Prevents moving the identifier allocation state.
   * \param other Stream that cannot be moved.
   */
  GGEMSPrimaryStream(GGEMSPrimaryStream &&other) = delete;

  /*!
   * \brief Prevents replacing the stream through copy assignment.
   * \param other Stream that cannot be copied.
   * \return No value is returned because this operation is deleted.
   */
  auto operator=(GGEMSPrimaryStream const &other)
    -> GGEMSPrimaryStream & = delete;

  /*!
   * \brief Prevents replacing the stream through move assignment.
   * \param other Stream that cannot be moved.
   * \return No value is returned because this operation is deleted.
   */
  auto operator=(GGEMSPrimaryStream &&other) -> GGEMSPrimaryStream & = delete;

  /*!
   * \brief Reserves a consecutive range of global primary identifiers.
   * \param run_id Caller-provided run label, copied into the returned view.
   * \param primary_count Number of primary identifiers to reserve; must be
   * positive.
   * \return Run label, reserved count, and first global identifier of the
   * range.
   * \throws GGEMSRecoverable If primary_count is zero or exceeds the number of
   * remaining valid identifiers.
   * \details The last reserved identifier is global_history_offset plus
   * source_primary_count minus one. On success, the next reservation starts
   * immediately after this range. On failure, no identifiers are consumed.
   */
  [[nodiscard]] auto PrepareRun(std::uint64_t run_id,
                                std::uint64_t primary_count)
    -> GGEMSPrimaryStreamRunView;

private:
  /*! \brief Next available identifier, or k_invalid_id_u64 when exhausted. */
  std::uint64_t next_global_primary_id_{
    0ULL}; /*!<Next available identifier, or k_invalid_id_u64 when exhausted*/
};

} // namespace ggems::core::particles
