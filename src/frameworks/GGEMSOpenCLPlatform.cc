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
 * \file GGEMSOpenCLPlatform.cc
 * \brief Declaration of the GGEMSOpenCLPlatform class for OpenCL 3.0 platform
 * abstraction.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-14
 * \copyright GNU General Public License v3.0
 * \version 3.0
 *
 * This header defines the \c GGEMSOpenCLPlatform class, an OpenCL 3.0 platform
 * façade that:
 * - stores the native \c cl::Platform and its stable index,
 * - discovers and owns all CPU/GPU devices on the platform,
 * - exposes strongly-typed getters for platform information,
 * - formats a comprehensive textual report via the GGEMS logger.
 *
 * The design is RAII-driven and thread-safe at the logging boundary. Device
 * ownership is unique and non-transferable (vector of \c std::unique_ptr).
 * The API emphasises const-correctness and minimal exposure of internals.
 */

#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"

namespace ggems::ocl {

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSOpenCLPlatform::GGEMSOpenCLPlatform(cl::Platform const &platform,
                                         std::size_t platform_index)
    : platform_{platform}, platform_index_{platform_index} {
  GGEMS_INFOEX("OpenCL", 2, "Allocating GGEMSOpenCLPlatform [{}]...",
               platform_index_);

  extensions_ = ExtractExtensions<CL_PLATFORM_EXTENSIONS>(platform_);

  DiscoverDevices();

  GGEMS_INFOEX("OpenCL", 2, "GGEMSOpenCLPlatform allocated with {} device(s)",
               devices_.size());
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSOpenCLPlatform::~GGEMSOpenCLPlatform() {
  GGEMS_INFOEX("OpenCL", 2, "Releasing GGEMSOpenCLPlatform [{}]",
               platform_index_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLPlatform::GetName() const {
  return GetInfo<CL_PLATFORM_NAME>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLPlatform::GetProfile() const {
  return GetInfo<CL_PLATFORM_PROFILE>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLPlatform::GetVersion() const {
  return GetInfo<CL_PLATFORM_VERSION>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLPlatform::GetVendor() const {
  return GetInfo<CL_PLATFORM_VENDOR>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLPlatform::GetExtensions() const {
  return GetInfo<CL_PLATFORM_EXTENSIONS>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_version GGEMSOpenCLPlatform::GetNumericVersion() const {
  return GetInfo<CL_PLATFORM_NUMERIC_VERSION>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_ulong GGEMSOpenCLPlatform::GetHostTimerResolution() const {
  return GetInfo<CL_PLATFORM_HOST_TIMER_RESOLUTION>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::vector<cl_name_version>
GGEMSOpenCLPlatform::GetExtensionsWithVersion() const {
  return GetInfo<CL_PLATFORM_EXTENSIONS_WITH_VERSION>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLPlatform::PrintIdentity() const {
  PrintInfo<CL_PLATFORM_PROFILE>(platform_);
  PrintInfo<CL_PLATFORM_VERSION>(platform_);
  PrintInfo<CL_PLATFORM_NUMERIC_VERSION>(platform_);
  PrintInfo<CL_PLATFORM_HOST_TIMER_RESOLUTION>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLPlatform::PrintExtension() const {
  PrintInfo<CL_PLATFORM_EXTENSIONS_WITH_VERSION>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLPlatform::Print() const {
  GGEMS_INFO("OpenCL", "++++++++++++++++++++++++++");

  GGEMS_INFO("OpenCL", "Platform [{}]: {} ({})", platform_index_, GetName(),
             GetVendor());
  GGEMS_INFO("OpenCL", "Discovered devices: {}", devices_.size());
  GGEMS_INFO("OpenCL", "++++++++++++++++++++++++++");

  PrintIdentity();
  PrintExtension();
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLPlatform::Clean() {
  GGEMS_INFOEX("OpenCL", 2, "Cleaning Platform {} resources...", GetName());

  platform_.unloadCompiler();
  devices_.clear();
  extensions_.clear();

  GGEMS_INFOEX("OpenCL", 2, "Platform resources cleaned.");
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLPlatform::DiscoverDevices() {
  GGEMS_INFOEX("OpenCL", 2, "Discovering OpenCL devices for platform [{}]...",
               platform_index_);

  constexpr cl_device_type mask = CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU;

  std::vector<cl::Device> natives;
  GGEMS_OCL_CHECK(platform_.getDevices(mask, &natives),
                  "No OpenCL devices detected on this platform.");

  devices_.clear();
  devices_.reserve(natives.size());

  for (std::size_t i = 0; i < natives.size(); ++i) {
    devices_.emplace_back(natives[i], platform_index_, i);
  }

  GGEMS_INFOEX("OpenCL", 2, "Found {} device(s) on platform [{}]",
               natives.size(), platform_index_);
}
} // namespace ggems::ocl
