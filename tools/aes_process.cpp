#include <cstdio>
#include <cstdlib>

#include <aestoy/aestoy.h>
#include <aestoy/tools.h>

using namespace aestoy;

int main(int argc, char** argv)
{
  if (argc < 4) {
    fprintf(stderr, "Usage: %s direction key_hex block_hex\n", argv[0]);
    fprintf(stderr, "Encrypt (direction = 0)/decrypt (direction = 1) block_hex (16 bytes) with key_hex\n");
    return 1;
  }

  const unsigned Direction = atoi(argv[1]);
  if (Direction >= 2) {
    fprintf(stderr, "Direction must be 0 (encrypt) or 1 (decrypt)\n");
    return 1;
  }

  const char* KeyHex = argv[2];
  auto Key = fromHex(KeyHex);
  if (Key.size() != 16) {
    fprintf(stderr, "Size of key must be 16 bytes!\n");
    return 1;
  }

  const char* DataHex = argv[3];
  auto Data = fromHex(DataHex);
  if (Data.size() != 16) {
    fprintf(stderr, "Block must be 16 bytes!\n");
    return 1;
  }
  
  AESCtx Ctx;
  AESKeyExpand(Ctx, &Key[0]);
  if (Direction == 0) {
    AESEncryptBlock(Ctx, &Data[0], &Data[0]);
  }
  else {
    AESDecryptBlock(Ctx, &Data[0], &Data[0]);
  }

  dumpHex(stdout, std::begin(Data), std::end(Data));
  printf("\n");

  return 0;
}

