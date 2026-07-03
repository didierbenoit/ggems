__kernel void vec_add_svm(__global const float *A, __global const float *B,
                          __global float *C, unsigned int const n) {
  size_t i = get_global_id(0);
  if (i < n) {
    C[i] = A[i] + B[i];
  }
}
