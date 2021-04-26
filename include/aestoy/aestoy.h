#ifndef AESTOY_H
#define AESTOY_H

#include <cstdint>
#include <array>

namespace aestoy {

static constexpr size_t NR = 10;

using RoundKey = std::array<uint8_t, 16>;
struct AESCtx
{
  RoundKey Keys[NR+1];
};

void AESKeyExpand(AESCtx& Ctx, uint8_t const* Key);
// Inverse the key expansiation. Rnd 0 is considered as the block cipher key,
// so that AESKeyInvertExpand(Key,RndKey,0) <=> memcpy(Key,RndKey,16);
void AESKeyInvertExpand(uint8_t* Key, uint8_t const* RoundKey, const size_t Rnd);
void AESEncryptBlock(AESCtx const& Ctx, uint8_t* Out, uint8_t const* In);
void AESDecryptBlock(AESCtx const& Ctx, uint8_t* Out, uint8_t const* In);

} // aestoy

#endif
