#ifndef _test_crypto_aead_h_
#define _test_crypto_aead_h_

#include <stddef.h>

int do_test_crypto_aead(
        const unsigned char *key,        size_t keyLen,
        const unsigned char *nonce,      size_t nonceLen,
        const unsigned char *AD,         size_t ADlen,
        const unsigned char *plaintext,  size_t plaintextLen,
        const unsigned char *ciphertext,
        unsigned int tagLen,
        unsigned char *temp1,
        unsigned char *temp2);

int test_crypto_aead(
        const unsigned char *key,        size_t keyLen,
        const unsigned char *nonce,      size_t nonceLen,
        const unsigned char *AD,         size_t ADlen,
        const unsigned char *plaintext,  size_t plaintextLen,
        const unsigned char *ciphertext,
        unsigned int tagLen);

#endif
