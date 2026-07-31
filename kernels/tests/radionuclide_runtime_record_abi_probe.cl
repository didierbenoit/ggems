#include "core/radioactivity/GGEMSRadionuclideEmissionRecord.clh"
#include "core/radioactivity/GGEMSRadionuclideGroupRange.clh"
#include "core/sources/GGEMSSourcePopulationRecord.clh"

typedef struct GGEMSSourcePopulationAlignmentProbe {
  uchar prefix;
  GGEMSSourcePopulationRecord record;
} GGEMSSourcePopulationAlignmentProbe;

typedef struct GGEMSRadionuclideEmissionAlignmentProbe {
  uchar prefix;
  GGEMSRadionuclideEmissionRecord record;
} GGEMSRadionuclideEmissionAlignmentProbe;

typedef struct GGEMSRadionuclideGroupAlignmentProbe {
  uchar prefix;
  GGEMSRadionuclideGroupRange range;
} GGEMSRadionuclideGroupAlignmentProbe;

__kernel void radionuclide_runtime_record_abi_probe(
    __global ulong *layout, __global GGEMSSourcePopulationRecord *populations,
    __global GGEMSRadionuclideEmissionRecord *emissions,
    __global GGEMSRadionuclideGroupRange *groups) {
  if (get_global_id(0) != 0U) {
    return;
  }

  GGEMSSourcePopulationRecord private_population;
  __private uchar const *population_base =
      (__private uchar const *)&private_population;
  layout[0] = (ulong)(sizeof(GGEMSSourcePopulationRecord));
  layout[1] =
      (ulong)((__private uchar const *)&private_population.population_mode -
              population_base);
  layout[2] = (ulong)((__private uchar const *)&private_population
                          .first_emission_index -
                      population_base);
  layout[3] =
      (ulong)((__private uchar const *)&private_population.emission_count -
              population_base);
  layout[4] =
      (ulong)((__private uchar const *)&private_population.scaled_decay -
              population_base);
  layout[5] = (ulong)((__global uchar const *)&populations[1] -
                      (__global uchar const *)&populations[0]);
  GGEMSSourcePopulationAlignmentProbe population_alignment;
  layout[6] = (ulong)((__private uchar const *)&population_alignment.record -
                      (__private uchar const *)&population_alignment);

  GGEMSRadionuclideEmissionRecord private_emission;
  __private uchar const *emission_base =
      (__private uchar const *)&private_emission;
  layout[7] = (ulong)(sizeof(GGEMSRadionuclideEmissionRecord));
  layout[8] = (ulong)((__private uchar const *)&private_emission.particle_type -
                      emission_base);
  layout[9] = (ulong)((__private uchar const *)&private_emission
                          .energy_distribution_record_index -
                      emission_base);
  layout[10] =
      (ulong)((__private uchar const *)&private_emission.mono_energy_milli_eV -
              emission_base);
  layout[11] = (ulong)((__global uchar const *)&emissions[1] -
                       (__global uchar const *)&emissions[0]);
  GGEMSRadionuclideEmissionAlignmentProbe emission_alignment;
  layout[12] = (ulong)((__private uchar const *)&emission_alignment.record -
                       (__private uchar const *)&emission_alignment);

  GGEMSRadionuclideGroupRange private_group;
  __private uchar const *group_base = (__private uchar const *)&private_group;
  layout[13] = (ulong)(sizeof(GGEMSRadionuclideGroupRange));
  layout[14] = (ulong)((__private uchar const *)&private_group
                           .source_local_primary_begin -
                       group_base);
  layout[15] = (ulong)((__private uchar const *)&private_group.primary_count -
                       group_base);
  layout[16] = (ulong)((__global uchar const *)&groups[1] -
                       (__global uchar const *)&groups[0]);
  GGEMSRadionuclideGroupAlignmentProbe group_alignment;
  layout[17] = (ulong)((__private uchar const *)&group_alignment.range -
                       (__private uchar const *)&group_alignment);

  populations[1].population_mode = 37U;
  populations[1].first_emission_index = 41U;
  populations[1].emission_count = 43U;
  populations[1].scaled_decay = 1.5F;

  emissions[1].particle_type = 47U;
  emissions[1].energy_distribution_record_index = 53U;
  emissions[1].mono_energy_milli_eV = 59UL;

  groups[1].source_local_primary_begin = 61UL;
  groups[1].primary_count = 67UL;
}
