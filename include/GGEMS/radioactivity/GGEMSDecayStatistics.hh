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
 * \brief Integrates source activity and samples parent-decay populations.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMS/GGEMSTimeWindow.hh"
#include "GGEMS/units/GGEMSActivityUnits.hh"

namespace ggems::core::random {
class GGEMSHostRandomStream;
}

namespace ggems::core::radioactivity {

/*!
 * \brief Integrates the exponentially decaying activity over a source window.
 *
 * For \f$\lambda=\ln(2)/T_{1/2}\f$, the expectation is
 * \f$\mu=A_r e^{-\lambda(t_0-t_r)}
 * (1-e^{-\lambda\Delta t})/\lambda\f$.
 * Here \f$A_r\f$ is activity in Bq, \f$T_{1/2}\f$ is half-life in seconds,
 * \f$t_r\f$ and \f$t_0\f$ are reference and window-start times converted to
 * seconds, and \f$\Delta t\f$ is the window duration in seconds. The
 * expectation
 * is dimensionless; it counts parent decays before emission yields are applied.
 * Uses expm1 to retain short-window precision. Callers supply a finite
 * activity,
 * valid half-life, and ordered window; this function does not validate all
 * three.
 *
 * \param[in] activity_at_reference Source activity in Bq at reference_time_ps.
 * \param[in] half_life_seconds Positive finite half-life in seconds.
 * \param[in] reference_time_ps Reference time in ps, no later than the start.
 * \param[in] time_window Ordered half-open source interval in ps.
 * \return Expected parent decays; zero for zero activity or an empty window.
 * \throws GGEMSRecoverable If activity fails the nonnegative comparison or the
 * reference time is later than the window start.
 */
[[nodiscard]] auto ComputeExpectedDecayEventCount(
  units::Activity activity_at_reference, long double half_life_seconds,
  std::uint64_t reference_time_ps, GGEMSTimeWindow time_window) -> long double;

/*!
 * \brief Samples a Poisson count with the integrated parent-decay expectation.
 * \param[in] activity_at_reference Source activity in Bq at reference_time_ps.
 * \param[in] half_life_seconds Positive finite half-life in seconds.
 * \param[in] reference_time_ps Reference time in ps, no later than the start.
 * \param[in] time_window Ordered half-open source interval in ps.
 * \param[in,out] random_stream Host stream advanced by the Poisson sampler.
 * \return Sampled number of parent decays.
 * \throws GGEMSRecoverable If expectation calculation or Poisson admission
 * rejects the inputs.
 */
[[nodiscard]] auto SampleDecayEventCount(
  units::Activity activity_at_reference, long double half_life_seconds,
  std::uint64_t reference_time_ps, GGEMSTimeWindow time_window,
  random::GGEMSHostRandomStream &random_stream) -> std::uint64_t;

} // namespace ggems::core::radioactivity
