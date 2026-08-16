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
 * \brief Declares internal metadata helpers used by GGEMS logging.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <string>
#include <string_view>

/// \endcond
/*!
 * \namespace ggems::core::logging::detail
 * \brief Provides internal metadata helpers used by the GGEMS logger.
 */
namespace ggems::core::logging::detail {

/*!
 * \brief Returns a stable compact tag for the current thread identifier.
 *
 * Tags use the form ``T<number>`` and are assigned lazily within the process.
 *
 * \return Compact tag associated with the current thread identifier.
 */
[[nodiscard]] auto ThreadTag() -> std::string;

/*!
 * \brief Reduces a source-location function spelling for concise log prefixes.
 *
 * The simplifier performs best-effort normalization of parameter lists,
 * lambdas, GGEMS namespace qualification, and function template arguments.
 *
 * \param[in] function_name Compiler-provided function spelling.
 * \return View into \p function_name containing the simplified spelling.
 */
[[nodiscard]] auto SimplifyFunctionName(std::string_view function_name) noexcept
    -> std::string_view;

} // namespace ggems::core::logging::detail
