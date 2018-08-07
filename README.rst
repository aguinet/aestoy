AESToy: a tiny C++ AES implementation
=====================================

WARNING
-------

This implementation is for educational-purposes only. It should never be used in
production environment, as it is probably vulnerable to lots of side-channels
and potentially other kind of attacks.

For this purpose, the library is licensed to GPLv3, so that it won't end up
"accidentally" in a proprietary software.

Introduction
------------

AESToy is a minimal C++ implementation of the AES128 block cipher. It is meant
for educational purpose only, and to provide a simple implementation that is
easily readable. It also provides various tools to play with AES, like
dumping/inverting the key schedule, block encryption/decryption, and so on...

Compilation
-----------

This needs no dependencies. You can compile it using CMake:

.. code:: bash

  $ cd /path/to/src && mkdir build && cd build
  $ cmake -DCMAKE_BUILD_TYPE=release .. && make
  $ make test

Usage
-----

There are some tools that could be useful:

aes_keyexpand
*************

This dumps the key schedule on stdout::

  $ ./tools/aes_keyexpand 000102030405060708090A0B0C0D0E0F
  000102030405060708090A0B0C0D0E0F
  D6AA74FDD2AF72FADAA678F1D6AB76FE
  B692CF0B643DBDF1BE9BC5006830B3FE
  B6FF744ED2C2C9BF6C590CBF0469BF41
  47F7F7BC95353E03F96C32BCFD058DFD
  3CAAA3E8A99F9DEB50F3AF57ADF622AA
  5E390F7DF7A69296A7553DC10AA31F6B
  14F9701AE35FE28C440ADF4D4EA9C026
  47438735A41C65B9E016BAF4AEBF7AD2
  549932D1F08557681093ED9CBE2C974E
  13111D7FE3944A17F307A78B4D2B30C5

aes_invertkeyexpand
*******************

This reverts the key schedule, starting from a round key, and print the original key::

  $ ./tools/aes_invertkeyexpand 5E390F7DF7A69296A7553DC10AA31F6B 6
  000102030405060708090A0B0C0D0E0F

aes_process
***********

This encrypt/decrypt a block using AES::

  $ $ ./tools/aes_process 0 000102030405060708090A0B0C0D0E0F AABBCCDDEEFF0001AABBCCDDEEFF0001
  9999F4D833C97BBD72C93449EDDEE73F
  $ ./tools/aes_process 1 000102030405060708090A0B0C0D0E0F 9999F4D833C97BBD72C93449EDDEE73F
  AABBCCDDEEFF0001AABBCCDDEEFF0001

Final note
----------

Again, do not use this in production!
