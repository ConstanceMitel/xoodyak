/*
Implementation by the Keccak Team, namely, Guido Bertoni, Joan Daemen,
Michaël Peeters, Gilles Van Assche and Ronny Van Keer,
hereby denoted as "the implementer".

For more information, feedback or questions, please refer to our website:
https://keccak.team/

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_crypto_aead.h"
#include "crypto_aead.h"

int do_test_crypto_aead(
        const unsigned char *key,           size_t keyLen,
        const unsigned char *nonce,         size_t nonceLen,
        const unsigned char *AD,            size_t ADlen,
        const unsigned char *plaintext,     size_t plaintextLen,
        const unsigned char *ciphertext,
        unsigned int tagLen,
        unsigned char *temp1,
        unsigned char *temp2)
{
    unsigned long long clen;
    if (crypto_aead_encrypt(temp1, &clen, plaintext, plaintextLen, AD, ADlen, 0, nonce, key) != 0) {
        printf("!!! crypto_aead_encrypt() did not return 0.\n");
        return 1;
    }
    printf("OK. crypto_aead_encrypt() did return 0.\n");

    if (clen != plaintextLen+tagLen) {
        printf("!!! clen does not have the expected value.\n");
        return 1;
    }
    printf("OK. clen does have the expected value.\n");

    /* if (memcmp(temp1, ciphertext, (size_t) clen) != 0) { */
    /*     printf("!!! The output of crypto_aead_encrypt() is not as expected.\n"); */
    /*     return 1; */
    /* } */

    unsigned long long mlen;
    if (crypto_aead_decrypt(temp2, &mlen, 0, temp1, plaintextLen+tagLen, AD, ADlen, nonce, key) != 0) {
        printf("!!! crypto_aead_decrypt() did not return 0.\n");
        return 1;
    }
    printf("OK. crypto_aead_decrypt() did return 0.\n");

    if (mlen != plaintextLen) {
        printf("!!! mlen does not have the expected value.\n");
        return 1;
    }
    printf("OK. mlen does have the expected value.\n");

    if (memcmp(temp2, plaintext, (size_t) mlen) != 0) {
        printf("!!! The output of crypto_aead_decrypt() is not as expected.\n");
        return 1;
    }
    printf("OK. The output of crypto_aead_decrypt() is as expected.\n");

    temp1[0] ^= 0x01;
    if (crypto_aead_decrypt(temp2, &mlen, 0, temp1, plaintextLen+tagLen, AD, ADlen, nonce, key) == 0) {
        printf("!!! Forgery found :-)\n");
        return 1;
    }
    printf("OK. Forgery detected\n");

    for(size_t i=0; i<plaintextLen; i++) if (temp2[i] != 0) {
        printf("!!! The output buffer is not cleared.\n");
        return 1;
    }
    printf("OK. The output buffer is cleared.\n");

    printf("Self-test OK\n");
    return 0;
}

int test_crypto_aead(
        const unsigned char *key,           size_t keyLen,
        const unsigned char *nonce,         size_t nonceLen,
        const unsigned char *AD,            size_t ADlen,
        const unsigned char *plaintext,     size_t plaintextLen,
        const unsigned char *ciphertext,
        unsigned int tagLen)
{
    unsigned char *temp1 = malloc((size_t) plaintextLen + tagLen);
    unsigned char *temp2 = malloc((size_t) plaintextLen + tagLen);

    int retcode = do_test_crypto_aead(
            key, keyLen,
            nonce, nonceLen,
            AD, ADlen,
            plaintext, plaintextLen,
            ciphertext,
            tagLen,
            temp1, temp2);

    free(temp1);
    free(temp2);
    return retcode;
}

