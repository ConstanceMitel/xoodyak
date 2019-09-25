#ifndef _XoodyakCyclist_h_
#define _XoodyakCyclist_h_

#include <stdint.h> // uint8_t etc.
#include <stddef.h> // size_t

#include "Xoodoo-SnP.h"

// TODO: constancts for C_D/C_U?

#define Xoodyak_rateHash      16
#define Xoodyak_rateKin       44
#define Xoodyak_rateKout      24
#define Xoodyak_lengthRatchet 16
#define Xoodyak_tagLength     16

#define Xoodyak_Permutation Xoodoo_Permute_12rounds

typedef enum {
    XOODYAK_PHASE_UP,
    XOODYAK_PHASE_DOWN,
} Cyclist_Phase;

typedef enum {
    XOODYAK_MODE_HASH,
    XOODYAK_MODE_KEYED,
} Cyclist_Mode;

typedef struct {
    Cyclist_Phase phase;
    Cyclist_Mode mode;
    size_t rate_absorb;
    size_t rate_squeeze;
    uint8_t state[Xoodoo_stateSizeInBytes];
} Cyclist_Instance;

void Cyclist_Initialize(
        Cyclist_Instance * const instance,
        uint8_t const *K, uint8_t K_len,
        uint8_t const *id, uint8_t id_len,
        uint8_t const *counter, size_t counter_len);

void Cyclist_Absorb(
        Cyclist_Instance *instance,
        uint8_t const *X, size_t X_len);

void Cyclist_Encrypt(
        Cyclist_Instance *instance,
        uint8_t *C,
        uint8_t const *P, size_t P_len);

void Cyclist_Decrypt(
        Cyclist_Instance *instance,
        uint8_t *P,
        uint8_t const *C, size_t C_len);

void Cyclist_Squeeze(
        Cyclist_Instance *instance,
        uint8_t *Y, size_t Y_len);

void Cyclist_SqueezeKey(
        Cyclist_Instance *instance,
        uint8_t *K, size_t K_len);

void Cyclist_Ratched(Cyclist_Instance *instance);

#endif
