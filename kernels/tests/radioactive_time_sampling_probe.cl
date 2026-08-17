#include "radioactivity/GGEMSRadioactiveTimeSampling.clh"

typedef struct GGEMSRadioactiveTimeSamplingResult {
  ulong time_ps;
  uint ticket;
  float relative;
} GGEMSRadioactiveTimeSamplingResult;

__kernel void radioactive_time_sampling_probe(
    __global ulong const *starts, __global ulong const *stops,
    __global float const *scaled_decays, __global uint const *raw_words,
    __global GGEMSRadioactiveTimeSamplingResult *results, uint sample_count) {
  uint const index = (uint)(get_global_id(0));
  if (index >= sample_count) {
    return;
  }

  float const uniform = GGEMS_UIntToUniform01(raw_words[index]);
  float const relative =
      GGEMS_ComputeRadioactiveTimeRelative(uniform, scaled_decays[index]);

  results[index].time_ps = GGEMS_SampleRadioactiveTimeFromRaw(
      starts[index], stops[index], scaled_decays[index], raw_words[index]);
  results[index].ticket = GGEMS_QuantizeRadioactiveTimeRelative(relative);
  results[index].relative = relative;
}
