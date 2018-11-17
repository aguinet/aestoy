#ifndef AESTOY_VEC_NEON_H
#define AESTOY_VEC_NEON_H

#include <arm_neon.h>

namespace {

using Vec16 = uint8x16_t;

Vec16 vec_loadu(void const* V) {
  return vld1q_u8((const uint8_t*)V);
}

Vec16 vec_loadu(std::array<uint8_t, 16> const& V) {
  return vec_loadu(&V[0]);
}

Vec16 vec_load(uint8_t const* V) {
  return vec_loadu(V);
}

Vec16 vec_load(std::array<uint8_t, 16> const& V) {
  return vec_loadu(V);
}

void vec_storeu(void* Out, Vec16 V) {
#ifdef __aarch64__
  vstrq_p128(Out, V);
#else
  // TODO: endianess
  memcpy(Out, &V, sizeof(V));
#endif
}

void vec_store(void* Out, Vec16 V) {
  vec_storeu(Out, V);
}

Vec16 xor_(Vec16 A, Vec16 B) {
  return veorq_u8(A,B);
}

Vec16 vec_set(uint32_t A, uint32_t B, uint32_t C, uint32_t D) {
  return reinterpret_cast<uint8x16_t>(uint32x4_t{A, B, C, D});
}

Vec16 vec_set(std::array<uint8_t, 16> V) {
  return Vec16{V[0],V[1],V[2],V[3],V[4],V[5],V[6],V[7],V[8],V[9],V[10],V[11],V[12],V[13],V[14],V[15]};
}

template <size_t I0, size_t I1, size_t I2, size_t I3>
Vec16 vec_gather_u32(uint32_t const* BaseAddr, Vec16 V) {
  alignas(Vec16) uint32_t Ret[] = {BaseAddr[V[I0]], BaseAddr[V[I1]], BaseAddr[V[I2]], BaseAddr[V[I3]]};
  return vec_loadu(&Ret[0]);
}

} // anonymous

#endif
