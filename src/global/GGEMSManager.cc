/*!
  \file GGEMSManager.cc

  \brief GGEMS class managing the GGEMS simulation

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Monday September 30, 2019
*/

#include <algorithm>
#include <fcntl.h>

#ifdef _WIN32
#ifdef _MSC_VER
#define NOMINMAX
#endif
#include <windows.h>
#include <wincrypt.h>
#else
#include <unistd.h>
#endif

#include "GGEMS/sources/GGEMSSourceManager.hh"

#include "GGEMS/tools/GGEMSSystemOfUnits.hh"
#include "GGEMS/tools/GGEMSPrint.hh"
#include "GGEMS/tools/GGEMSChrono.hh"
#include "GGEMS/tools/GGEMSMemoryAllocation.hh"
#include "GGEMS/tools/GGEMSTools.hh"

#include "GGEMS/global/GGEMSManager.hh"
#include "GGEMS/global/GGEMSConstants.hh"

#include "GGEMS/processes/GGEMSParticles.hh"
#include "GGEMS/processes/GGEMSPrimaryParticlesStack.hh"

#include "GGEMS/randoms/GGEMSPseudoRandomGenerator.hh"
#include "GGEMS/randoms/GGEMSRandomStack.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSManager::GGEMSManager(void)
: seed_(0),
  version_("1.0"),
  number_of_particles_(0),
  v_number_of_particles_in_batch_(0),
  v_physics_list_(0),
  v_secondaries_list_(0),
  photon_distance_cut_(GGEMSUnits::um),
  electron_distance_cut_(GGEMSUnits::um),
  geometry_tolerance_(GGEMSTolerance::GEOMETRY),
  photon_level_secondaries_(0),
  electron_level_secondaries_(0),
  cross_section_table_number_of_bins_(
    GGEMSLimit::CROSS_SECTION_TABLE_NUMBER_BINS),
  cross_section_table_energy_min_(GGEMSLimit::CROSS_SECTION_TABLE_ENERGY_MIN),
  cross_section_table_energy_max_(GGEMSLimit::CROSS_SECTION_TABLE_ENERGY_MAX),
  p_particle_(nullptr),
  p_pseudo_random_generator_(nullptr),
  source_manager_(GGEMSSourceManager::GetInstance()),
  opencl_manager_(GGEMSOpenCLManager::GetInstance())
{
  GGcout("GGEMSManager", "GGEMSManager", 3)
    << "Allocation of GGEMS Manager singleton..." << GGendl;

  // Allocation of the memory for the physics list
  v_physics_list_.resize(GGEMSProcessName::NUMBER_PROCESSES, false);

  // Allocation of the memory for the secondaries list
  v_secondaries_list_.resize(GGEMSProcessName::NUMBER_PARTICLES, false);

  // Allocation of particle
  p_particle_ = new GGEMSParticles();

  // Allocation of pseudo random generator
  p_pseudo_random_generator_ = new GGEMSPseudoRandomGenerator();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSManager::~GGEMSManager(void)
{
  // Freeing memory
  if (p_particle_) {
    delete p_particle_;
    p_particle_ = nullptr;
  }

  if (p_pseudo_random_generator_) {
    delete p_pseudo_random_generator_;
    p_pseudo_random_generator_ = nullptr;
  }

  GGcout("GGEMSManager", "~GGEMSManager", 3)
    << "Deallocation of GGEMS Manager singleton..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetSeed(uint32_t const& seed)
{
  seed_ = seed;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGuint GGEMSManager::GenerateSeed() const
{
  #ifdef _WIN32
  HCRYPTPROV seedWin32;
  if (CryptAcquireContext(&seedWin32, NULL, NULL, PROV_RSA_FULL,
    CRYPT_VERIFYCONTEXT ) == FALSE) {
    std::ostringstream oss(std::ostringstream::out);
    char buffer_error[256];
    oss << "Error finding a seed: " <<
      strerror_s(buffer_error, 256, errno) << std::endl;
    GGEMSMisc::ThrowException("GGEMSManager", "GenerateSeed",
      oss.str());
  }
  return static_cast<uint32_t>(seedWin32);
  #else
  // Open a system random file
  GGint file_descriptor = ::open("/dev/urandom", O_RDONLY | O_NONBLOCK);
  if (file_descriptor < 0) {
    std::ostringstream oss( std::ostringstream::out );
    oss << "Error opening the file '/dev/urandom': " << strerror(errno)
      << std::endl;
    GGEMSMisc::ThrowException("GGEMSManager", "GenerateSeed",
      oss.str());
  }

  // Buffer storing 8 characters
  char seedArray[sizeof(GGuint)];
  ::read(file_descriptor, reinterpret_cast<GGuint*>(seedArray),
     sizeof(GGuint));
  ::close(file_descriptor);
  GGuint *seedUInt = reinterpret_cast<GGuint*>(seedArray);
  return *seedUInt;
  #endif
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetNumberOfParticles(GGulong const& number_of_particles)
{
  number_of_particles_ = number_of_particles;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::CheckMemoryForParticles(void) const
{
  // By security the particle allocation by batch should not exceed 10% of
  // RAM memory

  // Compute the RAM memory percentage allocated for primary particles
  GGdouble const kRAMParticles =
    static_cast<GGdouble>(sizeof(GGEMSPrimaryParticles))
    + static_cast<GGdouble>(sizeof(GGEMSRandom));

  // Getting the RAM memory on activated device
  GGdouble const kMaxRAMDevice = static_cast<GGdouble>(
    opencl_manager_.GetMaxRAMMemoryOnActivatedDevice());

  // Computing the ratio of used RAM memory on device
  GGdouble const kMaxRatioUsedRAM = kRAMParticles / kMaxRAMDevice;

  // Computing a theoric max. number of particles depending on activated
  // device and advice this number to the user. 10% of RAM memory for particles
  GGulong const kTheoricMaxNumberOfParticles = static_cast<GGulong>(
    0.1 * kMaxRAMDevice / (kRAMParticles/MAXIMUM_PARTICLES));

  if (kMaxRatioUsedRAM > 0.1) { // Printing warning
    GGwarn("GGEMSManager", "CheckMemoryForParticles", 0)
      << "Warning!!! The number of particles in a batch defined during GGEMS "
      << "compilation is maybe to high. We recommand to not use more than 10% "
      << "of RAM memory for particles allocation. Your theoric number of "
      << "particles is " << kTheoricMaxNumberOfParticles << ". Recompile GGEMS "
      << "with this number of particles is recommended." << GGendl;
  }
  else { // Printing theoric number of particle
    GGcout("GGEMSManager", "CheckMemoryForParticles", 0)
      << "The number of particles in a batch defined during the compilation is "
      << "correct. We recommend to not used more than 10% of memory for "
      << "particle allocation. Your theoric number of particles is "
      << kTheoricMaxNumberOfParticles << ". Recompile GGEMS with this number "
      << " of particles is recommended" << GGendl;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::OrganizeParticlesInBatch(void)
{
  GGcout("GGEMSManager", "OrganizeParticlesInBatch", 3)
    << "Organizing the number of particles in batch..." << GGendl;

  // Computing the number of batch depending on the number of simulated
  // particles and the maximum simulated particles defined during GGEMS
  // compilation
  std::size_t const kNumberOfBatchs =
    number_of_particles_ / MAXIMUM_PARTICLES + 1;

  // Resizing vector storing the number of particles in batch
  v_number_of_particles_in_batch_.resize(kNumberOfBatchs, 0);

  // Computing the number of simulated particles in batch
  if (kNumberOfBatchs == 1) {
    v_number_of_particles_in_batch_[0] = number_of_particles_;
  }
  else {
    for (auto&& i : v_number_of_particles_in_batch_) {
      i = number_of_particles_ / kNumberOfBatchs;
    }

    // Adding the remaing particles
    for (std::size_t i = 0; i < number_of_particles_ % kNumberOfBatchs; ++i) {
      v_number_of_particles_in_batch_[i]++;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetProcess(char const* process_name)
{
  // Convert the process name in string
  std::string process_name_str(process_name);

  // Transform the string to lower character
  std::transform(process_name_str.begin(), process_name_str.end(),
    process_name_str.begin(), ::tolower);

  // Activate process
  if (!process_name_str.compare("compton")) {
    v_physics_list_.at(GGEMSProcessName::PHOTON_COMPTON) = true;
  }
  else if (!process_name_str.compare("photoelectric")) {
    v_physics_list_.at(GGEMSProcessName::PHOTON_PHOTOELECTRIC) = true;
  }
  else if (!process_name_str.compare("rayleigh")) {
    v_physics_list_.at(GGEMSProcessName::PHOTON_RAYLEIGH) = true;
  }
  else if (!process_name_str.compare("eionisation")) {
    v_physics_list_.at(GGEMSProcessName::ELECTRON_IONISATION) = true;
  }
  else if (!process_name_str.compare("ebremsstrahlung")) {
    v_physics_list_.at(GGEMSProcessName::ELECTRON_BREMSSTRAHLUNG) = true;
  }
  else if (!process_name_str.compare("emultiplescattering")) {
    v_physics_list_.at(GGEMSProcessName::ELECTRON_MSC) = true;
  }
  else {
    GGEMSMisc::ThrowException("GGEMSManager", "SetProcess",
      "Unknown process!!!");
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetParticleCut(char const* particle_name,
  GGdouble const& distance)
{
  // Convert the particle name in string
  std::string particle_name_str(particle_name);

  // Transform the string to lower character
  std::transform(particle_name_str.begin(), particle_name_str.end(),
    particle_name_str.begin(), ::tolower);

  if (!particle_name_str.compare("photon")) {
    photon_distance_cut_ = distance;
  }
  else if (!particle_name_str.compare("electron")) {
    electron_distance_cut_ = distance;
  }
  else {
    GGEMSMisc::ThrowException("GGEMSManager", "SetParticleCut",
      "Unknown particle!!!");
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetParticleSecondaryAndLevel(char const* particle_name,
  GGuint const& level)
{
  // Convert the particle name in string
  std::string particle_name_str(particle_name);

  // Transform the string to lower character
  std::transform(particle_name_str.begin(), particle_name_str.end(),
    particle_name_str.begin(), ::tolower);

  if (!particle_name_str.compare("photon")) {
    GGcout("GGEMSManager", "SetParticleSecondaryAndLevel",0)
      << "Warning!!! Photon as secondary is not available yet!!!" << GGendl;
    v_secondaries_list_.at(GGEMSParticleName::PHOTON) = false;
    photon_level_secondaries_ = 0;
  }
  else if (!particle_name_str.compare("electron")) {
    v_secondaries_list_.at(GGEMSParticleName::ELECTRON) = true;
    electron_level_secondaries_ = level;
  }
  else {
    GGEMSMisc::ThrowException("GGEMSManager", "SetParticleSecondaryAndLevel",
      "Unknown particle!!!");
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetGeometryTolerance(GGdouble const& distance)
{
  // Geometry tolerance distance in the range [1mm;1nm]
  geometry_tolerance_ = fmax(GGEMSUnits::nm, fmin(GGEMSUnits::mm, distance));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetCrossSectionTableNumberOfBins(
  GGuint const& number_of_bins)
{
  cross_section_table_number_of_bins_ = number_of_bins;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetCrossSectionTableEnergyMin(GGdouble const& min_energy)
{
  cross_section_table_energy_min_ = min_energy;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::SetCrossSectionTableEnergyMax(GGdouble const& max_energy)
{
  cross_section_table_energy_max_ = max_energy;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::CheckParameters()
{
  GGcout("GGEMSManager", "CheckParameters", 1)
    << "Checking the mandatory parameters..." << GGendl;

  // Checking the seed of the random generator
  if (seed_ == 0) seed_ = GenerateSeed();

  // Checking the number of particles
  if (number_of_particles_ == 0) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "You have to set a number of particles > 0!!!";
    GGEMSMisc::ThrowException("GGEMSManager", "CheckParameters", oss.str());
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::Initialize()
{
  GGcout("GGEMSManager", "Initialize", 1)
    << "Initialization of GGEMS Manager singleton..." << GGendl;

  // Printing the banner with the GGEMS version
  PrintBanner();

  // Checking the mandatory parameters
  CheckParameters();
  GGcout("GGEMSManager", "Initialize", 0) << "Parameters OK" << GGendl;

  // Initialize the pseudo random number generator
  srand(seed_);
  GGcout("GGEMSManager", "Initialize", 0)
    << "C++ Pseudo-random number generator seeded OK" << GGendl;

  // Checking the RAM memory for particle and propose a new MAXIMUM_PARTICLE
  // number
  CheckMemoryForParticles();

  // Organize the particles in batch
  OrganizeParticlesInBatch();
  GGcout("GGEMSManager", "Initialize", 0)
    << "Particles arranged in batch OK" << GGendl;

  // Initialization of the particles
  p_particle_->Initialize();
  GGcout("GGEMSManager", "Initialize", 0)
    << "Initialization of particles OK" << GGendl;

  // Initialization of GGEMS pseudo random generator
  p_pseudo_random_generator_->Initialize();
  GGcout("GGEMSManager", "Initialize", 0)
    << "Initialization of GGEMS pseudo random generator OK" << GGendl;

  // Give particle and random to source
  source_manager_.SetParticle(p_particle_);
  source_manager_.SetRandomGenerator(p_pseudo_random_generator_);

  // Initialization of the source
  source_manager_.Initialize();
  // Checking if the source is defined by the user
  if (!source_manager_.IsReady()) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "Problem during the source initialization!!!";
    GGEMSMisc::ThrowException("GGEMSManager", "Initialize", oss.str());
  }

  // Printing informations about the simulation
  PrintInfos();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::Run()
{
  GGcout("GGEMSManager", "Run", 0)
    << "GGEMS simulation is running..." << GGendl;

  // Get the start time
  ChronoTime start_time = GGEMSChrono::Now();

  // Loop over the number of batch
  for (std::size_t i = 0; i < v_number_of_particles_in_batch_.size(); ++i) {
    GGcout("GGEMSManager", "Run", 0) << "----> Launching batch " << i+1
      << "/" << v_number_of_particles_in_batch_.size() << GGendl;

    // Generating particles
    GGcout("GGEMSManager", "Run", 0) << "      + Generating "
      << v_number_of_particles_in_batch_[i] << " particles..." << GGendl;
    source_manager_.GetPrimaries(v_number_of_particles_in_batch_[i]);
  }

  // Get the end time
  ChronoTime end_time = GGEMSChrono::Now();

  GGcout("GGEMSManager", "Run", 0)
    << "GGEMS is finished!" << GGendl;

  // Display the elapsed time in GGEMS
  GGEMSChrono::DisplayTime(end_time - start_time, "GGEMS simulation");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::PrintInfos(void) const
{
  GGcout("GGEMSManager", "PrintInfos", 0) << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "++++++++++++++++" << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "*Seed: " << seed_ << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "*Number of particles: "
    << number_of_particles_ << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "*Number of batchs: "
    << v_number_of_particles_in_batch_.size() << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "*Physics list:" << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "  --> Photon: "
    << (v_physics_list_.at(0) ? "Compton " : "")
    << (v_physics_list_.at(1) ? "Photoelectric " : "")
    << (v_physics_list_.at(2) ? "Rayleigh" : "")
    << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "  --> Electron: "
    << (v_physics_list_.at(4) ? "eIonisation " : "")
    << (v_physics_list_.at(5) ? "eMultipleScattering " : "")
    << (v_physics_list_.at(6) ? "eBremsstrahlung" : "")
    << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "  --> Tables Min: "
    << cross_section_table_energy_min_/GGEMSUnits::MeV << " MeV, Max: "
    << cross_section_table_energy_max_/GGEMSUnits::MeV << " MeV, energy bins: "
    << cross_section_table_number_of_bins_ << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "  --> Range cuts Photon: "
    << photon_distance_cut_/GGEMSUnits::mm << " mm, Electron: "
    << electron_distance_cut_/GGEMSUnits::mm << " mm" << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "*Secondary particles:"
    << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "  --> Photon level: "
    << photon_level_secondaries_  << " NOT ACTIVATED!!!" << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "  --> Electron level: "
    << electron_level_secondaries_ << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "*Geometry tolerance:"
    << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "  --> Range: "
    << geometry_tolerance_/GGEMSUnits::mm << " mm" << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << "++++++++++++++++" << GGendl;
  GGcout("GGEMSManager", "PrintInfos", 0) << GGendl;
  ;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSManager::PrintBanner(void) const
{
  GGcout("GGEMSManager", "PrintBanner", 0) << "      ____                  "
    << GGendl;
  GGcout("GGEMSManager", "PrintBanner", 0) << ".--. /\\__/\\ .--.            "
    << GGendl;
  GGcout("GGEMSManager", "PrintBanner", 0) << "`O  / /  \\ \\  .`     GGEMS "
    << version_ << GGendl;
  GGcout("GGEMSManager", "PrintBanner", 0) << "  `-| |  | |O`              "
    << GGendl;
  GGcout("GGEMSManager", "PrintBanner", 0) << "   -|`|..|`|-        "
    << GGendl;
  GGcout("GGEMSManager", "PrintBanner", 0) << " .` \\.\\__/./ `.    "
    << GGendl;
  GGcout("GGEMSManager", "PrintBanner", 0) << "'.-` \\/__\\/ `-.'   "
    << GGendl;
  GGcout("GGEMSManager", "PrintBanner", 0) << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSManager* get_instance_ggems_manager(void)
{
  return &GGEMSManager::GetInstance();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_seed_ggems_manager(GGEMSManager* p_ggems_manager, GGuint const seed)
{
  p_ggems_manager->SetSeed(seed);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void initialize_ggems_manager(GGEMSManager* p_ggems_manager)
{
  p_ggems_manager->Initialize();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_number_of_particles_ggems_manager(GGEMSManager* p_ggems_manager,
  GGulong const number_of_particles)
{
  p_ggems_manager->SetNumberOfParticles(number_of_particles);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_process_ggems_manager(GGEMSManager* p_ggems_manager,
  char const* process_name)
{
  p_ggems_manager->SetProcess(process_name);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_particle_cut_ggems_manager(GGEMSManager* p_ggems_manager,
  char const* particle_name, GGdouble const distance)
{
  p_ggems_manager->SetParticleCut(particle_name, distance);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_geometry_tolerance_ggems_manager(GGEMSManager* p_ggems_manager,
  GGdouble const distance)
{
  p_ggems_manager->SetGeometryTolerance(distance);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_secondary_particle_and_level_ggems_manager(
  GGEMSManager* p_ggems_manager, char const* particle_name, GGuint const level)
{
  p_ggems_manager->SetParticleSecondaryAndLevel(particle_name, level);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_cross_section_table_number_of_bins_ggems_manager(
  GGEMSManager* p_ggems_manager, GGuint const number_of_bins)
{
  p_ggems_manager->SetCrossSectionTableNumberOfBins(number_of_bins);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_cross_section_table_energy_min_ggems_manager(
  GGEMSManager* p_ggems_manager, GGdouble const min_energy)
{
  p_ggems_manager->SetCrossSectionTableEnergyMin(min_energy);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_cross_section_table_energy_max_ggems_manager(
  GGEMSManager* p_ggems_manager, GGdouble const max_energy)
{
  p_ggems_manager->SetCrossSectionTableEnergyMax(max_energy);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void run_ggems_manager(GGEMSManager* p_ggems_manager)
{
  p_ggems_manager->Run();
}