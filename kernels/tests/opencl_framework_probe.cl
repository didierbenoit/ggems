__kernel void opencl_framework_probe(__global uint *values, uint increment) {
  size_t const index = get_global_id(0);
  values[index] += increment;
}
