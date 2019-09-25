#include "crypto_hash.h"

#include "api.h"
#include "XoodyakCyclist.h"

int crypto_hash(
        unsigned char *out,
        const unsigned char *in,
        unsigned long long inlen)
{
    Cyclist_Instance instance;
    Cyclist_Initialize(&instance, NULL, 0, NULL, 0, NULL, 0);
    Cyclist_Absorb(&instance, in, inlen);
    Cyclist_Squeeze(&instance, out, CRYPTO_BYTES);
    return 0;
}
