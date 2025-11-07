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

#define GGEMS_DEBUG(MODULE, FMT, ...) \
  ggems::core::GGEMSLogger::GetInstance().Debug((MODULE), (FMT) __VA_OPT__(,) __VA_ARGS__)

#define GGEMS_INFO(MODULE, FMT, ...) \
  ggems::core::GGEMSLogger::GetInstance().Info((MODULE), (FMT) __VA_OPT__(,) __VA_ARGS__)

#define GGEMS_WARN(MODULE, FMT, ...) \
  ggems::core::GGEMSLogger::GetInstance().Warn((MODULE), (FMT) __VA_OPT__(,) __VA_ARGS__)

#define GGEMS_ERROR(MODULE, FMT, ...) \
  ggems::core::GGEMSLogger::GetInstance().Error((MODULE), (FMT) __VA_OPT__(,) __VA_ARGS__)

#define GGEMS_INFOEX(MODULE, DEPTH, FMT, ...) \
  ggems::core::GGEMSLogger::GetInstance().InfoEx((DEPTH), (MODULE), (FMT) __VA_OPT__(,) __VA_ARGS__)


#define GGEMS_CHECK(COND, FMT, ...) \
  do { \
    if (!(COND)) { \
      throw ::ggems::core::GGEMSInternal(std::format((FMT) __VA_OPT__(,) __VA_ARGS__)); \
    } \
  } while (false)

#define GGOCL_CHECK(ERRCODE, FMT, ...) \
  do { \
    if ((ERRCODE) != CL_SUCCESS) { \
      throw ::ggems::core::GGEMSRecoverable( \
        std::format("{} (OpenCL error code: {})", std::format((FMT) __VA_OPT__(,) __VA_ARGS__), (ERRCODE)) \
      ); \
    } \
  } while (false)

