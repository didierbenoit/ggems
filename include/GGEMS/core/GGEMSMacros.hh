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

#include "GGEMS/core/GGEMSLogger.hh"

#define GGEMS_DEBUG(MODULE, FMT, ...)                                          \
  ggems::core::GGEMSLogger::GetInstance()                                      \
      .LogFmt<ggems::core::LogLevel::Debug>(                                   \
          (MODULE), (FMT), std::source_location::current(), __VA_ARGS__)

#define GGEMS_INFO(MODULE, FMT, ...)                                           \
  ggems::core::GGEMSLogger::GetInstance().LogFmt<ggems::core::LogLevel::Info>( \
      (MODULE), (FMT), std::source_location::current(), __VA_ARGS__)

#define GGEMS_WARN(MODULE, FMT, ...)                                           \
  ggems::core::GGEMSLogger::GetInstance().LogFmt<ggems::core::LogLevel::Warn>( \
      (MODULE), (FMT), std::source_location::current(), __VA_ARGS__)

#define GGEMS_ERROR(MODULE, FMT, ...)                                          \
  ggems::core::GGEMSLogger::GetInstance()                                      \
      .LogFmt<ggems::core::LogLevel::Error>(                                   \
          (MODULE), (FMT), std::source_location::current(), __VA_ARGS__)

#define GGEMS_INFOEX(MODULE, DEPTH, FMT, ...)                                  \
  ggems::core::GGEMSLogger::GetInstance().InfoEx(                              \
      (DEPTH), (MODULE), (FMT), std::source_location::current(), __VA_ARGS__)

#define GGEMS_CHECK(COND, MSG)                                                 \
  do {                                                                         \
    if (!(COND))                                                               \
      ggems::core::Throw<ggems::core::GGEMSFatal>(MSG);                        \
  } while (0)

#define GGEMS_OCL_CHECK(EXPR, CONTEXT)                                         \
  do {                                                                         \
    const cl_int error_code = (EXPR);                                          \
    ggems::ocl::CheckCLError(error_code, (CONTEXT), true,                      \
                             std::source_location::current());                 \
  } while (0)

#define GGEMS_OCL_CHECK_INTERNAL(EXPR, CONTEXT)                                \
  do {                                                                         \
    const cl_int error_code = (EXPR);                                          \
    ggems::ocl::CheckCLError<ggems::core::GGEMSInternal>(                      \
        error_code, (CONTEXT), false, std::source_location::current());        \
  } while (0)

#define GGEMS_OCL_CHECK_RECOVERABLE(EXPR, CONTEXT)                             \
  do {                                                                         \
    const cl_int error_code = (EXPR);                                          \
    ggems::ocl::CheckCLError<ggems::core::GGEMSRecoverable>(                   \
        error_code, (CONTEXT), false, std::source_location::current());        \
  } while (0)
