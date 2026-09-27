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
 * \brief Authors source population, energy, shape, and pose settings.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <span>
#include <memory>
#include <string_view>
#include <optional>

#include "GGEMS/GGEMSTimeWindow.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/units/GGEMSActivityUnits.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"

namespace ggems::core::sources {

/*!
 * \brief Authors an analytic source and its primary-population law.
 *
 * A count-driven source emits a configured number of one particle kind using
 * one energy law. An activity-driven source uses a shared radionuclide
 * definition whose emission groups have independent Poisson counts.
 * FinalizeInitialization() locks population-mode and energy changes.
 * Count-driven counts, particle kind, shape, angular law, position, and
 * orientation remain mutable between runs. Callers must synchronize mutations
 * with snapshot creation and execution.
 *
 * The default is a point source at the origin: 4096 Gamma primaries at 511 keV,
 * directed along global +Z. Source frames and global focus coordinates are
 * independent of any future physical navigation geometry.
 */
class GGEMSSource {
public:
  /*! \brief Creates the default count-driven analytic source. */
  GGEMSSource() = default;

  /*! \brief Releases owned energy tables and shared radionuclide ownership. */
  ~GGEMSSource() = default;

  /*! \brief Copies source configuration, including its finalized state. */
  GGEMSSource(GGEMSSource const &) = default;

  /*!
   * \brief Transfers a source configuration before it is finalized.
   * \param[in,out] other Mutable source whose tables and population are moved.
   * \throws GGEMSRecoverable If other has been finalized.
   */
  GGEMSSource(GGEMSSource &&other);

  /*!
   * \brief Copies configuration into a mutable source.
   * \param[in] other Source to copy, including its finalized state.
   * \return This source.
   * \throws GGEMSRecoverable If the destination is finalized and not other.
   */
  auto operator=(GGEMSSource const &other) -> GGEMSSource &;

  /*!
   * \brief Transfers configuration between mutable sources.
   * \param[in,out] other Source whose tables and population are moved.
   * \return This source; self-assignment leaves it unchanged.
   * \throws GGEMSRecoverable If either distinct source is finalized.
   */
  auto operator=(GGEMSSource &&other) -> GGEMSSource &;

  /*!
   * \brief Updates the count used by subsequent count-driven runs.
   *
   * This remains available after initialization.
   *
   * \param[in] primary_count Requested primary count, including zero.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If this source is activity-driven.
   */
  auto SetPrimaryCount(std::uint64_t primary_count) -> GGEMSSource &;

  /*!
   * \brief Reads the configured count-driven population.
   *
   * \return Requested primaries for one run.
   * \throws GGEMSRecoverable If this source is activity-driven.
   */
  [[nodiscard]] auto GetPrimaryCount() const -> std::uint64_t;

  /*!
   * \brief Selects an explicit count before population mode is finalized.
   *
   * \param[in] primary_count Requested primaries for each run, including zero.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If initialization has been finalized.
   */
  auto SetCountDrivenPopulation(std::uint64_t primary_count) -> GGEMSSource &;

  /*!
   * \brief Selects an activity-driven population before initialization.
   *
   * Activity and chronology are consumed during planning; this setter checks
   * only the definition pointer and mutability.
   *
   * \param[in] radionuclide Non-null shared immutable emission definition.
   * \param[in] activity_at_reference_time Finite nonnegative parent activity in
   * Bq.
   * \param[in] reference_time_ps Reference simulation time in ps, no later than
   * the run start.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If finalized or the definition pointer is null.
   */
  auto SetRadionuclide(
    std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>
      radionuclide,
    units::Activity activity_at_reference_time, std::uint64_t reference_time_ps)
    -> GGEMSSource &;

  /*!
   * \brief Reports the active primary-population law.
   *
   * \return CountDriven or ActivityDriven according to the stored
   * configuration.
   */
  [[nodiscard]] auto GetPopulationMode() const noexcept
    -> GGEMSSourcePopulationMode;

  /*!
   * \brief Copies the radioactive population settings.
   *
   * \return Activity, reference time, and shared definition ownership.
   * \throws GGEMSRecoverable If this source is count-driven.
   */
  [[nodiscard]] auto BuildActivityDrivenPopulationConfiguration() const
    -> GGEMSActivityDrivenSourceConfiguration;

  /*!
   * \brief Checks schedule compatibility for an activity-driven source.
   *
   * \param[in] initial_time_window First source window, or no value in static
   * mode.
   * \throws GGEMSRecoverable If an activity-driven source lacks a
   * positive-duration window or its reference time follows the start.
   */
  auto ValidatePopulationForRunInitialization(
    std::optional<GGEMSTimeWindow> const &initial_time_window) const -> void;

  /*!
   * \brief Selects the currently implemented analytic source kind.
   *
   * \return This source for chained configuration.
   */
  auto SetAnalytic() noexcept -> GGEMSSource &;

  /*!
   * \brief Selects emission at the local source origin.
   *
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetPointEmission() -> GGEMSSource &;

  /*!
   * \brief Selects uniform emission over a local xy rectangle.
   *
   * \param[in] width_pm Positive full local x width in pm.
   * \param[in] height_pm Positive full local y height in pm.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetRectangleEmissionPicoMeter(std::uint64_t width_pm,
                                     std::uint64_t height_pm) -> GGEMSSource &;

  /*!
   * \brief Selects uniform emission over a local xy ellipse.
   *
   * \param[in] diameter_x_pm Positive full local x diameter in pm.
   * \param[in] diameter_y_pm Positive full local y diameter in pm.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetEllipseEmissionPicoMeter(std::uint64_t diameter_x_pm,
                                   std::uint64_t diameter_y_pm)
    -> GGEMSSource &;

  /*!
   * \brief Selects uniform emission over a local xy disk.
   *
   * \param[in] diameter_pm Positive disk diameter in pm.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetCircleEmissionPicoMeter(std::uint64_t diameter_pm) -> GGEMSSource &;

  /*!
   * \brief Selects uniform emission throughout a centered local box.
   *
   * \param[in] width_pm Positive full x width in pm.
   * \param[in] height_pm Positive full y height in pm.
   * \param[in] depth_pm Positive full z depth in pm.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetBoxEmissionPicoMeter(std::uint64_t width_pm, std::uint64_t height_pm,
                               std::uint64_t depth_pm) -> GGEMSSource &;

  /*!
   * \brief Selects uniform emission throughout a centered sphere.
   *
   * \param[in] diameter_pm Positive full sphere diameter in pm.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetSphereEmissionPicoMeter(std::uint64_t diameter_pm) -> GGEMSSource &;

  /*!
   * \brief Selects uniform emission throughout a local z cylinder.
   *
   * \param[in] diameter_pm Positive circular-base diameter in pm.
   * \param[in] height_pm Positive full local z height in pm.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetCylinderEmissionPicoMeter(std::uint64_t diameter_pm,
                                    std::uint64_t height_pm) -> GGEMSSource &;

  /*!
   * \brief Directs births along the local +Z axis.
   *
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetFixedAngularDistribution() -> GGEMSSource &;

  /*!
   * \brief Selects uniform solid-angle emission over the full sphere.
   *
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetIsotropicAngularDistribution() -> GGEMSSource &;

  /*!
   * \brief Selects uniform solid angle within local polar and azimuth bounds.
   *
   * Angles are strong quantities in radians internally. Uniformity is in
   * cos(theta) and phi, not in theta.
   *
   * \param[in] theta_min Lower polar angle in [0, pi).
   * \param[in] theta_max Upper polar angle in (theta_min, pi].
   * \param[in] phi_min Finite lower azimuth angle.
   * \param[in] phi_max Finite upper azimuth with width in (0, 2*pi].
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If angles are nonfinite, domains are invalid, or
   * binary32 bounds fail record validation.
   */
  auto SetIsotropicAngularDistribution(ggems::units::Angle theta_min,
                                       ggems::units::Angle theta_max,
                                       ggems::units::Angle phi_min,
                                       ggems::units::Angle phi_max)
    -> GGEMSSource &;

  /*!
   * \brief Directs each birth toward a focus in the global frame.
   *
   * \param[in] focus_x_pm Signed global focus x coordinate in pm.
   * \param[in] focus_y_pm Signed global focus y coordinate in pm.
   * \param[in] focus_z_pm Signed global focus z coordinate in pm.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetFocusedAngularDistributionPicoMeter(std::int64_t focus_x_pm,
                                              std::int64_t focus_y_pm,
                                              std::int64_t focus_z_pm)
    -> GGEMSSource &;

  /*!
   * \brief Selects the particle kind for a count-driven source.
   *
   * \param[in] particle_type Particle kind to encode; no validity check is
   * performed here.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If this source is activity-driven.
   */
  auto SetEmittedParticleType(particles::GGEMSParticleType particle_type)
    -> GGEMSSource &;

  /*!
   * \brief Selects a monoenergetic count-driven source.
   *
   * See GGEMSEnergyDistribution for ticket quantization and spectrum admission.
   *
   * \param[in] energy_micro_eV Strictly positive energy in micro-eV.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If activity-driven, finalized, or
   * energy-distribution construction rejects the inputs.
   */
  auto SetEnergyMicroElectronVolt(std::uint64_t energy_micro_eV)
    -> GGEMSSource &;

  /*!
   * \brief Selects a weighted discrete energy distribution.
   *
   * See GGEMSEnergyDistribution for ticket quantization and spectrum admission.
   *
   * \param[in] energies At least two positive, strictly increasing energies.
   * \param[in] relative_weights Matching finite nonnegative line weights, not
   * all zero.
   * \param[in] unit Exact supported ASCII energy unit for energies.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If activity-driven, finalized, or
   * energy-distribution construction rejects the inputs.
   */
  auto SetDiscreteEnergyLines(std::span<double const> energies,
                              std::span<double const> relative_weights,
                              std::string_view unit) -> GGEMSSource &;

  /*!
   * \brief Selects a weighted regular energy histogram.
   *
   * See GGEMSEnergyDistribution for ticket quantization and spectrum admission.
   *
   * \param[in] bin_centers At least two centers forming an exact regular
   * micro-eV grid.
   * \param[in] relative_bin_weights Matching finite nonnegative integrated bin
   * weights.
   * \param[in] unit Exact supported ASCII energy unit for bin_centers.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If activity-driven, finalized, or
   * energy-distribution construction rejects the inputs.
   */
  auto SetRegularEnergySpectrum(std::span<double const> bin_centers,
                                std::span<double const> relative_bin_weights,
                                std::string_view unit) -> GGEMSSource &;

  /*!
   * \brief Loads a regular energy histogram from a two-column file.
   *
   * See GGEMSEnergyDistribution for ticket quantization and spectrum admission.
   *
   * \param[in] filename Text file of energy-center and relative-bin-weight
   * pairs.
   * \param[in] unit Exact supported ASCII unit for file energy values.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If activity-driven, finalized, or
   * energy-distribution construction rejects the inputs.
   */
  auto LoadRegularEnergySpectrum(std::filesystem::path const &filename,
                                 std::string_view unit) -> GGEMSSource &;

  /*!
   * \brief Borrows the count-driven energy distribution.
   *
   * \return Reference valid while this source and its current distribution
   * remain alive.
   * \throws GGEMSRecoverable If this source is activity-driven.
   */
  [[nodiscard]] auto GetEnergyDistribution() const
    -> GGEMSEnergyDistribution const &;

  /*!
   * \brief Moves the source origin in the global simulation frame.
   *
   * \param[in] x_pm Signed global x coordinate in pm.
   * \param[in] y_pm Signed global y coordinate in pm.
   * \param[in] z_pm Signed global z coordinate in pm.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetPositionPicoMeter(std::int64_t x_pm, std::int64_t y_pm,
                            std::int64_t z_pm) -> GGEMSSource &;

  /*!
   * \brief Orients local +Z using an automatically selected up vector.
   *
   * \param[in] dir_x Global x component of a finite nonzero direction.
   * \param[in] dir_y Global y component of that direction.
   * \param[in] dir_z Global z component of that direction.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetDirection(double dir_x, double dir_y, double dir_z) -> GGEMSSource &;

  /*!
   * \brief Orients the source using a forward vector and up reference.
   *
   * \param[in] direction Finite nonzero global forward vector.
   * \param[in] up_reference Finite nonzero global up reference, not nearly
   * parallel to direction.
   * \return This source for chained configuration.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  auto SetOrientation(std::array<double, 3U> const &direction,
                      std::array<double, 3U> const &up_reference)
    -> GGEMSSource &;

  /*!
   * \brief Copies and validates the current analytic pose and birth record.
   *
   * Activity-driven records clear the single particle-kind and monoenergy
   * fields; their emissions are described separately.
   *
   * \return Independent record copy.
   * \throws GGEMSRecoverable If the candidate analytic record has invalid
   * dimensions, frame, angular domain, or focus.
   */
  [[nodiscard]] auto BuildExecutionRecord() const -> GGEMSSourceRecord;

  /*!
   * \brief Copies the validated count-driven execution record.
   *
   * \return Independent record copy.
   * \throws GGEMSRecoverable If activity-driven or analytic-record validation
   * fails.
   */
  [[nodiscard]] auto BuildRecord() const -> GGEMSSourceRecord;

  /*! \brief Locks population-mode and energy changes for later runs. */
  auto FinalizeInitialization() noexcept -> void;

  /*! \brief Writes the current source description through the GGEMS logger. */
  auto Verbose() const -> void;

private:
  /*!
   * \brief Requires the single-particle count-driven configuration.
   *
   * \throws GGEMSRecoverable If activity-driven.
   */
  auto CheckCountDrivenConfiguration() const -> void;

  /*!
   * \brief Requires mutable count-driven energy configuration.
   *
   * \throws GGEMSRecoverable If activity-driven or finalized.
   */
  auto CheckEnergyConfigurationMutable() const -> void;

  /*!
   * \brief Requires population-mode configuration to remain mutable.
   *
   * \throws GGEMSRecoverable If finalized.
   */
  auto CheckPopulationConfigurationMutable() const -> void;

  /*!
   * \brief Moves a prepared law into the source and updates its mono field.
   *
   * \param[in] distribution Owned prepared law; table-based laws clear the
   * record monoenergy.
   */
  auto CommitEnergyDistribution(GGEMSEnergyDistribution distribution) noexcept
    -> void;

  /*! \brief Count settings or shared radionuclide activity settings. */
  GGEMSSourcePopulationConfiguration population_configuration_{
    GGEMSCountDrivenSourceConfiguration{}};

  /*! \brief Current analytic pose and birth-law record. */
  GGEMSSourceRecord record_{};

  /*! \brief Owned energy tables for count-driven emission. */
  GGEMSEnergyDistribution energy_distribution_;

  /*! \brief Locks mode and energy mutation after runtime setup. */
  bool initialization_finalized_{false};
};

} // namespace ggems::core::sources
