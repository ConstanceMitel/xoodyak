#include <assert.h>
#include <stdbool.h>
#include <string.h> // memcpy

#include "XoodyakCyclist.h"


// =====================================================================
//                          Internal interface
// =====================================================================

// TODO: For now optimised for dword size (32 bit)
static inline void memxor(uint8_t *dest, uint8_t const *src, size_t len)
{
    // move dest to be DWORD aligned
    while (len > 0 && ((uintptr_t) dest & (uintptr_t) 0x7)) {
        *dest ^= *src;
        dest += 1;
        src += 1;
        len -= 1;
    }

    // handle full DWORDs
    while (len >= 4) {
        *(uint32_t *) dest ^= * (uint32_t *) src;
        dest += 4;
        src += 4;
        len -= 4;
    }

    // remaining bytes
    while (len > 0) {
        *dest ^= *src;
        dest += 1;
        src += 1;
        len -= 1;
    }
}

static void Up(
        Cyclist_Instance * const instance,
        uint8_t * const Yi, size_t const Yi_len,
        uint8_t const C_U)
{
    assert(Yi != NULL || Yi_len == 0);
    assert(Yi_len <= Xoodyak_rateKin);
    assert(instance->phase == XOODYAK_PHASE_DOWN);

    instance->phase = XOODYAK_PHASE_UP;

    if (instance->mode != XOODYAK_MODE_HASH) {
        Xoodoo_AddByte(instance->state, C_U, Xoodoo_stateSizeInBytes - 1);
    }
    Xoodyak_Permutation(&(instance->state));

    if (Yi != NULL) {
        Xoodoo_ExtractBytes(instance->state, Yi, 0, Yi_len);
    }
}

static void Down(
        Cyclist_Instance * const instance,
        uint8_t const * const X, size_t X_len,
        uint8_t const C_D)
{
    assert(X != NULL || X_len == 0);
    assert(X_len <= Xoodyak_rateKin);
    assert(instance->phase == XOODYAK_PHASE_UP);

    instance->phase = XOODYAK_PHASE_DOWN;

    Xoodoo_AddBytes(instance->state, X, 0, X_len);
    Xoodoo_AddByte(instance->state, 0x01, X_len);
    Xoodoo_AddByte(instance->state,
            (instance->mode == XOODYAK_MODE_HASH) ? (C_D & 0x01) : C_D,
            Xoodoo_stateSizeInBytes - 1);
}

static void AbsorbAny(
        Cyclist_Instance * const instance,
        uint8_t const *X, size_t X_len,
        size_t const r, uint8_t const C_D)
{
    assert(X != NULL || X_len == 0);

    size_t block_len;

    // First block, possibly empty block
    {
        block_len = X_len < r ? X_len : r;
        if (instance->phase != XOODYAK_PHASE_UP) {
            Up(instance, NULL, 0, 0x00);
        }
        Down(instance, X, block_len, C_D);

        X += block_len;
        X_len -= block_len;
    }

    // Rest of the blocks
    while (X_len != 0) {
        block_len = X_len < r ? X_len : r;

        Up(instance, NULL, 0, 0x00);
        Down(instance, X, block_len, 0x00);

        X += block_len;
        X_len -= block_len;
    }
}

static void AbsorbKey(
        Cyclist_Instance * const instance,
        uint8_t const * const K, uint8_t K_len,
        uint8_t const * const id, uint8_t id_len,
        uint8_t const * const counter, size_t counter_len)
{
    assert(K != NULL || K_len == 0);
    assert(id != NULL || id == 0);
    assert(K_len + id_len <= Xoodyak_rateKin - 1);
    assert(counter != NULL || counter_len == 0);
    assert(instance->mode == XOODYAK_MODE_HASH);

    instance->mode = XOODYAK_MODE_KEYED;
    instance->rate_absorb = Xoodoo_stateSizeInBytes - 4;;

    // Copy concatination of the key, id, and length of id in temp
    uint8_t temp[Xoodyak_rateKin];
    memcpy(temp, K, K_len);
    if (id_len != 0) {
        memcpy(temp + K_len, id, id_len);
    }
    *(temp + K_len + id_len) = id_len;

    // Absorb the concatination
    AbsorbAny(instance, temp, K_len + id_len + 1, instance->rate_absorb, 0x02);

    if (counter_len != 0) {
        AbsorbAny(instance, counter, counter_len, 1, 0x00);
    }
}

static void Crypt(
        Cyclist_Instance *instance,
        uint8_t *O,
        uint8_t const *I, size_t I_len,
        bool Decrypt_Flag)
{
    assert(I != NULL || I_len == 0);
    assert(O != NULL || I_len == 0);

    size_t block_len;

    // TODO: move both blocks in one loop? variable for first block C_U?
    // First block, possibly empty block
    {
        block_len = I_len < Xoodyak_rateKout ? I_len : Xoodyak_rateKout;

        Up(instance, O, block_len, 0x80);
        memxor(O, I, block_len);
        Down(instance, Decrypt_Flag ? O : I, block_len, 0x00);

        O += block_len;
        I += block_len;
        I_len -= block_len;
    }

    // Rest of the blocks
    while (I_len != 0) {
        block_len = I_len < Xoodyak_rateKout ? I_len : Xoodyak_rateKout;

        Up(instance, O, block_len, 0x00);
        memxor(O, I, block_len);
        Down(instance, Decrypt_Flag ? O : I, block_len, 0x00);

        O += block_len;
        I += block_len;
        I_len -= block_len;
    }
}

static void SqueezeAny(
        Cyclist_Instance * const instance,
        uint8_t *Y, size_t l,
        uint8_t const C_U)
{
    assert(Y != NULL || l == 0);

    size_t block_len;

    // TODO: move both loops in one loop? variable for first block C_U?
    {
        block_len = l < instance->rate_squeeze ? l : instance->rate_squeeze;
        Up(instance, Y, block_len, C_U);

        Y += block_len;
        l -= block_len;
    }

    while (l > 0) {
        Down(instance, NULL, 0, 0x00);

        block_len = l < instance->rate_squeeze ? l : instance->rate_squeeze;
        Up(instance, Y, block_len, 0x00);

        Y += block_len;
        l -= block_len;
    }
}


// =====================================================================
//                           Public interface
// =====================================================================

void Cyclist_Initialize(
        Cyclist_Instance * const instance,
        uint8_t const * const K, uint8_t K_len,
        uint8_t const * const id, uint8_t id_len,
        uint8_t const * const counter, size_t counter_len)
{
    assert(K != NULL || K_len == 0);
    assert(id != NULL || id == 0);
    assert(K_len + id_len <= Xoodyak_rateKin - 1);
    assert(counter != NULL || counter_len == 0);

    instance->phase = XOODYAK_PHASE_UP;
    instance->mode = XOODYAK_MODE_HASH;
    instance->rate_absorb = Xoodyak_rateHash;
    instance->rate_squeeze = Xoodyak_rateHash;

    Xoodoo_StaticInitialize();
    Xoodoo_Initialize(instance->state);

    if (K_len != 0) {
        AbsorbKey(instance, K, K_len, id, id_len, counter, counter_len);
    }
}

void Cyclist_Absorb(
        Cyclist_Instance * const instance,
        uint8_t const * const X, size_t const X_len)
{
    assert(X != NULL || X_len == 0);

    AbsorbAny(instance, X, X_len, instance->rate_absorb, 0x03);
}

void Cyclist_Encrypt(
        Cyclist_Instance * const instance,
        uint8_t * const C,
        uint8_t const * const P, size_t const P_len)
{
    assert(P != NULL || P_len == 0);
    assert(C != NULL || P_len == 0);

    assert(instance->mode == XOODYAK_MODE_KEYED);

    Crypt(instance, C, P, P_len, false);
}

void Cyclist_Decrypt(
        Cyclist_Instance * const instance,
        uint8_t * const P,
        uint8_t const * const C, size_t const C_len)
{
    assert(C != NULL || C_len == 0);
    assert(P != NULL || C_len == 0);

    assert(instance->mode == XOODYAK_MODE_KEYED);

    Crypt(instance, P, C, C_len, true);
}

void Cyclist_Squeeze(
        Cyclist_Instance * const instance,
        uint8_t * const Y, size_t const Y_len)
{
    assert(Y != NULL || Y_len == 0);

    SqueezeAny(instance, Y, Y_len, 0x40);
}

void Cyclist_SqueezeKey(
        Cyclist_Instance * const instance,
        uint8_t * const K, size_t const K_len)
{
    assert(K != NULL || K_len == 0);

    assert(instance->mode == XOODYAK_MODE_KEYED);

    SqueezeAny(instance, K, K_len, 0x20);
}

void Cyclist_Ratched(Cyclist_Instance * const instance)
{
    assert(instance->mode == XOODYAK_MODE_KEYED);

    uint8_t temp[Xoodyak_lengthRatchet];
    SqueezeAny(instance, temp, Xoodyak_lengthRatchet, 0x10);
    AbsorbAny(instance, temp, Xoodyak_lengthRatchet, instance->rate_absorb, 0x00);
}
