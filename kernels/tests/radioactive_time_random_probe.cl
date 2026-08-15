#include "core/radioactivity/GGEMSRadioactiveTimeSampling.clh"
#include "core/random/GGEMSRandom.clh"

__kernel void
radioactive_time_random_probe(__global GGEMSRandomState *random_states,
                              __global uint *raw_values,
                              __global ulong *sampled_time, ulong time_start_ps,
                              ulong time_stop_ps, float scaled_decay) {
  if (get_global_id(0) != 0U) {
    return;
  }

  uint const time_word = GGEMS_RndmUInt32(random_states, 0U);
  raw_values[0] = time_word;
  sampled_time[0] = GGEMS_SampleRadioactiveTimeFromRaw(
      time_start_ps, time_stop_ps, scaled_decay, time_word);
  raw_values[1] = GGEMS_RndmUInt32(random_states, 0U);
}
