#include <cstdio>
#include <cstring>

#include <aestoy/aestoy.h>

using namespace aestoy;

#define BLOCK_SIZE 16

int test_encrypt_decrypt(uint8_t const* Key, uint8_t const* Pt, uint8_t const* CtRef)
{
  // Test encryption
  uint8_t Out[BLOCK_SIZE];
  AESCtx Ctx;
  AESKeyExpand(Ctx, Key);
  AESEncryptBlock(Ctx, Out, Pt);
  if (memcmp(Out, CtRef, 16) != 0) {
    return false;
  }

  // Test decryption
  AESDecryptBlock(Ctx, Out, CtRef);
  if (memcmp(Out, Pt, BLOCK_SIZE) != 0) {
    fprintf(stderr, "Invalid test!\n");
    return 1;
  }
  return 0;
}

int test_zeros(uint8_t const* CT0, uint8_t const* CT1)
{
  // Encrypt zeros with the zeros key, check if this is CT0 
  // Then encrypt CT0 with the zeros keys, and check it its CT1

  uint8_t Out[BLOCK_SIZE];
  const uint8_t BlockZeros[BLOCK_SIZE] = {0};
  AESCtx Ctx;
  AESKeyExpand(Ctx, &BlockZeros[0]);
  
  // Test encryption
  AESEncryptBlock(Ctx, Out, BlockZeros);
  if (memcmp(Out, CT0, BLOCK_SIZE) != 0) {
    fprintf(stderr, "Error in test_zeros (1)!\n");
    return 1;
  }
  AESEncryptBlock(Ctx, Out, Out);
  if (memcmp(Out, CT1, BLOCK_SIZE) != 0) {
    fprintf(stderr, "Error in test_zeros (2)!\n");
    return 1;
  }
  return 0;
}

int main()
{
  int Ret = 0;
  {
    uint8_t PT[] =  {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    uint8_t key[] = {
      0x2b, 0x7e, 0x15, 0x16,
      0x28, 0xae, 0xd2, 0xa6,
      0xab, 0xf7, 0x15, 0x88,
      0x09, 0xcf, 0x4f, 0x3c};
    uint8_t CT_ref[] = {0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32};
    Ret |= test_encrypt_decrypt(key, PT, CT_ref);
  }

  const uint8_t ZeroBlock[16] = {0};
  {
    uint8_t CT0[] = {0x66, 0xE9, 0x4B, 0xD4, 0xEF, 0x8A, 0x2C, 0x3B, 0x88, 0x4C, 0xFA, 0x59, 0xCA, 0x34, 0x2B, 0x2E};
    uint8_t CT1[] = {0xF7, 0x95, 0xBD, 0x4A, 0x52, 0xE2, 0x9E, 0xD7, 0x13, 0xD3, 0x13, 0xFA, 0x20, 0xE9, 0x8D, 0xBC};
    Ret |= test_zeros(CT0, CT1);
  }

  return Ret;
}
