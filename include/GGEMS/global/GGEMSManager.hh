#ifndef GUARD_GGEMS_GLOBAL_GGEMSMANAGER_HH
#define GUARD_GGEMS_GLOBAL_GGEMSMANAGER_HH

/*!
  \file GGEMSManager.hh

  \brief GGEMS class managing the GGEMS simulation

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Monday September 30, 2019
*/

#ifdef _MSC_VER
#pragma warning(disable: 4251) // Deleting warning exporting STL members!!!
#endif

#include <cstdint>
#include <string>
#include <vector>

#include "GGEMS/global/GGEMSExport.hh"
#include "GGEMS/tools/GGEMSTypes.hh"

/*!
  \class GGEMSManager
  \brief GGEMS class managing the complete simulation
*/
class GGEMS_EXPORT GGEMSManager
{
  private:
    /*!
      \brief Unable the constructor for the user
    */
    GGEMSManager(void);

    /*!
      \brief Unable the destructor for the user
    */
    ~GGEMSManager(void);

  public:
    /*!
      \fn static GGEMSManager& GetInstance(void)
      \brief Create at first time the Singleton
      \return Object of type GGEMSManager
    */
    static GGEMSManager& GetInstance(void)
    {
      static GGEMSManager instance;
      return instance;
    }

    /*!
      \fn GGEMSManager(GGEMSManager const& ggems_manager) = delete
      \param ggems_manager - reference on the ggems manager
      \brief Avoid copy of the class by reference
    */
    GGEMSManager(GGEMSManager const& ggems_manager) = delete;

    /*!
      \fn GGEMSManager& operator=(GGEMSManager const& ggems_manager) = delete
      \param ggems_manager - reference on the ggems manager
      \brief Avoid assignement of the class by reference
    */
    GGEMSManager& operator=(GGEMSManager const& ggems_manager) = delete;

    /*!
      \fn GGEMSManager(GGEMSManager const&& ggems_manager) = delete
      \param ggems_manager - rvalue reference on the ggems manager
      \brief Avoid copy of the class by rvalue reference
    */
    GGEMSManager(GGEMSManager const&& ggems_manager) = delete;

    /*!
      \fn GGEMSManager& operator=(GGEMSManager const&& ggems_manager) = delete
      \param ggems_manager - rvalue reference on the ggems manager
      \brief Avoid copy of the class by rvalue reference
    */
    GGEMSManager& operator=(GGEMSManager const&& ggems_manager) = delete;

    /*!
      \fn void Initialize(void)
      \brief Initialization of the GGEMS simulation and check parameters
    */
    void Initialize(void);

    /*!
      \fn void Run(void)
      \brief run the GGEMS simulation
    */
    void Run(void);

    /*!
      \fn void SetSeed(GGuint const& seed)
      \param seed - seed for the random generator
      \brief Set the seed of random for the simulation
    */
    void SetSeed(GGuint const& seed);

    /*!
      \fn inline GGuint GetSeed(void) const
      \return the seed given by the user or generated by GGEMS
      \brief Get the general seed for the simulation
    */
    inline GGuint GetSeed(void) const {return seed_;};

    /*!
      \fn inline std::string GetVersion(void) const
      \return string of GGEMS version
      \brief Get the version of GGEMS
    */
    inline std::string GetVersion(void) const {return version_;};

    /*!
      \fn void SetOpenCLVerbose(bool const& is_opencl_verbose)
      \param is_opencl_verbose - flag for opencl verbosity
      \brief set the flag for OpenCL verbosity
    */
    void SetOpenCLVerbose(bool const& is_opencl_verbose);

    /*!
      \fn void SetMaterialDatabaseVerbose(bool const& is_material_database_verbose)
      \param is_material_database_verbose - flag for material database verbosity
      \brief set the flag for material database verbosity
    */
    void SetMaterialDatabaseVerbose(bool const& is_material_database_verbose);

    /*!
      \fn void SetSourceVerbose(bool const& is_source_verbose)
      \param is_source_verbose - flag for source verbosity
      \brief set the flag for source verbosity
    */
    void SetSourceVerbose(bool const& is_source_verbose);

    /*!
      \fn void SetPhantomVerbose(bool const is_phantom_verbose)
      \param is_phantom_verbose - flag for phantom verbosity
      \brief set the flag for phantom verbosity
    */
    void SetPhantomVerbose(bool const& is_phantom_verbose);

    /*!
      \fn void SetMemoryRAMVerbose(bool const& is_memory_ram_verbose)
      \param is_memory_ram_verbose - flag for memory RAM verbosity
      \brief set the flag for memory RAM verbosity
    */
    void SetMemoryRAMVerbose(bool const& is_memory_ram_verbose);

    /*!
      \fn void SetProcessesVerbose(bool const& is_processes_verbose)
      \param is_processes_verbose - flag for processes verbosity
      \brief set the flag for processes verbosity
    */
    void SetProcessesVerbose(bool const& is_processes_verbose);

    /*!
      \fn void SetRangeCutsVerbose(bool const& is_range_cuts_verbose)
      \param is_range_cuts_verbose - flag for range cuts verbosity
      \brief set the flag for range cuts verbosity
    */
    void SetRangeCutsVerbose(bool const& is_range_cuts_verbose);

    /*!
      \fn void SetRandomVerbose(bool const& is_random_verbose)
      \param is_random_verbose - flag for random verbosity
      \brief set the flag for random verbosity
    */
    void SetRandomVerbose(bool const& is_random_verbose);

    /*!
      \fn void SetTrackingVerbose(bool const& is_tracking_verbose, GGint const& particle_tracking_id)
      \param is_tracking_verbose - flag for tracking verbosity
      \param particle_tracking_id - particle id for tracking
      \brief set the flag for tracking verbosity and an index for particle tracking
    */
    void SetTrackingVerbose(bool const& is_tracking_verbose, GGint const& particle_tracking_id);

  private:
    /*!
      \fn void PrintBanner(void) const
      \brief Print GGEMS banner
    */
    void PrintBanner(void) const;

    /*!
      \fn void CheckParameters(void)
      \brief check the mandatory parameters for the GGEMS simulation
    */
    void CheckParameters(void);

    /*!
      \fn GGuint GenerateSeed(void) const
      \return the seed computed by GGEMS
      \brief generate a seed by GGEMS and return it
    */
    GGuint GenerateSeed(void) const;

  private: // Global simulation parameters
    GGuint seed_; /*!< Seed for the random generator */
    std::string version_; /*!< Version of GGEMS */
    bool is_opencl_verbose_; /*!< Flag for OpenCL verbosity */
    bool is_material_database_verbose_; /*!< Flag for material database verbosity */
    bool is_source_verbose_; /*!< Flag for source verbosity */
    bool is_phantom_verbose_; /*!< Flag for phantom verbosity */
    bool is_memory_ram_verbose_; /*!< Flag for memory RAM verbosity */
    bool is_processes_verbose_; /*!< Flag for processes verbosity */
    bool is_range_cuts_verbose_; /*!< Flag for range cuts verbosity */
    bool is_random_verbose_; /*!< Flag for random verbosity */
    bool is_tracking_verbose_; /*!< Flag for tracking verbosity */
    GGint particle_tracking_id_; /*!< Particle if for tracking */
};

/*!
  \fn GGEMSManager* get_instance_ggems_manager(void)
  \return the pointer on the singleton
  \brief Get the GGEMSManager pointer for python user.
*/
extern "C" GGEMS_EXPORT GGEMSManager* get_instance_ggems_manager(void);

/*!
  \fn void set_seed_ggems_manager(GGEMSManager* ggems_manager, GGuint const seed)
  \param ggems_manager - pointer on the singleton
  \param seed - seed given by the user
  \brief Set the seed for the simulation
*/
extern "C" void GGEMS_EXPORT set_seed_ggems_manager(GGEMSManager* ggems_manager, GGuint const seed);

/*!
  \fn void initialize_ggems_manager(GGEMSManager* ggems_manager)
  \param ggems_manager - pointer on the singleton
  \brief Initialize GGEMS simulation
*/
extern "C" GGEMS_EXPORT void initialize_ggems_manager(GGEMSManager* ggems_manager);

/*!
  \fn void set_opencl_verbose_ggems_manager(GGEMSManager* ggems_manager, bool const is_opencl_verbose)
  \param ggems_manager - pointer on the singleton
  \param is_opencl_verbose - flag on opencl verbose
  \brief Set the OpenCL verbosity
*/
extern "C" GGEMS_EXPORT void set_opencl_verbose_ggems_manager(GGEMSManager* ggems_manager, bool const is_opencl_verbose);

/*!
  \fn void set_material_database_verbose_ggems_manager(GGEMSManager* ggems_manager, bool const is_material_database_verbose)
  \param ggems_manager - pointer on the singleton
  \param is_material_database_verbose - flag on material database verbose
  \brief Set the material database verbosity
*/
extern "C" GGEMS_EXPORT void set_material_database_verbose_ggems_manager(GGEMSManager* ggems_manager, bool const is_material_database_verbose);

/*!
  \fn void set_source_ggems_manager(GGEMSManager* ggems_manager, bool const is_source_verbose)
  \param ggems_manager - pointer on the singleton
  \param is_source_verbose - flag on source verbose
  \brief Set the source verbosity
*/
extern "C" GGEMS_EXPORT void set_source_ggems_manager(GGEMSManager* ggems_manager, bool const is_source_verbose);

/*!
  \fn void set_phantom_ggems_manager(GGEMSManager* ggems_manager, bool const is_phantom_verbose)
  \param ggems_manager - pointer on the singleton
  \param is_phantom_verbose - flag on phantom verbose
  \brief Set the phantom verbosity
*/
extern "C" GGEMS_EXPORT void set_phantom_ggems_manager(GGEMSManager* ggems_manager, bool const is_phantom_verbose);

/*!
  \fn void set_memory_ram_ggems_manager(GGEMSManager* ggems_manager, bool const is_memory_ram_verbose)
  \param ggems_manager - pointer on the singleton
  \param is_memory_ram_verbose - flag on memory RAM verbose
  \brief Set the memory RAM verbosity
*/
extern "C" GGEMS_EXPORT void set_memory_ram_ggems_manager(GGEMSManager* ggems_manager, bool const is_memory_ram_verbose);

/*!
  \fn void set_processes_ggems_manager(GGEMSManager* ggems_manager, bool const is_processes_verbose)
  \param ggems_manager - pointer on the singleton
  \param is_processes_verbose - flag on processes verbose
  \brief Set the processes verbosity
*/
extern "C" GGEMS_EXPORT void set_processes_ggems_manager(GGEMSManager* ggems_manager, bool const is_processes_verbose);

/*!
  \fn void set_range_cuts_ggems_manager(GGEMSManager* ggems_manager, bool const is_range_cuts_verbose)
  \param ggems_manager - pointer on the singleton
  \param is_range_cuts_verbose - flag on range cuts verbose
  \brief Set the range cuts verbosity
*/
extern "C" GGEMS_EXPORT void set_range_cuts_ggems_manager(GGEMSManager* ggems_manager, bool const is_range_cuts_verbose);

/*!
  \fn void set_random_ggems_manager(GGEMSManager* ggems_manager, bool const is_random_verbose)
  \param ggems_manager - pointer on the singleton
  \param is_random_verbose - flag on random verbose
  \brief Set the random verbosity
*/
extern "C" GGEMS_EXPORT void set_random_ggems_manager(GGEMSManager* ggems_manager, bool const is_random_verbose);

/*!
  \fn void set_tracking_ggems_manager(GGEMSManager* ggems_manager, bool const is_tracking_verbose, GGint const particle_id_tracking)
  \param ggems_manager - pointer on the singleton
  \param is_tracking_verbose - flag on tracking verbose
  \param particle_id_tracking - particle id for tracking
  \brief Set the tracking verbosity
*/
extern "C" GGEMS_EXPORT void set_tracking_ggems_manager(GGEMSManager* ggems_manager, bool const is_tracking_verbose, GGint const particle_id_tracking);

/*!
  \fn void run_ggems_manager(GGEMSManager* ggems_manager)
  \param ggems_manager - pointer on the singleton
  \brief Run the GGEMS simulation
*/
extern "C" GGEMS_EXPORT void run_ggems_manager(GGEMSManager* ggems_manager);

#endif // End of GUARD_GGEMS_GLOBAL_GGEMSMANAGER_HH
