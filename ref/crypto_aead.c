#include "string.h" // memcmp/memset

#include "crypto_aead.h"

#include "api.h"
#include "XoodyakCyclist.h"

int crypto_aead_encrypt(
        unsigned char *c, unsigned long long *clen,
        const unsigned char *m, unsigned long long mlen,
        const unsigned char *ad, unsigned long long adlen,
        const unsigned char *nsec,
        const unsigned char *npub,
        const unsigned char *k
        )
{
    Cyclist_Instance instance;
    Cyclist_Initialize(&instance, k, CRYPTO_KEYBYTES, NULL, 0, NULL, 0);

    Cyclist_Absorb(&instance, npub, CRYPTO_NPUBBYTES);
    Cyclist_Absorb(&instance, ad, adlen);
    Cyclist_Encrypt(&instance, c, m, mlen);

    Cyclist_Squeeze(&instance, c + mlen, Xoodyak_tagLength);

    *clen = mlen + Xoodyak_tagLength;

    return 0;
}

int crypto_aead_decrypt(
        unsigned char *m, unsigned long long *mlen,
        unsigned char *nsec,
        const unsigned char *c, unsigned long long clen,
        const unsigned char *ad, unsigned long long adlen,
        const unsigned char *npub,
        const unsigned char *k
        )
{
    *mlen = clen - Xoodyak_tagLength;

    Cyclist_Instance instance;
    Cyclist_Initialize(&instance, k, CRYPTO_KEYBYTES, NULL, 0, NULL, 0);

    Cyclist_Absorb(&instance, npub, CRYPTO_NPUBBYTES);
    Cyclist_Absorb(&instance, ad, adlen);
    Cyclist_Decrypt(&instance, m, c, *mlen);

    uint8_t tag[Xoodyak_tagLength];
    Cyclist_Squeeze(&instance, tag, Xoodyak_tagLength);

    if (memcmp(tag, c + *mlen, Xoodyak_tagLength) != 0) {
        memset(m, 0x00, *mlen);
        return 1;
    } else {
        return 0;
    }
}
