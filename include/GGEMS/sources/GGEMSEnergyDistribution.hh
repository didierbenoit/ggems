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
 * \brief Builds source energy laws and quantized selection tables.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string_view>
#include <vector>

#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace ggems::core::sources {

/*!
 * \brief Owns a monoenergetic, discrete-line, or regular-bin energy law.
 *
 * Tabulated energies are canonical unsigned micro-eV. Relative weights describe
 * line probabilities or integrated bin masses, not density per energy unit.
 * They need not sum to one. A 32-bit random word selects the first cumulative
 * upper bound strictly greater than the word. Quotas in the 2^32-ticket space
 * are rounded down, then remaining tickets go to the largest remainders, with
 * lower indices winning ties. Positive weights must receive a reachable ticket;
 * zero weights retain repeated cumulative bounds. These quantized probabilities
 * approximate the mathematical relative weights.
 *
 * A regular spectrum uses equally spaced centers, an even integer micro-eV
 * width, and strictly positive lower edges. Device sampling selects a bin by
 * its integrated weight, then samples within that bin. Returned spans borrow
 * this object's arrays and are invalidated by assignment or destruction.
 */
class GGEMSEnergyDistribution {
public:
  /*! \brief Creates a 511 keV monoenergetic distribution. */
  GGEMSEnergyDistribution() = default;

  /*! \brief Releases the owned energy and ticket tables. */
  ~GGEMSEnergyDistribution() = default;

  /*! \brief Copies the energy law and its owned tables. */
  GGEMSEnergyDistribution(GGEMSEnergyDistribution const &) = default;

  /*! \brief Transfers the owned energy and ticket tables. */
  GGEMSEnergyDistribution(GGEMSEnergyDistribution &&) noexcept = default;

  /*!
   * \brief Replaces the energy law with an independent copy.
   * \return This object after assignment.
   */
  auto operator=(GGEMSEnergyDistribution const &)
    -> GGEMSEnergyDistribution & = default;

  /*!
   * \brief Replaces the energy law by transferring owned tables.
   * \return This object after assignment.
   */
  auto operator=(GGEMSEnergyDistribution &&) noexcept
    -> GGEMSEnergyDistribution & = default;

  /*!
   * \brief Builds a strictly positive monoenergetic law.
   *
   * \param[in] energy_micro_eV Energy in canonical micro-eV.
   * \return Distribution with no tabulated entries.
   * \throws GGEMSRecoverable If energy is zero.
   */
  [[nodiscard]] static auto BuildMono(std::uint64_t energy_micro_eV)
    -> GGEMSEnergyDistribution;

  /*!
   * \brief Builds a weighted discrete-line law.
   *
   * Callers supply finite weights. Line energies must remain distinct and
   * increasing after canonical conversion.
   *
   * \param[in] energies At least two positive line energies.
   * \param[in] relative_weights Matching finite nonnegative weights with
   * positive total mass.
   * \param[in] unit Exact supported ASCII energy unit.
   * \return Owned energy values, original relative weights, and cumulative
   * tickets.
   * \throws GGEMSRecoverable If sizes, energy values, weight signs, or ticket
   * allocation are invalid.
   */
  [[nodiscard]] static auto
  BuildDiscreteLines(std::span<double const> energies,
                     std::span<double const> relative_weights,
                     std::string_view unit) -> GGEMSEnergyDistribution;

  /*!
   * \brief Builds a regular energy histogram.
   *
   * Callers supply finite weights. Centers must form an exactly regular grid
   * after canonical conversion; the width must be even and the first lower edge
   * positive.
   *
   * \param[in] bin_centers At least two positive bin centers.
   * \param[in] relative_weights Matching finite nonnegative weights with
   * positive total mass.
   * \param[in] unit Exact supported ASCII energy unit.
   * \return Owned energy values, original relative weights, and cumulative
   * tickets.
   * \throws GGEMSRecoverable If sizes, energy values, bin geometry, weight
   * signs, or ticket allocation are invalid.
   */
  [[nodiscard]] static auto
  BuildRegularSpectrum(std::span<double const> bin_centers,
                       std::span<double const> relative_weights,
                       std::string_view unit) -> GGEMSEnergyDistribution;

  /*!
   * \brief Builds a weighted discrete-line law.
   *
   * Callers supply finite weights. Line energies must remain distinct and
   * increasing after canonical conversion.
   *
   * \param[in] energies_micro_eV At least two strictly increasing positive
   * canonical micro-eV line energies.
   * \param[in] relative_weights Matching finite nonnegative weights with
   * positive total mass.
   * \return Owned energy values, original relative weights, and cumulative
   * tickets.
   * \throws GGEMSRecoverable If sizes, energy values, weight signs, or ticket
   * allocation are invalid.
   */
  [[nodiscard]] static auto
  BuildDiscreteLines(std::span<std::uint64_t const> energies_micro_eV,
                     std::span<double const> relative_weights)
    -> GGEMSEnergyDistribution;

  /*!
   * \brief Builds a regular energy histogram.
   *
   * Callers supply finite weights. Centers must form an exactly regular grid
   * after canonical conversion; the width must be even and the first lower edge
   * positive.
   *
   * \param[in] bin_centers_micro_eV At least two strictly increasing positive
   * canonical micro-eV centers.
   * \param[in] relative_weights Matching finite nonnegative weights with
   * positive total mass.
   * \return Owned energy values, original relative weights, and cumulative
   * tickets.
   * \throws GGEMSRecoverable If sizes, energy values, bin geometry, weight
   * signs, or ticket allocation are invalid.
   */
  [[nodiscard]] static auto
  BuildRegularSpectrum(std::span<std::uint64_t const> bin_centers_micro_eV,
                       std::span<double const> relative_weights)
    -> GGEMSEnergyDistribution;

  /*!
   * \brief Reads a two-column regular energy histogram.
   *
   * Whitespace separates fields; # starts a line comment. Empty/comment-only
   * lines are ignored. Weights are integrated bin masses.
   *
   * \param[in] filename Text file containing energy-center and relative-weight
   * columns.
   * \param[in] unit Exact supported ASCII unit for energy centers.
   * \return Prepared regular-spectrum law.
   * \throws GGEMSRecoverable If file access, numeric parsing, finite-value
   * checks, or spectrum construction fails.
   */
  [[nodiscard]] static auto
  LoadRegularSpectrum(std::filesystem::path const &filename,
                      std::string_view unit) -> GGEMSEnergyDistribution;

  /*!
   * \brief Identifies the stored energy law.
   *
   * \return Mono, DiscreteLines, or RegularSpectrum.
   */
  [[nodiscard]] auto GetType() const noexcept -> GGEMSEnergyDistributionType {
    return type_;
  }

  /*!
   * \brief Reads the monoenergetic payload.
   *
   * \return Energy in micro-eV; zero for tabulated laws.
   */
  [[nodiscard]] auto GetMonoEnergyMicroElectronVolt() const noexcept
    -> std::uint64_t {
    return mono_energy_micro_eV_;
  }

  /*!
   * \brief Reads the canonical regular-bin width.
   *
   * \return Width in micro-eV; zero for non-spectrum laws.
   */
  [[nodiscard]] auto GetRegularBinWidthMicroElectronVolt() const noexcept
    -> std::uint64_t {
    return regular_bin_width_micro_eV_;
  }

  /*!
   * \brief Returns the descriptor table-entry count.
   *
   * \return Number of energies represented by the descriptor uint32 field.
   */
  [[nodiscard]] auto GetTableCount() const noexcept -> std::uint32_t {
    return static_cast<std::uint32_t>(energy_values_micro_eV_.size());
  }

  /*!
   * \brief Packs a descriptor referring to caller-managed shared tables.
   *
   * \param[in] table_offset Element offset of this law in the parallel packed
   * arrays.
   * \return Device descriptor; Mono ignores table_offset and has zero table
   * count.
   */
  [[nodiscard]] auto BuildRecord(std::uint64_t table_offset) const noexcept
    -> GGEMSEnergyDistributionRecord;

  /*!
   * \brief Borrows the ordered canonical energy table.
   *
   * \return Line energies or bin centers in micro-eV; empty for Mono.
   */
  [[nodiscard]] auto GetEnergyValuesMicroElectronVolt() const noexcept
    -> std::span<std::uint64_t const> {
    return energy_values_micro_eV_;
  }

  /*!
   * \brief Borrows the unnormalized authoring weights.
   *
   * \return Weights parallel to the energy table; empty for Mono.
   */
  [[nodiscard]] auto GetRelativeWeights() const noexcept
    -> std::span<double const> {
    return relative_weights_;
  }

  /*!
   * \brief Borrows the exclusive cumulative ticket bounds.
   *
   * \return Bounds parallel to energies, ending at 2^32 for a tabulated law.
   */
  [[nodiscard]] auto GetCumulativeTicketUpperBounds() const noexcept
    -> std::span<std::uint64_t const> {
    return cumulative_ticket_upper_;
  }

private:
  /*!
   * \brief Takes ownership of an already prepared energy law.
   * \param[in] type Distribution kind.
   * \param[in] mono_energy_micro_eV Mono energy in micro-eV, or zero for a
   * table.
   * \param[in] regular_bin_width_micro_eV Spectrum width in micro-eV, or zero.
   * \param[in] energy_values_micro_eV Ordered line energies or bin centers.
   * \param[in] relative_weights Original parallel weights.
   * \param[in] cumulative_ticket_upper Parallel exclusive ticket bounds.
   */
  GGEMSEnergyDistribution(GGEMSEnergyDistributionType type,
                          std::uint64_t mono_energy_micro_eV,
                          std::uint64_t regular_bin_width_micro_eV,
                          std::vector<std::uint64_t> energy_values_micro_eV,
                          std::vector<double> relative_weights,
                          std::vector<std::uint64_t> cumulative_ticket_upper);

  /*!
   * \brief Prepares spectrum tables from validated boundary inputs.
   *
   * Checks sizes and converts centers before regular-grid admission.
   *
   * \param[in] bin_centers Authoring centers.
   * \param[in] relative_bin_weights Parallel integrated bin masses.
   * \param[in] unit Exact ASCII energy unit.
   * \param[in] filename Optional diagnostic filename.
   * \param[in] line_numbers Optional source line for each center.
   * \return Owned prepared energy law.
   * \throws GGEMSRecoverable If grid or ticket preparation rejects the supplied
   * values.
   */
  [[nodiscard]] static auto BuildRegularSpectrumWithContext(
    std::span<double const> bin_centers,
    std::span<double const> relative_bin_weights, std::string_view unit,
    std::string_view filename, std::span<std::size_t const> line_numbers)
    -> GGEMSEnergyDistribution;

  /*!
   * \brief Prepares line tables from validated boundary inputs.
   *
   * Caller establishes energy/count consistency before ticket preparation.
   *
   * \param[in] energy_values Already checked canonical line energies, moved
   * into the law.
   * \param[in] relative_weights Matching finite nonnegative line weights.
   * \return Owned prepared energy law.
   * \throws GGEMSRecoverable If grid or ticket preparation rejects the supplied
   * values.
   */
  [[nodiscard]] static auto
  BuildDiscreteLinesFromValues(std::vector<std::uint64_t> energy_values,
                               std::span<double const> relative_weights)
    -> GGEMSEnergyDistribution;

  /*!
   * \brief Prepares spectrum tables from validated boundary inputs.
   *
   * Checks the canonical regular grid, even width, and positive lower edge.
   *
   * \param[in] energy_values At least two checked increasing canonical centers.
   * \param[in] relative_bin_weights Matching finite nonnegative integrated bin
   * masses.
   * \param[in] filename Optional diagnostic filename.
   * \param[in] line_numbers Optional source lines parallel to centers.
   * \return Owned prepared energy law.
   * \throws GGEMSRecoverable If grid or ticket preparation rejects the supplied
   * values.
   */
  [[nodiscard]] static auto BuildRegularSpectrumFromValues(
    std::vector<std::uint64_t> energy_values,
    std::span<double const> relative_bin_weights, std::string_view filename,
    std::span<std::size_t const> line_numbers) -> GGEMSEnergyDistribution;

  /*! \brief Active energy-law kind. */
  GGEMSEnergyDistributionType type_{GGEMSEnergyDistributionType::Mono};

  /*! \brief Mono energy in micro-eV; zero when a table supplies energies. */
  std::uint64_t mono_energy_micro_eV_{511'000'000'000ULL};

  /*! \brief Canonical width shared by all spectrum bins, in micro-eV. */
  std::uint64_t regular_bin_width_micro_eV_{0ULL};

  /*! \brief Owned canonical line energies or regular-bin centers. */
  std::vector<std::uint64_t> energy_values_micro_eV_;

  /*! \brief Original integrated weights parallel to the energy table. */
  std::vector<double> relative_weights_;

  /*! \brief Exclusive integer ticket bounds parallel to the energy table. */
  std::vector<std::uint64_t> cumulative_ticket_upper_;
};
} // namespace ggems::core::sources
