#include "core/sources/GGEMSSourceRecord.clh"
#include "core/sources/GGEMSSourceRunRange.clh"

typedef struct GGEMSSourceRecordAlignmentProbe {
  uchar prefix;
  GGEMSSourceRecord record;
} GGEMSSourceRecordAlignmentProbe;

typedef struct GGEMSSourceRunRangeAlignmentProbe {
  uchar prefix;
  GGEMSSourceRunRange range;
} GGEMSSourceRunRangeAlignmentProbe;

__kernel void source_record_abi_probe(__global ulong *layout,
                                      __global GGEMSSourceRecord *records,
                                      __global GGEMSSourceRunRange *ranges,
                                      __global ulong *host_values) {
  if (get_global_id(0) != 0U) {
    return;
  }

  GGEMSSourceRecord private_record;
  __private uchar const *record_base = (__private uchar const *)&private_record;

#define GGEMS_WRITE_RECORD_OFFSET(INDEX, FIELD)                                \
  layout[INDEX] =                                                              \
      (ulong)((__private uchar const *)&private_record.FIELD - record_base)

  layout[0] = (ulong)(sizeof(GGEMSSourceRecord));
  GGEMS_WRITE_RECORD_OFFSET(1, source_id);
  GGEMS_WRITE_RECORD_OFFSET(2, time_start_ps);
  GGEMS_WRITE_RECORD_OFFSET(3, time_stop_ps);
  GGEMS_WRITE_RECORD_OFFSET(4, energy_milli_eV);
  GGEMS_WRITE_RECORD_OFFSET(5, position_x_pm);
  GGEMS_WRITE_RECORD_OFFSET(6, position_y_pm);
  GGEMS_WRITE_RECORD_OFFSET(7, position_z_pm);
  GGEMS_WRITE_RECORD_OFFSET(8, source_type);
  GGEMS_WRITE_RECORD_OFFSET(9, emitted_particle_type);
  GGEMS_WRITE_RECORD_OFFSET(10, flags);
  GGEMS_WRITE_RECORD_OFFSET(11, reserved_0);
  GGEMS_WRITE_RECORD_OFFSET(12, axis_x_x);
  GGEMS_WRITE_RECORD_OFFSET(13, axis_x_y);
  GGEMS_WRITE_RECORD_OFFSET(14, axis_x_z);
  GGEMS_WRITE_RECORD_OFFSET(15, axis_y_x);
  GGEMS_WRITE_RECORD_OFFSET(16, axis_y_y);
  GGEMS_WRITE_RECORD_OFFSET(17, axis_y_z);
  GGEMS_WRITE_RECORD_OFFSET(18, axis_z_x);
  GGEMS_WRITE_RECORD_OFFSET(19, axis_z_y);
  GGEMS_WRITE_RECORD_OFFSET(20, axis_z_z);
  GGEMS_WRITE_RECORD_OFFSET(21, weight);
  GGEMS_WRITE_RECORD_OFFSET(22, emission_geometry_type);
  GGEMS_WRITE_RECORD_OFFSET(23, angular_distribution_type);
  GGEMS_WRITE_RECORD_OFFSET(24, geometry_size_x_pm);
  GGEMS_WRITE_RECORD_OFFSET(25, geometry_size_y_pm);
  GGEMS_WRITE_RECORD_OFFSET(26, focus_position_x_pm);
  GGEMS_WRITE_RECORD_OFFSET(27, focus_position_y_pm);
  GGEMS_WRITE_RECORD_OFFSET(28, focus_position_z_pm);

#undef GGEMS_WRITE_RECORD_OFFSET

  layout[29] = (ulong)((__global uchar const *)&records[1] -
                       (__global uchar const *)&records[0]);

  GGEMSSourceRecordAlignmentProbe record_alignment_probe;
  __private uchar const *record_alignment_base =
      (__private uchar const *)&record_alignment_probe;
  layout[30] = (ulong)((__private uchar const *)&record_alignment_probe.record -
                       record_alignment_base);

  GGEMSSourceRunRange private_range;
  __private uchar const *range_base = (__private uchar const *)&private_range;

  layout[31] = (ulong)(sizeof(GGEMSSourceRunRange));
  layout[32] =
      (ulong)((__private uchar const *)&private_range.projection_primary_begin -
              range_base);
  layout[33] = (ulong)((__private uchar const *)&private_range.primary_count -
                       range_base);
  layout[34] = (ulong)((__global uchar const *)&ranges[1] -
                       (__global uchar const *)&ranges[0]);

  GGEMSSourceRunRangeAlignmentProbe range_alignment_probe;
  __private uchar const *range_alignment_base =
      (__private uchar const *)&range_alignment_probe;
  layout[35] = (ulong)((__private uchar const *)&range_alignment_probe.range -
                       range_alignment_base);

  host_values[0] = records[0].source_id;
  host_values[1] = records[0].time_start_ps;
  host_values[2] = records[0].time_stop_ps;
  host_values[3] = records[0].energy_milli_eV;
  host_values[4] = as_ulong(records[0].position_x_pm);
  host_values[5] = as_ulong(records[0].position_y_pm);
  host_values[6] = as_ulong(records[0].position_z_pm);
  host_values[7] = (ulong)(records[0].source_type);
  host_values[8] = (ulong)(records[0].emitted_particle_type);
  host_values[9] = (ulong)(records[0].flags);
  host_values[10] = (ulong)(records[0].reserved_0);
  host_values[11] = (ulong)(as_uint(records[0].axis_x_x));
  host_values[12] = (ulong)(as_uint(records[0].axis_x_y));
  host_values[13] = (ulong)(as_uint(records[0].axis_x_z));
  host_values[14] = (ulong)(as_uint(records[0].axis_y_x));
  host_values[15] = (ulong)(as_uint(records[0].axis_y_y));
  host_values[16] = (ulong)(as_uint(records[0].axis_y_z));
  host_values[17] = (ulong)(as_uint(records[0].axis_z_x));
  host_values[18] = (ulong)(as_uint(records[0].axis_z_y));
  host_values[19] = (ulong)(as_uint(records[0].axis_z_z));
  host_values[20] = (ulong)(as_uint(records[0].weight));
  host_values[21] = (ulong)(records[0].emission_geometry_type);
  host_values[22] = (ulong)(records[0].angular_distribution_type);
  host_values[23] = records[0].geometry_size_x_pm;
  host_values[24] = records[0].geometry_size_y_pm;
  host_values[25] = as_ulong(records[0].focus_position_x_pm);
  host_values[26] = as_ulong(records[0].focus_position_y_pm);
  host_values[27] = as_ulong(records[0].focus_position_z_pm);
  host_values[28] = ranges[0].projection_primary_begin;
  host_values[29] = ranges[0].primary_count;

  records[1].source_id = 201UL;
  records[1].time_start_ps = 202UL;
  records[1].time_stop_ps = 203UL;
  records[1].energy_milli_eV = 204UL;
  records[1].position_x_pm = -205L;
  records[1].position_y_pm = 206L;
  records[1].position_z_pm = -207L;
  records[1].source_type = 208U;
  records[1].emitted_particle_type = 209U;
  records[1].flags = 210U;
  records[1].reserved_0 = 211U;
  records[1].axis_x_x = -7.25f;
  records[1].axis_x_y = 8.5f;
  records[1].axis_x_z = -9.75f;
  records[1].axis_y_x = 10.125f;
  records[1].axis_y_y = -11.25f;
  records[1].axis_y_z = 12.5f;
  records[1].axis_z_x = -13.75f;
  records[1].axis_z_y = 14.875f;
  records[1].axis_z_z = -15.5f;
  records[1].weight = 0.625f;
  records[1].emission_geometry_type = GGEMS_EMISSION_GEOMETRY_TYPE_ELLIPSE;
  records[1].angular_distribution_type =
      GGEMS_ANGULAR_DISTRIBUTION_TYPE_FOCUSED;
  records[1].geometry_size_x_pm = 212UL;
  records[1].geometry_size_y_pm = 213UL;
  records[1].focus_position_x_pm = -214L;
  records[1].focus_position_y_pm = 215L;
  records[1].focus_position_z_pm = -216L;

  ranges[1].projection_primary_begin = 401UL;
  ranges[1].primary_count = 402UL;
}
