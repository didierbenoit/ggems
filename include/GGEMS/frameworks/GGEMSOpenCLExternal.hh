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
 * \file GGEMSOpenCLExternal.hh
 * \brief External inclusion of the OpenCL C++ bindings under controlled
 * warnings.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright GNU GPL v3
 *
 * This header centralises the inclusion of the Khronos OpenCL 3.0 C++
 * bindings while locally suppressing compiler warnings emitted from within
 * the external header.
 *
 * Encapsulating the OpenCL interface here prevents global exposure of vendor
 * constraints and keeps the build pipeline clean and uniform across Windows
 * and Linux toolchains.
 */

/*!
 * \def CL_HPP_TARGET_OPENCL_VERSION
 * \brief Target OpenCL version for the C++ bindings (Khronos specification).
 */
#define CL_HPP_TARGET_OPENCL_VERSION 300

/*!
 * \def CL_TARGET_OPENCL_VERSION
 * \brief Target OpenCL version for the underlying C API features.
 */
#define CL_TARGET_OPENCL_VERSION 300

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(_MSC_VER)
#pragma warning(push)
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

/// \cond
#include <CL/opencl.hpp>
/// \endcond

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
