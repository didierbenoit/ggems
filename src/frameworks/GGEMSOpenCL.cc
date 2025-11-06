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
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSOpenCL::GGEMSOpenCL() {
//  gglog::info3("GGEMSOpenCL", "GGEMSOpenCL") << "Constructing GGEMSOpenCL singleton..." << gglog::endl;
  std::set_terminate(ggocl::TerminateHandler);

  DisableKernelCache();

  try {
    InitPlatformsAndDevices();
//    gglog::info2("GGEMSOpenCL", "GGEMSOpenCL") << "GGEMSOpenCL successfully constructed!" << gglog::endl;
  } catch(GGEMSException& e) {
//    gglog::err("GGEMSOpenCL", "GGEMSOpenCL") << "Critical initialization error: " << e.what() << gglog::endl;
    std::terminate();
  } catch(std::exception const& e) {
//    gglog::err("GGEMSOpenCL", "GGEMSOpenCL") << "Unexpected std::exception: " << e.what() << gglog::endl;
    std::terminate();
  } catch(...) {
//    gglog::err("GGEMSOpenCL", "GGEMSOpenCL") << "Unknown fatal error during OpenCL initialization." << gglog::endl;
    std::terminate();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSOpenCL::~GGEMSOpenCL() {
//  gglog::info3("GGEMSOpenCL", "~GGEMSOpenCL") << "Releasing GGEMSOpenCL resources..." << gglog::endl;
//  gglog::info3("GGEMSOpenCL", "~GGEMSOpenCL") << "GGEMSOpenCL singleton destroyed (memory intentionally retained)." << gglog::endl;
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

//  gglog::info3("GGEMSOpenCL", "~DisableKernelCache") << "CUDA kernel cache disabled" << gglog::endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::InitPlatformsAndDevices() {
//  gglog::info2("GGEMSOpenCL", "InitPlatformsAndDevices") << "Enumerating OpenCL platforms..." << gglog::endl;

  std::vector<cl::Platform> platforms;
//  GGOCL_CHECK(cl::Platform::get(&platforms));

  if (platforms.empty()) {
    throw GGEMSException(__FILENAME__, __PRETTY_FUNCTION__, __LINE__,
      "No OpenCL platforms detected on this system.");
  }

  platforms_.clear();
  platforms_.reserve(platforms.size());
  std::size_t plat_index{0};
  for (auto const& p : platforms) {
    platforms_.emplace_back(p, plat_index++);
  }

//  gglog::info2("GGEMSOpenCL", "InitPlatformsAndDevices") << platforms_.size() << " OpenCL platform(s) initialized." << gglog::endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::PrintPlatforms() const {
//  gglog::info("GGEMSOpenCL", "PrintPlatforms") << "Listing available OpenCL platforms..." << gglog::endl;

  for (auto const& p : platforms_) {
    p.Print();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::PrintDevices() const {
//  gglog::info("GGEMSOpenCL", "PrintDevices") << "Listing available OpenCL devices..." << gglog::endl;

  for (auto const& p : platforms_) {
    auto const& devices = p.GetDevices();
    for (auto const& d : devices)
      d->Print();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::Clean() noexcept {
//  gglog::info2("GGEMSOpenCL", "Clean") << "Cleaning all OpenCL platform resources..." << gglog::endl;

  for (auto& p : platforms_) {
    p.Clean();
  }

//  gglog::info2("GGEMSOpenCL", "Clean") << "All OpenCL platforms cleaned successfully." << gglog::endl;
}
