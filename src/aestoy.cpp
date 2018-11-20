#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <aestoy/aestoy.h>
#include "aes_csts.h"
#include "intmem.h"
#include "vec.h"

using namespace intmem;

namespace aestoy {

namespace {

template <class T, size_t N>
__attribute__((always_inline)) std::array<T, N>& operator^=(std::array<T,N>& __restrict A, std::array<T,N> const& __restrict B)
{
  for (size_t i = 0; i < N; ++i) {
    A[i] ^= B[i];
  }
  return A;
}

using RkBlock = std::array<uint8_t, 4>;
RkBlock* GetRksBlock(RoundKey* RKeys, size_t n)
{
  size_t idx = n*4;
  return (RkBlock*) &RKeys[idx/16][idx%16];
}

template <class Container>
void ApplySB(Container& C)
{
  for (uint8_t& V: C) {
    V = RJD_SBOX[V];
  }
}

template <size_t Idx0_, size_t Idx1_, size_t Idx2_, size_t Idx3_>
struct SRIdx {
  static constexpr size_t Idx0 = Idx0_;
  static constexpr size_t Idx1 = Idx1_;
  static constexpr size_t Idx2 = Idx2_;
  static constexpr size_t Idx3 = Idx3_;
};

using SR0 = SRIdx< 0, 5,10,15>;
using SR1 = SRIdx< 4, 9,14, 3>;
using SR2 = SRIdx< 8,13, 2, 7>;
using SR3 = SRIdx<12, 1, 6,11>;

using InvSR0 = SRIdx< 0,13,10, 7>;
using InvSR1 = SRIdx< 4, 1,14,11>;
using InvSR2 = SRIdx< 8, 5, 2,15>;
using InvSR3 = SRIdx<12, 9, 6, 3>;


static constexpr std::array<uint8_t, 16> SR = {
  SR0::Idx0,SR0::Idx1,SR0::Idx2,SR0::Idx3,
  SR1::Idx0,SR1::Idx1,SR1::Idx2,SR1::Idx3,  
  SR2::Idx0,SR2::Idx1,SR2::Idx2,SR2::Idx3,  
  SR3::Idx0,SR3::Idx1,SR3::Idx2,SR3::Idx3};  

template <class SR>
uint32_t SRSBMixC(uint8_t const* S, std::array<uint32_t, 256> const& T)
{
  return T[S[SR::Idx0]] ^
         rol<8>(T[S[SR::Idx1]]) ^
         rol<16>(T[S[SR::Idx2]]) ^
         rol<24>(T[S[SR::Idx3]]);
}

uint32_t InvMixC(uint8_t const* S)
{
  return RJD_INVMC[S[0]] ^
         rol<8>(RJD_INVMC[S[1]]) ^
         rol<16>(RJD_INVMC[S[2]]) ^
         rol<24>(RJD_INVMC[S[3]]);
}

using AESState = std::array<uint8_t, 16>;

void dump_state(AESState const& S) {
  for (uint8_t C: S) {
    printf("%02X", C);
  }
  printf("\n");
}

template <class SR0_, class SR1_, class SR2_, class SR3_>
void AESProcessBlock(AESCtx const& C, std::array<uint32_t, 256> const& T, std::array<uint8_t, 256> const& SB, uint8_t* Out, uint8_t const* In)
{
  alignas(uint32_t) AESState State;
  static constexpr size_t SR_[] = {
    SR0_::Idx0, SR0_::Idx1, SR0_::Idx2, SR0_::Idx3,
    SR1_::Idx0, SR1_::Idx1, SR1_::Idx2, SR1_::Idx3,
    SR2_::Idx0, SR2_::Idx1, SR2_::Idx2, SR2_::Idx3,
    SR3_::Idx0, SR3_::Idx1, SR3_::Idx2, SR3_::Idx3};

  memcpy(&State[0], In, sizeof(State));
  State ^= C.Keys[0];
#ifndef NDEBUG
  dump_state(State);
#endif

#pragma unroll
  for (size_t R = 1; R < 10; ++R) {
    AESState CurState = State;
    auto* S = &CurState[0];

    storeu_le<uint32_t>(&State[0],  SRSBMixC<SR0_>(S, T));
    storeu_le<uint32_t>(&State[4],  SRSBMixC<SR1_>(S, T));
    storeu_le<uint32_t>(&State[8],  SRSBMixC<SR2_>(S, T));
    storeu_le<uint32_t>(&State[12], SRSBMixC<SR3_>(S, T));
    State ^= C.Keys[R];
#ifndef NDEBUG
  dump_state(State);
#endif
  }

  // Output encoding of these rounds if "1"!

  // Last round is only sub bytes and shiftrows
  AESState CurState = State;
  auto* S = &CurState[0];
  for (size_t i = 0; i < 16; ++i) {
    const size_t InIdx = SR_[i];
    State[i] = SB[CurState[InIdx]];
  }
#ifndef NDEBUG
  dump_state(State);
#endif
  State ^= C.Keys[10];

  memcpy(Out, &State[0], sizeof(State));
}

} // anonymous

#ifdef AESTOY_ENABLE_VECTOR_IMPL
namespace {
Vec16 SRSBMixC_all(Vec16 S)
{
  auto S0 = vec_gather_u32<
    SR3::Idx0,
    SR2::Idx0,
    SR1::Idx0,
    SR0::Idx0>(&RJD_Te0[0], S);

  auto S1 = vec_gather_u32<
    SR3::Idx1,
    SR2::Idx1,
    SR1::Idx1,
    SR0::Idx1>(&RJD_Te1[0], S);

  auto S2 = vec_gather_u32<
    SR3::Idx2,
    SR2::Idx2,
    SR1::Idx2,
    SR0::Idx2>(&RJD_Te2[0], S);

  auto S3 = vec_gather_u32<
    SR3::Idx3,
    SR2::Idx3,
    SR1::Idx3,
    SR0::Idx3>(&RJD_Te3[0], S);

  return xor_(xor_(S0, S1), xor_(S2, S3));
}

void dump_state(Vec16 V) {
  alignas(Vec16) uint8_t V_[16];
  vec_store(V_, V);
  for (uint8_t C: V_) {
    printf("%02X", C);
  }
  printf("\n");
}
} // anonymous

void AESEncryptBlock(AESCtx const& C, uint8_t* Out, uint8_t const* In)
{
  Vec16 State = vec_loadu(In);
  State = xor_(State, vec_loadu(C.Keys[0]));
#ifndef NDEBUG
  dump_state(State);
#endif

  for (size_t R = 1; R < 10; ++R) {
    State = SRSBMixC_all(State);
    State = xor_(State, vec_loadu(C.Keys[R]));
#ifndef NDEBUG
    dump_state(State);
#endif
  }

  // TODO: make this cleaner?
  alignas(Vec16) uint8_t Tmp[16];
  vec_store(Tmp, State);
  for (size_t I = 0; I < 16; ++I) {
    Out[I] = RJD_SBOX[Tmp[SR[I]]];
  }
  State = xor_(vec_load(Out), vec_loadu(C.Keys[10]));
  vec_storeu(Out, State);
}
#else
void AESEncryptBlock(AESCtx const& C, uint8_t* Out, uint8_t const* In)
{
  AESProcessBlock<SR0, SR1, SR2, SR3>(C, RJD_Te0, RJD_SBOX, Out, In);
}

#endif

void AESDecryptBlock(AESCtx const& C, uint8_t* Out, uint8_t const* In)
{
  AESProcessBlock<InvSR0, InvSR1, InvSR2, InvSR3>(C, RJD_Td0, RJD_SBOX_INV, Out, In);
}


void AESKeyInvertExpand(uint8_t* Key, uint8_t const* RndKey, const size_t Rnd)
{
  if (Rnd == 0) {
    memcpy(Key, RndKey, sizeof(RoundKey));
    return;
  }

  if (Rnd > NR) {
    return;
  }

  RoundKey RKeys[NR+1];
  // This is basically the inverse of the algorithm above!
  RkBlock Tmp;

  memcpy(&RKeys[Rnd], RndKey, sizeof(RoundKey));

  for (ssize_t BI = 4*(Rnd)-1; BI >= 0; --BI) {
    Tmp = *GetRksBlock(RKeys, BI+3);

    if ((BI % 4) == 0) {
      // Rotate
      auto First = Tmp[0];
      Tmp[0] = Tmp[1];
      Tmp[1] = Tmp[2];
      Tmp[2] = Tmp[3];
      Tmp[3] = First;

      ApplySB(Tmp);

      Tmp[0] ^= RC_TBL[(BI+4)/4];
    }

    auto const& Next = *GetRksBlock(RKeys, BI+4);
    Tmp ^= Next;

    *GetRksBlock(RKeys, BI) = Tmp;
  }
  memcpy(Key, &RKeys[0], sizeof(RoundKey));
}

void AESKeyExpand(AESCtx& Ctx, uint8_t const* Key)
{
  RoundKey* RKeys = &Ctx.Keys[0];
  memcpy(&RKeys[0], Key, sizeof(RoundKey));
  RkBlock Tmp;

  // A "block" is a 4-byte stride of round keys
  for (size_t BI = 4; BI < 4*(NR+1); ++BI) {
    Tmp = *GetRksBlock(RKeys, BI-1);

    if (BI % 4 == 0) {
      // Rotate
      auto First = Tmp[0];
      Tmp[0] = Tmp[1];
      Tmp[1] = Tmp[2];
      Tmp[2] = Tmp[3];
      Tmp[3] = First;

      ApplySB(Tmp);

      Tmp[0] ^= RC_TBL[BI/4];
    }

    auto const& Prev = *GetRksBlock(RKeys, BI-4);
    Tmp ^= Prev;

    *GetRksBlock(RKeys, BI) = Tmp;
  }
}

void AESPrepareForDecryption(AESCtx& Ctx)
{
  // Apply InvMixColumn on the AES keys, except the first and last one.
  // This allows to use a table-based implementation also for decryption, by
  // tabulating SB-1 with InvMixC-1.
  for (size_t K = 9; K > 0; --K) {
    auto& RK = Ctx.Keys[K];
    for (size_t i = 0; i < 16; i += 4) {
      storeu_le<uint32_t>(&RK[i], InvMixC(&RK[i]));
    }
  }
  std::reverse(std::begin(Ctx.Keys), std::end(Ctx.Keys));
}

} // aestoy
