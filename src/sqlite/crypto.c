/* This file is included by sqleet.c */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ROL32(x, c) (((x) << (c)) | ((x) >> (32-(c))))
#define ROR32(x, c) (((x) >> (c)) | ((x) << (32-(c))))

#define LOAD32_LE(p)                \
    ( ((uint32_t)((p)[0]) <<  0)    \
    | ((uint32_t)((p)[1]) <<  8)    \
    | ((uint32_t)((p)[2]) << 16)    \
    | ((uint32_t)((p)[3]) << 24)    \
    )
#define LOAD32_BE(p)                \
    ( ((uint32_t)((p)[3]) <<  0)    \
    | ((uint32_t)((p)[2]) <<  8)    \
    | ((uint32_t)((p)[1]) << 16)    \
    | ((uint32_t)((p)[0]) << 24)    \
    )

#define STORE32_LE(p, v)            \
    (p)[0] = ((v) >>  0) & 0xFF;    \
    (p)[1] = ((v) >>  8) & 0xFF;    \
    (p)[2] = ((v) >> 16) & 0xFF;    \
    (p)[3] = ((v) >> 24) & 0xFF;
#define STORE32_BE(p, v)            \
    (p)[3] = ((v) >>  0) & 0xFF;    \
    (p)[2] = ((v) >>  8) & 0xFF;    \
    (p)[1] = ((v) >> 16) & 0xFF;    \
    (p)[0] = ((v) >> 24) & 0xFF;
#define STORE64_BE(p, v)            \
    (p)[7] = ((v) >>  0) & 0xFF;    \
    (p)[6] = ((v) >>  8) & 0xFF;    \
    (p)[5] = ((v) >> 16) & 0xFF;    \
    (p)[4] = ((v) >> 24) & 0xFF;    \
    (p)[3] = ((v) >> 32) & 0xFF;    \
    (p)[2] = ((v) >> 40) & 0xFF;    \
    (p)[1] = ((v) >> 48) & 0xFF;    \
    (p)[0] = ((v) >> 56) & 0xFF;

/*
 * ChaCha20 stream cipher
 */
static void chacha20_block(uint32_t x[16])
{
    int i;
    #define QR(x, a, b, c, d)                           \
    x[a] += x[b]; x[d] ^= x[a]; x[d] = ROL32(x[d], 16); \
    x[c] += x[d]; x[b] ^= x[c]; x[b] = ROL32(x[b], 12); \
    x[a] += x[b]; x[d] ^= x[a]; x[d] = ROL32(x[d],  8); \
    x[c] += x[d]; x[b] ^= x[c]; x[b] = ROL32(x[b],  7);
    for (i = 0; i < 10; i++) {
        /* Column round */
        QR(x, 0, 4, 8, 12)
        QR(x, 1, 5, 9, 13)
        QR(x, 2, 6, 10, 14)
        QR(x, 3, 7, 11, 15)
        /* Diagonal round */
        QR(x, 0, 5, 10, 15)
        QR(x, 1, 6, 11, 12)
        QR(x, 2, 7, 8, 13)
        QR(x, 3, 4, 9, 14)
    }
    #undef QR
}

void chacha20_xor(void *buffer, size_t n, const uint8_t key[32],
                  const uint8_t nonce[12], uint32_t counter)
{
    int i;
    union {
        uint8_t bytes[64];
        uint32_t words[16];
    } block;
    uint32_t state[16];
    uint8_t *buf = buffer;

    state[ 0] = 0x61707865; /* 'expa' */
    state[ 1] = 0x3320646e; /* 'nd 3' */
    state[ 2] = 0x79622d32; /* '2-by' */
    state[ 3] = 0x6b206574; /* 'te k' */

    state[ 4] = LOAD32_LE(key +  0);
    state[ 5] = LOAD32_LE(key +  4);
    state[ 6] = LOAD32_LE(key +  8);
    state[ 7] = LOAD32_LE(key + 12);
    state[ 8] = LOAD32_LE(key + 16);
    state[ 9] = LOAD32_LE(key + 20);
    state[10] = LOAD32_LE(key + 24);
    state[11] = LOAD32_LE(key + 28);

    state[12] = counter;
    state[13] = LOAD32_LE(nonce + 0);
    state[14] = LOAD32_LE(nonce + 4);
    state[15] = LOAD32_LE(nonce + 8);

    while (n > 64) {
        for (i = 0; i < 16; i++)
            block.words[i] = state[i];
        chacha20_block(block.words);
        for (i = 0; i < 16; i++) {
            block.words[i] += state[i];
            block.words[i] ^= LOAD32_LE(buf);
            STORE32_LE(buf, block.words[i]);
            buf += 4;
        }
        state[12]++;
        n -= 64;
    }

    for (i = 0; i < 16; i++)
        block.words[i] = state[i];
    chacha20_block(state);
    for (i = 0; i < 16; i++) {
        state[i] += block.words[i];
        STORE32_LE(&block.bytes[4*i], state[i]);
    }
    for (i = 0; i < n; i++)
        buf[i] ^= block.bytes[i];
}

/*
 * Poly1305 authentication tags
 */
void poly1305(const uint8_t *msg, size_t n, const uint8_t key[32],
              uint8_t tag[16])
{
    uint64_t d0, d1, d2, d3, d4;
    uint32_t h0, h1, h2, h3, h4;
    uint32_t r0, r1, r2, r3, r4;
    uint32_t s1, s2, s3, s4;

    h0 = h1 = h2 = h3 = h4 = 0;
    r0 = (LOAD32_LE(key +  0) >> 0) & 0x03FFFFFF;
    r1 = (LOAD32_LE(key +  3) >> 2) & 0x03FFFF03; s1 = r1 * 5;
    r2 = (LOAD32_LE(key +  6) >> 4) & 0x03FFC0FF; s2 = r2 * 5;
    r3 = (LOAD32_LE(key +  9) >> 6) & 0x03F03FFF; s3 = r3 * 5;
    r4 = (LOAD32_LE(key + 12) >> 8) & 0x000FFFFF; s4 = r4 * 5;
    while (n >= 16) {
        h4 += 0x01000000;

process_block:
        h0 += (LOAD32_LE(msg +  0) >> 0) & 0x03FFFFFF;
        h1 += (LOAD32_LE(msg +  3) >> 2) & 0x03FFFFFF;
        h2 += (LOAD32_LE(msg +  6) >> 4) & 0x03FFFFFF;
        h3 += (LOAD32_LE(msg +  9) >> 6) & 0x03FFFFFF;
        h4 += (LOAD32_LE(msg + 12) >> 8);

        #define MUL(a,b) ((uint64_t)(a) * (b))
        d0 = MUL(h0,r0) + MUL(h1,s4) + MUL(h2,s3) + MUL(h3,s2) + MUL(h4,s1);
        d1 = MUL(h0,r1) + MUL(h1,r0) + MUL(h2,s4) + MUL(h3,s3) + MUL(h4,s2);
        d2 = MUL(h0,r2) + MUL(h1,r1) + MUL(h2,r0) + MUL(h3,s4) + MUL(h4,s3);
        d3 = MUL(h0,r3) + MUL(h1,r2) + MUL(h2,r1) + MUL(h3,r0) + MUL(h4,s4);
        d4 = MUL(h0,r4) + MUL(h1,r3) + MUL(h2,r2) + MUL(h3,r1) + MUL(h4,r0);
        #undef MUL

        h0 = d0 & 0x03FFFFFF; d1 += (d0 >> 26);
        h1 = d1 & 0x03FFFFFF; d2 += (d1 >> 26);
        h2 = d2 & 0x03FFFFFF; d3 += (d2 >> 26);
        h3 = d3 & 0x03FFFFFF; d4 += (d3 >> 26);
        h4 = d4 & 0x03FFFFFF; h0 += (uint32_t)(d4 >> 26) * 5;

        msg += 16;
        n -= 16;
    }
    if (n) {
        int i;
        for (i = 0; i < n; tag[i] = msg[i], i++);
        for (tag[i++] = 1; i < 16; tag[i++] = 0);
        msg = tag;
        n = 16;
        goto process_block;
    }

    r0 = h0 + 5;
    r1 = h1 + (r0 >> 26); *(volatile uint32_t *)&r0 = 0;
    r2 = h2 + (r1 >> 26); *(volatile uint32_t *)&r1 = 0;
    r3 = h3 + (r2 >> 26); *(volatile uint32_t *)&r2 = 0;
    r4 = h4 + (r3 >> 26); *(volatile uint32_t *)&r3 = 0;
    h0 = h0 + (r4 >> 26) * 5; *(volatile uint32_t *)&r4 = 0;

    d0 = (uint64_t)LOAD32_LE(key + 16) + (h0 >>  0) + (h1 << 26);
    d1 = (uint64_t)LOAD32_LE(key + 20) + (h1 >>  6) + (h2 << 20) + (d0 >> 32);
    d2 = (uint64_t)LOAD32_LE(key + 24) + (h2 >> 12) + (h3 << 14) + (d1 >> 32);
    d3 = (uint64_t)LOAD32_LE(key + 28) + (h3 >> 18) + (h4 <<  8) + (d2 >> 32);

    STORE32_LE(tag +  0, d0); *(volatile uint32_t *)&s1 = 0;
    STORE32_LE(tag +  4, d1); *(volatile uint32_t *)&s2 = 0;
    STORE32_LE(tag +  8, d2); *(volatile uint32_t *)&s3 = 0;
    STORE32_LE(tag + 12, d3); *(volatile uint32_t *)&s4 = 0;
    *(volatile uint64_t *)&d0 = 0; *(volatile uint32_t *)&h0 = 0;
    *(volatile uint64_t *)&d1 = 0; *(volatile uint32_t *)&h1 = 0;
    *(volatile uint64_t *)&d2 = 0; *(volatile uint32_t *)&h2 = 0;
    *(volatile uint64_t *)&d3 = 0; *(volatile uint32_t *)&h3 = 0;
    *(volatile uint64_t *)&d4 = 0; *(volatile uint32_t *)&h4 = 0;
}

int poly1305_tagcmp(const uint8_t tag1[16], const uint8_t tag2[16])
{
    uint8_t d = 0;
    d |= tag1[ 0] ^ tag2[ 0];
    d |= tag1[ 1] ^ tag2[ 1];
    d |= tag1[ 2] ^ tag2[ 2];
    d |= tag1[ 3] ^ tag2[ 3];
    d |= tag1[ 4] ^ tag2[ 4];
    d |= tag1[ 5] ^ tag2[ 5];
    d |= tag1[ 6] ^ tag2[ 6];
    d |= tag1[ 7] ^ tag2[ 7];
    d |= tag1[ 8] ^ tag2[ 8];
    d |= tag1[ 9] ^ tag2[ 9];
    d |= tag1[10] ^ tag2[10];
    d |= tag1[11] ^ tag2[11];
    d |= tag1[12] ^ tag2[12];
    d |= tag1[13] ^ tag2[13];
    d |= tag1[14] ^ tag2[14];
    d |= tag1[15] ^ tag2[15];
    return (int)d;
}

/*
 * SHA256 hash function
 */
struct sha256 {
    uint32_t state[8];
    uint8_t buffer[64];
    uint64_t n64;
    int n;
};

void sha256_init(struct sha256 *ctx)
{
    ctx->state[0] = 0x6a09e667; /* sqrt(2) */
    ctx->state[1] = 0xbb67ae85; /* sqrt(3) */
    ctx->state[2] = 0x3c6ef372; /* sqrt(5) */
    ctx->state[3] = 0xa54ff53a; /* sqrt(7) */
    ctx->state[4] = 0x510e527f; /* sqrt(11) */
    ctx->state[5] = 0x9b05688c; /* sqrt(13) */
    ctx->state[6] = 0x1f83d9ab; /* sqrt(17) */
    ctx->state[7] = 0x5be0cd19; /* sqrt(19) */
    ctx->n64 = 0;
    ctx->n = 0;
}

static void sha256_block(uint32_t state[8], const uint8_t p[64])
{
    uint32_t w[64], a, b, c, d, e, f, g, h;
    uint32_t s0, s1, S0, S1, t1, t2;
    static const uint32_t K256[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    #define ROUND_CORE(i)                                           \
    S1 = ROR32(e, 6) ^ ROR32(e, 11) ^ ROR32(e, 25);                 \
    t1 = h + S1 + ((e & f) ^ (~e & g)) + K256[i] + w[i];            \
    S0 = ROR32(a, 2) ^ ROR32(a, 13) ^ ROR32(a, 22);                 \
    t2 = S0 + ((a & b) ^ (a & c) ^ (b & c));                        \
    h = g; g = f; f = e; e = d + t1;                                \
    d = c; c = b; b = a; a = t1 + t2;

    #define ROUND_0_15(i) w[i] = LOAD32_BE(p); p += 4; ROUND_CORE(i)
    ROUND_0_15( 0) ROUND_0_15( 1) ROUND_0_15( 2) ROUND_0_15( 3)
    ROUND_0_15( 4) ROUND_0_15( 5) ROUND_0_15( 6) ROUND_0_15( 7)
    ROUND_0_15( 8) ROUND_0_15( 9) ROUND_0_15(10) ROUND_0_15(11)
    ROUND_0_15(12) ROUND_0_15(13) ROUND_0_15(14) ROUND_0_15(15)
    #undef ROUND_0_15

    #define ROUND_16_19(i)                                          \
    s0 = ROR32(w[i-15], 7) ^ ROR32(w[i-15], 18) ^ (w[i-15] >> 3);   \
    s1 = ROR32(w[i-2], 17) ^ ROR32(w[i-2],  19) ^ (w[i-2] >> 10);   \
    w[i] = w[i-16] + s0 + w[i-7] + s1; ROUND_CORE(i)
    ROUND_16_19(16) ROUND_16_19(17) ROUND_16_19(18) ROUND_16_19(19)
    ROUND_16_19(20) ROUND_16_19(21) ROUND_16_19(22) ROUND_16_19(23)
    ROUND_16_19(24) ROUND_16_19(25) ROUND_16_19(26) ROUND_16_19(27)
    ROUND_16_19(28) ROUND_16_19(29) ROUND_16_19(30) ROUND_16_19(31)
    ROUND_16_19(32) ROUND_16_19(33) ROUND_16_19(34) ROUND_16_19(35)
    ROUND_16_19(36) ROUND_16_19(37) ROUND_16_19(38) ROUND_16_19(39)
    ROUND_16_19(40) ROUND_16_19(41) ROUND_16_19(42) ROUND_16_19(43)
    ROUND_16_19(44) ROUND_16_19(45) ROUND_16_19(46) ROUND_16_19(47)
    ROUND_16_19(48) ROUND_16_19(49) ROUND_16_19(50) ROUND_16_19(51)
    ROUND_16_19(52) ROUND_16_19(53) ROUND_16_19(54) ROUND_16_19(55)
    ROUND_16_19(56) ROUND_16_19(57) ROUND_16_19(58) ROUND_16_19(59)
    ROUND_16_19(60) ROUND_16_19(61) ROUND_16_19(62) ROUND_16_19(63)
    #undef ROUND_16_19
    #undef ROUND_CORE

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void sha256_update(struct sha256 *ctx, const void *data, size_t n)
{
    const uint8_t *input = data;
    if (n < 64 || ctx->n) {
        int i, j = (ctx->n + n < 64) ? n : 64 - ctx->n;
        for (i = 0; i < j; i++)
            ctx->buffer[ctx->n + i] = input[i];
        if ((ctx->n += j) < 64)
            return;
        sha256_block(ctx->state, ctx->buffer);
        ctx->n64 += 64;
        ctx->n = 0;
        input += j;
        n -= j;
    }

    while (n >= 64) {
        sha256_block(ctx->state, input);
        ctx->n64 += 64;
        input += 64;
        n -= 64;
    }

    if (n) {
        int i = 0;
        while (i < n) {
            ctx->buffer[i] = input[i];
            i++;
        }
        ctx->n = n;
    }
}

static void sha256_serialize(const uint32_t state[8], uint8_t hash[32])
{
    STORE32_BE(hash +  0, state[0]);
    STORE32_BE(hash +  4, state[1]);
    STORE32_BE(hash +  8, state[2]);
    STORE32_BE(hash + 12, state[3]);
    STORE32_BE(hash + 16, state[4]);
    STORE32_BE(hash + 20, state[5]);
    STORE32_BE(hash + 24, state[6]);
    STORE32_BE(hash + 28, state[7]);
}

void sha256_final(struct sha256 *ctx, uint8_t hash[32])
{
    int i;
    uint8_t buf[128];
    const uint64_t nbits = (ctx->n64 + ctx->n) * 8;
    for (i = 1, buf[0] = 0x80; (ctx->n + i + 8) % 64; buf[i++] = 0);
    STORE64_BE(buf + i, nbits);
    sha256_update(ctx, buf, i + 8);
    sha256_serialize(ctx->state, hash);
}

/*
 * PBKDF2-HMAC-SHA256 key derivation optimized to reuse intermediate SHA256
 * states computed in the HMAC-SHA256 calculation of the inner and outer pad.
 */
void pbkdf2_hmac_sha256(const void *pass, size_t m, const void *salt, size_t n,
                        size_t iter, uint8_t *dk, size_t dklen)
{
    size_t i, j;
    uint32_t I[8], O[8];
    struct sha256 ctx, ictx, octx;
    uint8_t keyblock[64], iblock[64], oblock[64];

    /* Initialize keyblock */
    if (m > 64) {
        sha256_init(&ctx);
        sha256_update(&ctx, pass, m);
        sha256_final(&ctx, keyblock);
        memset(keyblock + 32, 0, 32);
    } else {
        memcpy(keyblock, pass, m);
        memset(keyblock + m, 0, 64 - m);
    }

    /* Prepare iblock and oblock */
    sha256_init(&ictx);
    sha256_init(&octx);
    for (i = 0; i < 64; i++) {
        iblock[i] = 0x36 ^ keyblock[i];
        oblock[i] = 0x5C ^ keyblock[i];
        *(volatile uint8_t *)(keyblock + i) = 0;
    }
    sha256_update(&ictx, iblock, 64);
    sha256_update(&octx, oblock, 64);
    memset(iblock+32, 0, 32);
    memset(oblock+32, 0, 32);
    STORE32_BE(&iblock[64-4], 96*8);
    STORE32_BE(&oblock[64-4], 96*8);
    iblock[32] = oblock[32] = 0x80;

    /* PBKDF2 main loop */
    for (i = 1; dklen; i++) {
        int k;
        uint8_t ibuf[4];
        const int len = (dklen < 32) ? (int)dklen : 32;
        STORE32_BE(ibuf, i);
        memcpy(&ctx, &ictx, sizeof(struct sha256));
        sha256_update(&ctx, salt, n);
        sha256_update(&ctx, ibuf, 4);
        sha256_final(&ctx, oblock);

        memcpy(O, octx.state, 32);
        sha256_block(O, oblock);
        sha256_serialize(O, iblock);

        memcpy(dk, iblock, len);
        for (j = 1; j < iter; j++) {
            memcpy(I, ictx.state, 32);
            memcpy(O, octx.state, 32);
            sha256_block(I, iblock);
            sha256_serialize(I, oblock);
            sha256_block(O, oblock);
            sha256_serialize(O, iblock);
            for (k = 0; k < len; k++)
                dk[k] ^= iblock[k];
        }
        dklen -= len;
        dk += len;
    }

    /* Burn key material */    /* TODO: is this really necessary? */
    for (i = 0; i < 64; i++) { /* for truly paranoid people, yes */
        *(volatile uint8_t *)(iblock + i) = 0;
        *(volatile uint8_t *)(oblock + i) = 0;
    }
}

/*
 * Platform-specific entropy functions for seeding RNG
 */
#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#define RtlGenRandom SystemFunction036
BOOLEAN NTAPI RtlGenRandom(PVOID RandomBuffer, ULONG RandomBufferLength);
#pragma comment(lib, "advapi32.lib")
static size_t entropy(void *buf, size_t n)
{
    return RtlGenRandom(buf, n) ? n : 0;
}
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/ioctl.h>
/* musl does not have <linux/random.h> so let's define RNDGETENTCNT here */
#ifndef RNDGETENTCNT
#define RNDGETENTCNT _IOR('R', 0x00, int)
#endif
#endif

/* Returns the number of urandom bytes read (either 0 or n) */
static size_t read_urandom(void *buf, size_t n)
{
    size_t i;
    ssize_t ret;
    int fd, count;
    struct stat st;
    int errnold = errno;

    do {
        fd = open("/dev/urandom", O_RDONLY, 0);
    } while (fd == -1 && errno == EINTR);
    if (fd == -1)
        goto fail;
    fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);

    /* Check the sanity of the device node */
    if (fstat(fd, &st) == -1 || !S_ISCHR(st.st_mode)
            #ifdef __linux__
            || ioctl(fd, RNDGETENTCNT, &count) == -1
            #endif
            ) {
        close(fd);
        goto fail;
    }

    /* Read bytes */
    for (i = 0; i < n; i += ret) {
        while ((ret = read(fd, (uint8_t *)buf + i, n - i)) == -1) {
            if (errno != EAGAIN && errno != EINTR) {
                close(fd);
                goto fail;
            }
        }
    }
    close(fd);

    /* Verify that the random device returned non-zero data */
    for (i = 0; i < n; i++) {
        if (((uint8_t *)buf)[i] != 0) {
            errno = errnold;
            return n;
        }
    }

    /* Tiny n may unintentionally fall through! */

fail:
    fprintf(stderr, "bad /dev/urandom RNG\n");
    abort(); /* PANIC! */
    return 0;
}

static size_t entropy(void *buf, size_t n)
{
    #if defined(__linux__) && defined(SYS_getrandom)
    if (syscall(SYS_getrandom, buf, n, 0) == n)
        return n;
    #elif defined(SYS_getentropy)
    if (syscall(SYS_getentropy, buf, n) == 0)
        return n;
    #endif
    return read_urandom(buf, n);
}
#else
#error "Secure pseudorandom number generator unimplemented for this OS"
#endif

/*
 * ChaCha20 random number generator
 */
void chacha20_rng(void *out, size_t n)
{
    static uint8_t key[32], nonce[12], buffer[64] = {0};
    static uint32_t counter = 0;
    static size_t available = 0;

#if SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG);
    sqlite3_mutex_enter(mutex);
#endif

    while (n > 0) {
        size_t m;
        if (available == 0) {
            if (counter == 0) {
                if (entropy(key, sizeof(key)) != sizeof(key))
                    abort();
                if (entropy(nonce, sizeof(nonce)) != sizeof(nonce))
                    abort();
            }
            chacha20_xor(buffer, sizeof(buffer), key, nonce, counter++);
            available = sizeof(buffer);
        }
        m = (available < n) ? available : n;
        memcpy(out, buffer + (sizeof(buffer) - available), m);
        out = (uint8_t *)out + m;
        available -= m;
        n -= m;
    }

#if SQLITE_THREADSAFE
    sqlite3_mutex_leave(mutex);
#endif
}
