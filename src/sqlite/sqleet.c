#if (defined(__linux__) || defined(__unix__)) && !defined(_GNU_SOURCE)
# define _GNU_SOURCE
#endif

#define SQLITE3_H_OMIT
#include "sqleet.h"
#include "sqlite3.c"
#include "rekeyvacuum.c"
#include "crypto.c"

/*
 * SQLite3 codec implementation.
 */
typedef struct codec {
    struct codec *reader, *writer;
    uint8_t key[32], salt[16], header[16];
    enum {
        SQLEET_HAS_KEY      = 0x01,
        SQLEET_HAS_SALT     = 0x02,
        SQLEET_HAS_HEADER   = 0x04,
        SQLEET_HAS_PAGESIZE = 0x08,
        SQLEET_HAS_SKIP     = 0x10,
        SQLEET_HAS_KDF      = 0x20
    } flags;
    void *pagebuf;
    int pagesize;
    int skip;
    enum {
        SQLEET_KDF_NONE = 0,
        SQLEET_KDF_PBKDF2_HMAC_SHA256
    } kdf;
    Btree *btree;
    int error;
    const void *zKey;
    int nKey;
} Codec;

Codec *codec_new(const void *zKey, int nKey, Codec *from)
{
    Codec *codec;
    if ((codec = sqlite3_malloc(sizeof(Codec)))) {
        if (from) {
            memcpy(codec, from, sizeof(Codec));
            if (codec->reader == from)
                codec->reader = codec;
            if (codec->writer == from)
                codec->writer = codec;
            codec->pagebuf = NULL;
            codec->btree = NULL;
        } else {
            memset(codec, 0, sizeof(Codec));
            codec->reader = codec->writer = codec;
        }
        codec->zKey = zKey;
        codec->nKey = nKey;
    }
    return codec;
}

void codec_free(void *pcodec)
{
    if (pcodec) {
        int i;
        volatile uint8_t *p;
        Codec *codec = pcodec;
        if ((p = codec->pagebuf)) {
            for (i = 0; i < codec->pagesize; p[i++] = '\0');
            sqlite3_free(codec->pagebuf);
        }
        for (i = 0, p = pcodec; i < sizeof(Codec); p[i++] = '\0');
        sqlite3_free(codec);
    }
}

static int codec_uri_parameter(const char *zUri, const char *parameter,
                               size_t len_min, size_t len_max,
                               uint8_t *out)
{
    int rc;
    char *p, buffer[256];
    const char *val, *hex;
    size_t size = strlen(parameter) + 1;

    /* Get URI query parameter value (with and without 'hex' prefix) */
    if ((p = (3 + size < sizeof(buffer)) ? buffer : sqlite3_malloc(3 + size))) {
        p[0] = 'h'; p[1] = 'e'; p[2] = 'x';
        memcpy(p + 3, parameter, size);
        hex = sqlite3_uri_parameter(zUri, p);
        if (p != buffer)
            sqlite3_free(p);
    } else {
        return SQLITE_NOMEM;
    }
    val = sqlite3_uri_parameter(zUri, parameter);

    /* Parse parameter value of length len_min..len_max bytes */
    if ((val && hex) || len_min > len_max) {
        rc = SQLITE_MISUSE;
    } else if (val) {
        /* Copy a non-hex value */
        size_t len = strlen(val);
        if (len_min <= len && len <= len_max) {
            if (len)
                memcpy(out, val, len);
            if (len < len_max)
                memset(&out[len], 0, len_max - len);
            rc = SQLITE_OK;
        } else {
            rc = len ? SQLITE_MISUSE : SQLITE_EMPTY;
        }
    } else if (hex) {
        /* Decode a hex-encoded value */
        size_t i;
        for (i = 0; i < 2*len_max && hex[i]; i++) {
            char c = hex[i];
            if (c >= '0' && c <= '9') {
                c = c - '0';
            } else if (c >= 'A' && c <= 'F') {
                c = c - 'A' + 10;
            } else if (c >= 'a' && c <= 'f') {
                c = c - 'a' + 10;
            } else {
                break;
            }
            out[i/2] = (out[i/2] << 4) | c;
        }
        if (2*len_min <= i && i <= 2*len_max) {
            if  (i & 1) {
                out[i/2] <<= 4;
                i++;
            }
            for (i = i/2; i < len_max; out[i++] = '\0');
            rc = SQLITE_OK;
        } else {
            rc = i ? SQLITE_MISUSE : SQLITE_EMPTY;
        }
    } else {
        /* Parameter missing */
        rc = SQLITE_NOTFOUND;
    }

    return rc;
}

int codec_parse_uri_config(Codec *codec, const char *zUri)
{
    int rc, pagesize;
    const char *value;
    if (zUri && !*zUri)
        zUri = NULL;

    /* Override page_size PRAGMA */
    pagesize = (int)sqlite3_uri_int64(zUri, "page_size", -1);
    if (pagesize > 0) {
        if (pagesize != codec->pagesize) {
            if (pagesize < 512 || pagesize > 65536 || (pagesize & (pagesize-1)))
                return SQLITE_MISUSE;
            codec->pagesize = pagesize;
        }
        codec->flags |= SQLEET_HAS_PAGESIZE;
    } else if (pagesize == 0) {
        codec->pagesize = pagesize;
        codec->flags &= ~SQLEET_HAS_PAGESIZE;
    }

    /* Override SKIP_HEADER_BYTES */
    if ((value = sqlite3_uri_parameter(zUri, "skip")) && !*value) {
        codec->flags &= ~SQLEET_HAS_SKIP;
        value = NULL;
    }
    if (value && (rc = (int)sqlite3_uri_int64(zUri, "skip", -1)) >= 0) {
        codec->skip = rc;
        codec->flags |= SQLEET_HAS_SKIP;
    } else if (!(codec->flags & SQLEET_HAS_SKIP)) {
        codec->skip = SKIP_HEADER_BYTES;
    }
    if (codec->skip < 0 || (pagesize > 0 && codec->skip > pagesize))
        return SQLITE_MISUSE;

    /* Override key derivation function (KDF) */
    if ((value = sqlite3_uri_parameter(zUri, "kdf")) && !*value) {
        codec->flags &= ~SQLEET_HAS_KDF;
        value = NULL;
    }
    if (!value) {
        if (!(codec->flags & SQLEET_HAS_KDF))
            codec->kdf = SQLEET_KDF_PBKDF2_HMAC_SHA256;
    } else if (!strcmp(value, "none") && codec->nKey == sizeof(codec->key)) {
        codec->kdf = SQLEET_KDF_NONE;
        codec->flags |= SQLEET_HAS_KDF;
    } else {
        return SQLITE_MISUSE;
    }

    /* KDF salt */
    rc = codec_uri_parameter(zUri, "salt", 1, sizeof(codec->salt), codec->salt);
    if (rc == SQLITE_OK) {
        if (!codec->zKey)
            return SQLITE_MISUSE;
        codec->flags |= SQLEET_HAS_SALT;
    } else if (rc == SQLITE_EMPTY) {
        chacha20_rng(codec->salt, sizeof(codec->salt));
        codec->flags &= ~SQLEET_HAS_SALT;
    } else if (rc == SQLITE_NOTFOUND) {
        if (!(codec->flags & (SQLEET_HAS_KEY|SQLEET_HAS_SALT)))
            chacha20_rng(codec->salt, sizeof(codec->salt));
    } else {
        return rc;
    }
    if (codec->kdf == SQLEET_KDF_NONE && (codec->flags & SQLEET_HAS_SALT))
        return SQLITE_MISUSE;

    /* Header */
    rc = codec_uri_parameter(zUri, "header", 1, sizeof(codec->header),
                             codec->header);
    if (rc == SQLITE_OK) {
        /* Header requires explicit salt when KDF is enabled */
        if (codec->kdf == SQLEET_KDF_NONE || (codec->flags & SQLEET_HAS_SALT)) {
            codec->flags |= SQLEET_HAS_HEADER;
        } else {
            rc = SQLITE_MISUSE;
        }
    } else if (rc == SQLITE_NOTFOUND || rc == SQLITE_EMPTY) {
        if (rc == SQLITE_EMPTY)
            codec->flags &= ~SQLEET_HAS_HEADER;
        if (!(codec->flags & SQLEET_HAS_HEADER)) {
            if (codec->kdf == SQLEET_KDF_NONE) {
                /* If KDF disabled, skip SQLite3 magic header */
                int skip;
                if ((skip = codec->skip) > sizeof(codec->header))
                    skip = sizeof(codec->header);
                memcpy(codec->header, "SQLite format 3", skip);
                chacha20_rng(&codec->header[skip], sizeof(codec->header)-skip);
            } else {
                /* By default, store the KDF salt as the header  */
                memcpy(codec->header, codec->salt, sizeof(codec->header));
            }
        }
        rc = SQLITE_OK;
    }

    return rc;
}

#ifndef PBKDF2_HMAC_SHA256_ITERATIONS
#define PBKDF2_HMAC_SHA256_ITERATIONS 12345
#endif
void codec_kdf(Codec *codec)
{
    if (codec->kdf == SQLEET_KDF_PBKDF2_HMAC_SHA256) {
        pbkdf2_hmac_sha256(codec->zKey, codec->nKey,
                           codec->salt, sizeof(codec->salt),
                           PBKDF2_HMAC_SHA256_ITERATIONS,
                           codec->key, sizeof(codec->key));
    } else /*if (codec->kdf == SQLEET_KDF_NONE)*/ {
        memcpy(codec->key, codec->zKey, sizeof(codec->key));
    }

    codec->flags |= SQLEET_HAS_KEY;
    codec->zKey = NULL;
    codec->nKey = 0;
}

/*
 * The encrypted database page format.
 *
 * +----------------------------------------+----------------+----------------+
 * | Encrypted data                         | 16-byte nonce  | 16-byte tag    |
 * +----------------------------------------+----------------+----------------+
 *
 * As the only exception, the first page (page_no=1) starts with a plaintext
 * salt contained in the first 16 bytes of the database file. The "master" key
 * is derived from a user-given password with the salt and 12345 iterations of
 * PBKDF-HMAC-SHA256. Future plans include switching to BLAKE2 and Argon2.
 *
 * - The data is encrypted by XORing with the ChaCha20 keystream produced from
 *   the 16-byte nonce and a 32-byte encryption key derived from the master key.
 *   - OK, I lied a little: ChaCha20 uses only the first 12 bytes as the nonce.
 *     However, ChaCha20 also requires an initial value for a counter of 4 bytes
 *     that encodes a block position in the output stream. We derive the counter
 *     value from the last 4 bytes, effectively extending the nonce to 16 bytes.
 *   - Specifically, counter = LOAD32_LE(nonce[12..15])^page_no is first applied
 *     to generate a single 64-byte block from nonce[0..11] and the master key.
 *     The block consists of two 32-byte one-time keys, the former is a Poly1305
 *     key for the authentication tag, and the latter is a ChaCha20 key for the
 *     data encryption. The encryption with the one-time key uses nonce[0..11]
 *     and the initial counter value of counter+1.
 *   - The XOR with page_no prevents malicious reordering of the pages.
 *
 * - The nonce consists of 128 randomly generated bits, which should be enough
 *   to guarantee uniqueness with a secure pseudorandom number generator.
 *   - Given a secure PRNG, the adversary needs to observe at least 2^61 nonces
 *     to break Poly1305 with the birthday attack at a success rate of 1%.
 *   - If a nonce is reused, we lose confidentiality of the associated messages.
 *     Moreover, the compromised nonce can also be used to forge valid tags for
 *     new messages having the same nonce (basically, the one-time Poly1305 key
 *     can be recovered from distinct messages with identical nonces).
 *
 * - The tag is a Poly1305 MAC calculated over the encrypted data and the nonce
 *   with the one-time key generated from the master key and the nonce.
 */
#define PAGE_NONCE_LEN 16
#define PAGE_TAG_LEN 16
#define PAGE_RESERVED_LEN (PAGE_NONCE_LEN + PAGE_TAG_LEN)
void *codec_handle(void *codec, void *pdata, Pgno page, int mode)
{
    uint32_t counter;
    uint8_t otk[64], tag[16], *data = pdata;
    Codec *reader = ((Codec *)codec)->reader;
    Codec *writer = ((Codec *)codec)->writer;

    switch (mode) {
    case 0: /* Journal decryption */
    case 2: /* Reload a page */
    case 3: /* Load a page */
        if (reader) {
            const int n = reader->pagesize - PAGE_RESERVED_LEN;
            const int skip = (page == 1) ? reader->skip : 0;
            if (page == 1 && !(reader->flags & SQLEET_HAS_KEY)) {
                if (!(reader->flags & SQLEET_HAS_SALT))
                    memcpy(reader->salt, data, sizeof(reader->salt));
                if (!(reader->flags & SQLEET_HAS_HEADER))
                    memcpy(reader->header, reader->salt, sizeof(reader->salt));
                codec_kdf(reader);
            }

            /* Generate one-time keys */
            memset(otk, 0, 64);
            counter = LOAD32_LE(data + n + PAGE_NONCE_LEN-4) ^ page;
            chacha20_xor(otk, 64, reader->key, data + n, counter);

            /* Decrypt and authenticate */
            poly1305(data, n + PAGE_NONCE_LEN, otk, tag);
            chacha20_xor(data + skip, n - skip, otk+32, data + n, counter+1);
            if (poly1305_tagcmp(data + n + PAGE_NONCE_LEN, tag) != 0) {
                reader->error = SQLITE_AUTH;
                return NULL;
            }
            if (page == 1) memcpy(data, "SQLite format 3", 16);
        }
        break;

    case 7: /* Encrypt a journal page (with the reader key) */
        writer = reader;
        /* fall-through */
    case 6: /* Encrypt a main database page */
        if (writer) {
            const int n = writer->pagesize - PAGE_RESERVED_LEN;
            const int skip = (page == 1) ? writer->skip : 0;
            data = memcpy(writer->pagebuf, data, writer->pagesize);

            /* Generate one-time keys */
            memset(otk, 0, 64);
            chacha20_rng(data + n, PAGE_NONCE_LEN);
            counter = LOAD32_LE(data + n + PAGE_NONCE_LEN-4) ^ page;
            chacha20_xor(otk, 64, writer->key, data + n, counter);

            /* Encrypt and authenticate */
            chacha20_xor(data + skip, n - skip, otk+32, data + n, counter+1);
            if (page == 1) memcpy(data, writer->header, sizeof(writer->header));
            poly1305(data, n + PAGE_NONCE_LEN, otk, data + n + PAGE_NONCE_LEN);
        }
        break;
    }

    return data;
}

/*
 * A hack to control the page size of attached vacuum database.
 * Otherwise the database inherits page size from the source database.
 */
static void size_hook(void *pcodec, int new_pagesize, int reserved)
{
    Codec *codec = (Codec *)pcodec;
    if (codec && codec->pagesize != new_pagesize) {
        u32 pagesize = codec->pagesize;
        const int expected = (codec->writer ? PAGE_RESERVED_LEN : 0);
        if (expected <= reserved) {
            Pager *pager = sqlite3BtreePager(codec->btree);
            if (sqlite3PagerSetPagesize(pager, &pagesize, -1) == SQLITE_OK) {
                if (pagesize == codec->pagesize)
                    codec->btree->pBt->pageSize = pagesize;
            }
        } else {
            codec->reader = codec->writer = NULL;
            codec->error = SQLITE_MISUSE;
        }
    }
}

/*
 * Set (or unset) a codec for a Btree pager.
 * The function consumes the passed in codec.
 */
static int codec_set_to(Codec *codec, Btree *pBt)
{
    Pager *pager;
    int rc, count;
    sqlite3BtreeEnter(pBt);
    pager = sqlite3BtreePager(pBt);

    /* Prepare codec */
    if (codec) {
        if (codec->pagebuf)
            sqlite3_free(codec->pagebuf);
        if (!codec->pagesize)
            codec->pagesize = sqlite3BtreeGetPageSize(pBt);
        if (!(codec->pagebuf = sqlite3_malloc(codec->pagesize))) {
            rc = SQLITE_NOMEM;
            goto kill_codec;
        }
        codec->btree = pBt;
        codec->error = SQLITE_OK;
    }

    /* Acquire shared pager lock (may block due to concurrent writes) */
    while ((rc = sqlite3PagerSharedLock(pager)) != SQLITE_OK) {
        if ((rc & 0xFF) != SQLITE_BUSY || !btreeInvokeBusyHandler(pBt->pBt))
            goto kill_codec;
    }

    /* Set (or unset) pager codec */
    if (codec) {
        const int reserved = codec->writer ? PAGE_RESERVED_LEN : 0;
        sqlite3BtreeSetPageSize(pBt, codec->pagesize, reserved, 0);
        sqlite3BtreeSecureDelete(pBt, 1);
        sqlite3PagerSetCodec(pager, codec_handle, size_hook, codec_free, codec);
    } else {
        sqlite3PagerSetCodec(pager, NULL, NULL, NULL, NULL);
    }

    /* Verify codec */
    sqlite3PagerPagecount(pager, &count);
    if (count > 0) {
        /* Non-empty database, read page 1 with the codec */
        DbPage *page;
        sqlite3PcacheClear(pager->pPCache);
        rc = sqlite3PagerGet(pager, 1, &page, PAGER_GET_READONLY);
        if (rc == SQLITE_OK) {
            rc = SQLITE_NOTADB;
            if (!memcmp(page->pData, "SQLite format 3", 16)) {
                const uint8_t *data = page->pData;
                const uint16_t pagesize = (data[16] << 8) | data[17];
                if (pagesize >= 512 && !(pagesize & (pagesize-1))) {
                    if (data[21] == 64 && data[22] == 32 && data[23] == 32) {
                        uint32_t version = data[96];
                        version = (version << 8) | data[97];
                        version = (version << 8) | data[98];
                        version = (version << 8) | data[99];
                        if (3000000 <= version && version < 4000000)
                            rc = SQLITE_OK;
                    }
                }
            }
            sqlite3PagerUnrefPageOne(page);
        } else if (codec)  {
            /* Invalid codec */
            if (codec->error != SQLITE_OK)
                rc = codec->error;
            sqlite3PagerSetCodec(pager, NULL, NULL, NULL, NULL);
        }
    } else {
        /* Empty database, assume the codec is valid */
        if (codec && !(codec->flags & SQLEET_HAS_KEY)) {
            /* Derive a new encryption key */
            codec_kdf(codec);
        }
        rc = SQLITE_OK;
    }

    pagerUnlockIfUnused(pager);
    sqlite3BtreeLeave(pBt);
    return rc;

kill_codec:
    codec_free(codec);
    sqlite3BtreeLeave(pBt);
    return rc;
}

void sqlite3CodecGetKey(sqlite3 *db, int nDb, void **zKey, int *nKey)
{
    /*
     * This function retrieves the encryption key of database `nDb` in order to
     * use the same encryption scheme for a new database attached without a key.
     * *nKey != 0 indicates that the existing encryption is available for reuse.
     */
    Codec *codec = sqlite3PagerGetCodec(sqlite3BtreePager(db->aDb[nDb].pBt));
    *zKey = NULL;
    *nKey = codec ? (nDb ? nDb : -1) : 0;
}

int sqlite3CodecAttach(sqlite3 *db, int nDb, const void *zKey, int nKey)
{
    int rc;
    Codec *codec;
    Btree *pBt = db->aDb[nDb].pBt;

    rc = SQLITE_NOMEM;
    sqlite3_mutex_enter(db->mutex);
    if (!nKey) {
        /* Attach with an empty key (no encryption) */
        rc = codec_set_to(NULL, pBt);
    } else if (zKey) {
        /* Attach with the provided key */
        if ((codec = codec_new(zKey, nKey, NULL))) {
            const char *zUri = sqlite3BtreeGetFilename(pBt);
            codec->pagesize = db->nextPagesize;
            if ((rc = codec_parse_uri_config(codec, zUri)) == SQLITE_OK) {
                if (codec->flags & SQLEET_HAS_SALT)
                    codec_kdf(codec);
                rc = codec_set_to(codec, pBt);
            } else {
                codec_free(codec);
            }
        } else {
            rc = SQLITE_NOMEM;
        }
    } else if (nDb != 0) {
        /* Use an existing codec (no key given) */
        Codec *dup;
        int mDb = (nKey < 0) ? 0 : nKey;
        codec = sqlite3PagerGetCodec(sqlite3BtreePager(db->aDb[mDb].pBt));
        if (codec && (dup = codec_new(NULL, 0, codec))) {
            const char *zUri = sqlite3BtreeGetFilename(pBt);
            if (!(dup->flags & SQLEET_HAS_PAGESIZE) && db->nextPagesize)
                dup->pagesize = db->nextPagesize;
            if ((rc = codec_parse_uri_config(dup, zUri)) == SQLITE_OK) {
                rc = codec_set_to(dup, pBt);
            } else {
                codec_free(dup);
            }
        } else {
            rc = SQLITE_NOMEM;
        }
    }
    sqlite3_mutex_leave(db->mutex);
    return rc;
}

/* Returns the main database if there is no match */
static int db_index_of(sqlite3 *db, const char *name)
{
    if (name) {
        int i = 0;
        while (i < db->nDb) {
            if (!strcmp(db->aDb[i].zDbSName, name))
                return i;
            i++;
        }
    }
    return 0;
}

int sqlite3_key_v2(sqlite3 *db, const char *zDbName, const void *zKey, int nKey)
{
    return sqlite3CodecAttach(db, db_index_of(db, zDbName), zKey, nKey);
}

int sqlite3_key(sqlite3 *db, const void *zKey, int nKey)
{
    return sqlite3_key_v2(db, "main", zKey, nKey);
}

int sqlite3_rekey_v2(sqlite3 *db, const char *zDbName,
                     const void *zKey, int nKey)
{
    int nDb, rc;
    Btree *pBt;
    Pager *pager;
    Codec *reader, *codec;
    Pgno pgno;

    sqlite3_mutex_enter(db->mutex);
    if (!(pBt = db->aDb[(nDb = db_index_of(db, zDbName))].pBt)) {
        rc = SQLITE_INTERNAL;
        goto leave;
    }

    pager = sqlite3BtreePager(pBt);
    reader = sqlite3PagerGetCodec(pager);
    if (!nKey) {
        /* Decrypt */
        if (reader) {
            char *err = NULL;
            reader->writer = NULL;
            rc = sqlite3RekeyVacuum(&err, db, nDb, NULL, 0);
            if (rc == SQLITE_OK) {
                rc = codec_set_to(NULL, pBt);
            } else {
                reader->writer = reader->reader;
            }
        } else {
            rc = codec_set_to(NULL, pBt);
        }
        goto leave;
    }

    /* Create a codec for the new key */
    if ((codec = codec_new(zKey, nKey, reader))) {
        const char *zUri = sqlite3BtreeGetFilename(pBt);
        codec->flags &= ~SQLEET_HAS_KEY;
        if (!(codec->flags & SQLEET_HAS_PAGESIZE) && db->nextPagesize)
            codec->pagesize = db->nextPagesize;
        if ((rc = codec_parse_uri_config(codec, zUri)) != SQLITE_OK) {
            codec_free(codec);
            goto leave;
        }
        codec_kdf(codec);
    } else {
        rc = SQLITE_NOMEM;
        goto leave;
    }

    if (!reader) {
        /* Encrypt */
        codec->reader = NULL;
        if ((rc = codec_set_to(codec, pBt)) == SQLITE_OK) {
            char *err = NULL;
            rc = sqlite3RekeyVacuum(&err, db, nDb, NULL, PAGE_RESERVED_LEN);
            if (rc == SQLITE_OK) {
                codec->reader = codec->writer;
            } else {
                sqlite3PagerSetCodec(pager, NULL, NULL, NULL, NULL);
            }
        }
        goto leave;
    }

    /* Change key (re-encrypt) */
    reader->writer = codec;
    codec->pagebuf = reader->pagebuf;
    rc = sqlite3BtreeBeginTrans(pBt, 1, NULL);
    for (pgno = 1; rc == SQLITE_OK && pgno <= pager->dbSize; pgno++) {
        DbPage *page;
        if (pgno == PENDING_BYTE_PAGE(pager))
            continue; /* page occupied by the PENDING_BYTE is never used */
        if ((rc = sqlite3PagerGet(pager, pgno, &page, 0)) == SQLITE_OK) {
            rc = sqlite3PagerWrite(page);
            sqlite3PagerUnref(page);
        }
    }
    if (rc == SQLITE_OK && (rc = sqlite3BtreeCommit(pBt)) == SQLITE_OK) {
        reader->pagebuf = NULL;
        rc = codec_set_to(codec, pBt);
    } else {
        reader->writer = reader;
        sqlite3BtreeRollback(pBt, SQLITE_ABORT_ROLLBACK, 0);
    }

leave:
    sqlite3_mutex_leave(db->mutex);
    return rc;
}

int sqlite3_rekey(sqlite3 *db, const void *zKey, int nKey)
{
    return sqlite3_rekey_v2(db, "main", zKey, nKey);
}

void sqlite3_activate_see(const char *info)
{
}
