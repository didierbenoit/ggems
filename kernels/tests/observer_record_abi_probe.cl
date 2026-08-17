#include "observer/GGEMSObserverRecord.clh"

typedef struct GGEMSObserverConfigAlignmentProbe {
  uchar prefix;
  GGEMSObserverConfigRecord config;
} GGEMSObserverConfigAlignmentProbe;

__kernel void
observer_record_abi_probe(__global ulong *layout,
                          __global GGEMSObserverRecord *records,
                          __global GGEMSObserverConfigRecord *configs) {
  if (get_global_id(0) != 0U) {
    return;
  }

  GGEMSObserverRecord private_record;

  __private uchar const *base = (__private uchar const *)&private_record;

#define GGEMS_WRITE_RECORD_OFFSET(INDEX, FIELD)                                \
  layout[INDEX] = (ulong)((__private uchar const *)&private_record.FIELD - base)

  layout[0] = (ulong)(sizeof(GGEMSObserverRecord));
  GGEMS_WRITE_RECORD_OFFSET(1, run_id);
  GGEMS_WRITE_RECORD_OFFSET(2, global_primary_id);
  GGEMS_WRITE_RECORD_OFFSET(3, source_local_primary_id);
  GGEMS_WRITE_RECORD_OFFSET(4, global_particle_id);
  GGEMS_WRITE_RECORD_OFFSET(5, track_id);
  GGEMS_WRITE_RECORD_OFFSET(6, parent_track_id);
  GGEMS_WRITE_RECORD_OFFSET(7, time_ps);
  GGEMS_WRITE_RECORD_OFFSET(8, position_x_pm);
  GGEMS_WRITE_RECORD_OFFSET(9, position_y_pm);
  GGEMS_WRITE_RECORD_OFFSET(10, position_z_pm);
  GGEMS_WRITE_RECORD_OFFSET(11, record_kind);
  GGEMS_WRITE_RECORD_OFFSET(12, particle_type);
  GGEMS_WRITE_RECORD_OFFSET(13, status);
  GGEMS_WRITE_RECORD_OFFSET(14, generation);
  GGEMS_WRITE_RECORD_OFFSET(15, direction_x);
  GGEMS_WRITE_RECORD_OFFSET(16, direction_y);
  GGEMS_WRITE_RECORD_OFFSET(17, direction_z);
  GGEMS_WRITE_RECORD_OFFSET(18, direction_w);
  GGEMS_WRITE_RECORD_OFFSET(19, energy_milli_eV);
  GGEMS_WRITE_RECORD_OFFSET(20, deposited_energy_milli_eV);
  GGEMS_WRITE_RECORD_OFFSET(21, weight);
  GGEMS_WRITE_RECORD_OFFSET(22, source_index);

#undef GGEMS_WRITE_RECORD_OFFSET

  layout[23] = (ulong)((__global uchar const *)&records[1] -
                       (__global uchar const *)&records[0]);

  records[0].energy_milli_eV = 101UL;
  records[0].deposited_energy_milli_eV = 202UL;
  records[0].weight = 0.25f;
  records[0].source_index = 3U;

  records[1].energy_milli_eV = 303UL;
  records[1].deposited_energy_milli_eV = 404UL;
  records[1].weight = 0.75f;
  records[1].source_index = 5U;

  GGEMSObserverConfigRecord private_config;
  __private uchar const *config_base = (__private uchar const *)&private_config;

#define GGEMS_WRITE_CONFIG_OFFSET(INDEX, FIELD)                                \
  layout[INDEX] =                                                              \
      (ulong)((__private uchar const *)&private_config.FIELD - config_base)

  layout[24] = (ulong)(sizeof(GGEMSObserverConfigRecord));
  GGEMS_WRITE_CONFIG_OFFSET(25, enabled);
  GGEMS_WRITE_CONFIG_OFFSET(26, capture_first_primary_count_per_source);
  GGEMS_WRITE_CONFIG_OFFSET(27, capture_specific_primary_enabled);
  GGEMS_WRITE_CONFIG_OFFSET(28, capture_source_index);
  GGEMS_WRITE_CONFIG_OFFSET(29, capture_source_local_primary_id);

#undef GGEMS_WRITE_CONFIG_OFFSET

  layout[30] = (ulong)((__global uchar const *)&configs[1] -
                       (__global uchar const *)&configs[0]);

  GGEMSObserverConfigAlignmentProbe alignment_probe;
  __private uchar const *alignment_base =
      (__private uchar const *)&alignment_probe;

  layout[31] = (ulong)((__private uchar const *)&alignment_probe.config -
                       alignment_base);

  configs[0].enabled = 1U;
  configs[0].capture_first_primary_count_per_source = 3U;
  configs[0].capture_specific_primary_enabled = 1U;
  configs[0].capture_source_index = 7U;
  configs[0].capture_source_local_primary_id = 11UL;

  configs[1].enabled = 0U;
  configs[1].capture_first_primary_count_per_source = 5U;
  configs[1].capture_specific_primary_enabled = 1U;
  configs[1].capture_source_index = 9U;
  configs[1].capture_source_local_primary_id = 13UL;
}
