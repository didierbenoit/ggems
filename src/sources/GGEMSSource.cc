/*!
  \file GGEMSSource.cc

  \brief GGEMS mother class for the source

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Tuesday October 15, 2019
*/

#include <sstream>
#include <algorithm>

#include "GGEMS/global/GGEMSConstants.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/maths/GGEMSGeometryTransformation.hh"
#include "GGEMS/tools/GGEMSTools.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSSource::GGEMSSource(void)
: particle_type_(99),
  p_kernel_get_primaries_(nullptr),
  p_particle_(nullptr),
  p_pseudo_random_generator_(nullptr),
  opencl_manager_(GGEMSOpenCLManager::GetInstance())
{
  GGcout("GGEMSSource", "GGEMSSource", 3)
    << "Allocation of GGEMSSource..." << GGendl;

  // Checking if a context is activated
  if (!opencl_manager_.IsReady()) {
    GGEMSMisc::ThrowException("GGEMSSource", "GGEMSSource",
      "OpenCL Manager is not ready, you have to choose a context!!!");
  }

  // Allocation of geometry transformation
  p_geometry_transformation_ = new GGEMSGeometryTransformation;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSSource::~GGEMSSource(void)
{
  if (p_geometry_transformation_) {
    delete p_geometry_transformation_;
    p_geometry_transformation_ = nullptr;
  }

  GGcout("GGEMSSource", "~GGEMSSource", 3)
    << "Deallocation of GGEMSSource..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSource::SetPosition(GGfloat const& pos_x, GGfloat const& pos_y,
  GGfloat const& pos_z)
{
  p_geometry_transformation_->SetTranslation(
    MakeFloat3(pos_x, pos_y, pos_z));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSource::SetParticle(GGEMSParticles* const p_particle)
{
  p_particle_ = p_particle;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSource::SetRandomGenerator(
  GGEMSPseudoRandomGenerator* const p_random_generator)
{
  p_pseudo_random_generator_ = p_random_generator;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSource::SetSourceParticleType(char const* particle_type)
{
  // Convert the particle type in string
  std::string particle_type_str(particle_type);

  // Transform the string to lower character
  std::transform(particle_type_str.begin(), particle_type_str.end(),
    particle_type_str.begin(), ::tolower);

  if (!particle_type_str.compare("photon")) {
    particle_type_ = GGEMSParticleName::PHOTON;
  }
  else if (!particle_type_str.compare("electron")) {
    particle_type_ = GGEMSParticleName::ELECTRON;
  }
  else
  {
    GGEMSMisc::ThrowException("GGEMSSourceManager", "SetParticleType",
      "Unknown particle!!!");
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSource::SetLocalAxis(
  GGfloat const& m00, GGfloat const& m01, GGfloat const& m02,
  GGfloat const& m10, GGfloat const& m11, GGfloat const& m12,
  GGfloat const& m20, GGfloat const& m21, GGfloat const& m22)
{
  p_geometry_transformation_->SetAxisTransformation(
    m00, m01, m02,
    m10, m11, m12,
    m20, m21, m22
  );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSource::SetRotation(GGfloat const& rx, GGfloat const& ry,
  GGfloat const& rz)
{
  p_geometry_transformation_->SetRotation(MakeFloat3(rx, ry, rz));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSource::UpdateRotation(GGfloat const& rx, GGfloat const& ry,
  GGfloat const& rz)
{
  p_geometry_transformation_->SetRotation(MakeFloat3(rx, ry, rz));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSource::CheckParameters(void) const
{
  GGcout("GGEMSSource", "CheckParameters", 3)
    << "Checking the mandatory parameters..." << GGendl;

  // Checking the type of particles
  if (particle_type_ == 99) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "You have to set a particle type for the source:" << std::endl;
    oss << "    - Photon" << std::endl;
    oss << "    - Electron" << std::endl;
    GGEMSMisc::ThrowException("GGEMSSource", "CheckParameters", oss.str());
  }

  // Checking the particle pointer
  if (!p_particle_) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "The particle pointer is empty in source manager!!!" << std::endl;
    GGEMSMisc::ThrowException("GGEMSSource", "CheckParameters", oss.str());
  }

  // Checking the random generator pointer
  if (!p_pseudo_random_generator_) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "The random generator pointer is empty in source manager!!!"
      << std::endl;
    GGEMSMisc::ThrowException("GGEMSSource", "CheckParameters", oss.str());
  }

  // Checking the position of particles
  cl_float3 const kPosition = p_geometry_transformation_->GetPosition();
  if (GGEMSMisc::IsEqual(kPosition.s[0], std::numeric_limits<float>::min()) ||
      GGEMSMisc::IsEqual(kPosition.s[1], std::numeric_limits<float>::min()) ||
      GGEMSMisc::IsEqual(kPosition.s[2], std::numeric_limits<float>::min())) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "You have to set a position for the source!!!";
    GGEMSMisc::ThrowException("GGEMSSource", "CheckParameters", oss.str());
  }

  // Checking the rotation of particles
  cl_float3 const kRotation = p_geometry_transformation_->GetRotation();
  if (GGEMSMisc::IsEqual(kRotation.s[0], std::numeric_limits<float>::min()) ||
      GGEMSMisc::IsEqual(kRotation.s[1], std::numeric_limits<float>::min()) ||
      GGEMSMisc::IsEqual(kRotation.s[2], std::numeric_limits<float>::min())) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "You have to set a rotation for the source!!!";
    GGEMSMisc::ThrowException("GGEMSSource", "CheckParameters", oss.str());
  }
}