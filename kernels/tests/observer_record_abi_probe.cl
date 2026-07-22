#include "core/observer/GGEMSObserverRecord.clh"

__kernel void observer_record_abi_probe(__global ulong *layout,
                                        __global GGEMSObserverRecord *records) {
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
}
