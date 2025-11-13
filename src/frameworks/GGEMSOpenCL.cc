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
 * \file GGEMSOpenCL.cc
 * \brief Declaration of the GGEMSOpenCL singleton class for OpenCL management
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"

using ggems::core::GGEMSExceptionBase;

namespace ggems::ocl {
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSOpenCL::GGEMSOpenCL() {
  GGEMS_INFOEX("OpenCL", 2, "Constructing GGEMSOpenCL singleton...");
  std::set_terminate(ggems::core::TerminateHandler);
  DisableKernelCache();

  try {
    InitPlatformsAndDevices();
    GGEMS_INFOEX("OpenCL", 1, "GGEMSOpenCL successfully constructed!");
  } catch (GGEMSExceptionBase &) {
    std::terminate();
  } catch (std::exception const &) {
    std::terminate();
  } catch (...) {
    std::terminate();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSOpenCL::~GGEMSOpenCL() {
  GGEMS_INFOEX("OpenCL", 2, "Releasing GGEMSOpenCL resources...");
  GGEMS_INFOEX(
      "OpenCL", 2,
      "GGEMSOpenCL singleton destroyed (memory intentionally retained).");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::DisableKernelCache() const {
#ifdef _MSC_VER
  static char env_var[] = "CUDA_CACHE_DISABLE=1";
  _putenv(env_var);
#else
  setenv("CUDA_CACHE_DISABLE", "1", 1);
#endif

  GGEMS_INFOEX("OpenCL", 2, "CUDA kernel cache disabled.");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::InitPlatformsAndDevices() {
  GGEMS_INFOEX("OpenCL", 1, "Enumerating OpenCL platforms...");

  std::vector<cl::Platform> platforms;
  GGEMS_OCL_CHECK(cl::Platform::get(&platforms),
                  "No OpenCL platforms detected on this system.");

  platforms_.clear();
  platforms_.reserve(platforms.size());
  std::size_t plat_index{0};
  for (auto const &p : platforms) {
    platforms_.emplace_back(p, plat_index++);
  }

  GGEMS_INFOEX("OpenCL", 1, "{} OpenCL platform(s) initialized.",
               platforms_.size());
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::PrintPlatforms() const {
  GGEMS_INFO("OpenCL", "Listing available OpenCL platforms...");

  for (auto const &p : platforms_) {
    p.Print();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::PrintDevices() const {
  GGEMS_INFO("OpenCL", "Listing available OpenCL devices...");

  for (auto const &p : platforms_) {
    auto const &devices = p.GetDevices();
    for (auto const &d : devices)
      d->Print();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::Clean() noexcept {
  GGEMS_INFOEX("OpenCL", 2, "Cleaning all OpenCL platform resources...");

  for (auto &p : platforms_) {
    p.Clean();
  }

  GGEMS_INFOEX("OpenCL", 2, "All OpenCL platforms cleaned successfully.");
}
} // namespace ggems::ocl
