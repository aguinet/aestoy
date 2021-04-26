#include <cstdio>
#include <cstdlib>

#include <aestoy/aestoy.h>
#include <aestoy/tools.h>

using namespace aestoy;

int main(int argc, char** argv)
{
  if (argc <= 2) {
    fprintf(stderr, "Usage: %s round_key_hex round\n", argv[0]);
    return 1;
  }

  const char* RndKeyHex = argv[1];
  auto RndKey = fromHex(RndKeyHex);
  if (RndKey.size() != 16) {
    fprintf(stderr, "Size of key must be 16 bytes!\n");
    return 1;
  }

  unsigned Round = atoi(argv[2]);
  if (Round > 10) {
    fprintf(stderr, "Round must be between 0 and 10!\n");
    return 1;
  }
  
  uint8_t Key[16];
  AESKeyInvertExpand(Key, &RndKey[0], Round);

  dumpHex(stdout, std::begin(Key), std::end(Key));
  printf("\n");

  return 0;
}
