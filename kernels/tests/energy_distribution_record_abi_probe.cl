#include "sources/GGEMSEnergyDistributionRecord.clh"

typedef struct GGEMSEnergyDistributionRecordAlignmentProbe {
  uchar prefix;
  GGEMSEnergyDistributionRecord record;
} GGEMSEnergyDistributionRecordAlignmentProbe;

__kernel void energy_distribution_record_abi_probe(
    __global ulong *layout, __global GGEMSEnergyDistributionRecord *records,
    __global ulong *host_values) {
  if (get_global_id(0) != 0U) {
    return;
  }

  GGEMSEnergyDistributionRecord private_record;
  __private uchar const *record_base = (__private uchar const *)&private_record;

  layout[0] = (ulong)(sizeof(GGEMSEnergyDistributionRecord));
  layout[1] = (ulong)((__private uchar const *)&private_record
                          .regular_bin_width_milli_eV -
                      record_base);
  layout[2] = (ulong)((__private uchar const *)&private_record.table_offset -
                      record_base);
  layout[3] =
      (ulong)((__private uchar const *)&private_record.distribution_type -
              record_base);
  layout[4] = (ulong)((__private uchar const *)&private_record.table_count -
                      record_base);
  layout[5] = (ulong)((__global uchar const *)&records[1] -
                      (__global uchar const *)&records[0]);

  GGEMSEnergyDistributionRecordAlignmentProbe alignment_probe;
  layout[6] = (ulong)((__private uchar const *)&alignment_probe.record -
                      (__private uchar const *)&alignment_probe);

  host_values[0] = records[0].regular_bin_width_milli_eV;
  host_values[1] = records[0].table_offset;
  host_values[2] = (ulong)(records[0].distribution_type);
  host_values[3] = (ulong)(records[0].table_count);

  records[1].regular_bin_width_milli_eV = 101UL;
  records[1].table_offset = 102UL;
  records[1].distribution_type =
      GGEMS_ENERGY_DISTRIBUTION_TYPE_REGULAR_SPECTRUM;
  records[1].table_count = 103U;
}
