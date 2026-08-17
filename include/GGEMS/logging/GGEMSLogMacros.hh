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
 * \brief Defines the GGEMS logging front-end macros.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <source_location>

/// \endcond
#include "GGEMS/logging/GGEMSLogger.hh"

/*!
 * \brief Logs a debug message through the process-wide GGEMS logger.
 *
 * MODULE identifies the emitting subsystem, FMT is a runtime format string,
 * and optional variadic arguments provide format values. Debug messages use
 * verbosity depth -1.
 */
#define GGEMS_DEBUG(MODULE, FMT, ...)                                          \
  ggems::core::GGEMSLogger::GetInstance()                                      \
      .LogFmt<ggems::core::LogLevel::Debug>(-1, (MODULE), (FMT),               \
                                            std::source_location::current()    \
                                                __VA_OPT__(, __VA_ARGS__))

/*!
 * \brief Logs a base informational message through the GGEMS logger.
 *
 * MODULE identifies the emitting subsystem, FMT is a runtime format string,
 * and optional variadic arguments provide format values. Base informational
 * messages use verbosity depth 0.
 */
#define GGEMS_INFO(MODULE, FMT, ...)                                           \
  ggems::core::GGEMSLogger::GetInstance().LogFmt<ggems::core::LogLevel::Info>( \
      0, (MODULE), (FMT),                                                      \
      std::source_location::current() __VA_OPT__(, __VA_ARGS__))

/*!
 * \brief Logs a warning message through the GGEMS logger.
 *
 * MODULE identifies the emitting subsystem, FMT is a runtime format string,
 * and optional variadic arguments provide format values. Warning messages use
 * verbosity depth -1.
 */
#define GGEMS_WARN(MODULE, FMT, ...)                                           \
  ggems::core::GGEMSLogger::GetInstance().LogFmt<ggems::core::LogLevel::Warn>( \
      -1, (MODULE), (FMT),                                                     \
      std::source_location::current() __VA_OPT__(, __VA_ARGS__))

/*!
 * \brief Logs an error message through the GGEMS logger.
 *
 * MODULE identifies the emitting subsystem, FMT is a runtime format string,
 * and optional variadic arguments provide format values. Error messages use
 * verbosity depth -1.
 */
#define GGEMS_ERROR(MODULE, FMT, ...)                                          \
  ggems::core::GGEMSLogger::GetInstance()                                      \
      .LogFmt<ggems::core::LogLevel::Error>(-1, (MODULE), (FMT),               \
                                            std::source_location::current()    \
                                                __VA_OPT__(, __VA_ARGS__))

/*!
 * \brief Logs an informational message at an explicit verbosity depth.
 *
 * MODULE identifies the emitting subsystem, DEPTH controls detail filtering,
 * FMT is a runtime format string, and optional variadic arguments provide
 * format values.
 */
#define GGEMS_INFOEX(MODULE, DEPTH, FMT, ...)                                  \
  ggems::core::GGEMSLogger::GetInstance().LogFmt<ggems::core::LogLevel::Info>( \
      (DEPTH), (MODULE), (FMT),                                                \
      std::source_location::current() __VA_OPT__(, __VA_ARGS__))
