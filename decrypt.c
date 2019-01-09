#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <gcrypt.h>
#include <fcntl.h>
// #include <sys/stat.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

typedef enum
{
    NKS_ENT_UNKNOWN,
    NKS_ENT_DIRECTORY,
    NKS_ENT_FILE,
} NksEntryType;

struct NksEntry
{
    char *name;
    NksEntryType type;
    off_t offset;
};

typedef struct NksEntry NksEntry;
typedef struct Nks Nks;

typedef struct
{
    uint32_t set_id;
    uint8_t data[0x10000];
} NksSetKey;

struct Nks
{
    int fd;
    NksEntry root_entry;
};

static uint8_t *nks_0110_base_key;

uint32_t
rand_ms(uint32_t *seedp)
{
    *seedp = *seedp * UINT32_C(0x343fd) + UINT32_C(0x269ec3);
    return (*seedp >> 16);
}

static void
increment_counter(uint8_t *num, size_t len)
{
    size_t n;

    for (n = len - 1; n > 0; n--)
    {
        if (++num[n] != 0)
            break;
    }
}

static void
generate_0110_base_key(void)
{
    int n;
    uint32_t seed;

    if (nks_0110_base_key != NULL)
        return;

    nks_0110_base_key = (uint8_t *)malloc(0x10000);
    seed = UINT32_C(0x608da0a2);

    for (n = 0; n < 0x10000; n++)
        nks_0110_base_key[n] = rand_ms(&seed) & 0xff;
}

static int
initialise_gcrypt(void)
{
    if (gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P))
        return 0;

    if (!gcry_check_version(GCRYPT_VERSION))
    {
        fprintf(stderr, "Error: Incompatible gcrypt version.\n");
        return -1;
    }

    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

    return 0;
}

int nks_create_0110_key(uint8_t key[], size_t key_len, uint8_t iv[], size_t iv_len, uint8_t *buffer, size_t len)
{
    gcry_cipher_hd_t cipher;
    uint8_t ctr[16];
    uint8_t *bkp;
    uint8_t *bp;
    size_t n, m;
    int algo;
    int r;

    if (key == NULL)
        1;

    if (key_len != 16 && key_len != 24 && key_len != 32)
        1;

    if (iv == NULL || iv_len != 16)
        1;

    if (buffer == NULL || len < 16 || (len & 15) != 0)
        1;

    r = initialise_gcrypt();
    if (r != 0)
        return r;

    if (nks_0110_base_key == NULL)
        generate_0110_base_key();

    switch (key_len)
    {
    case 16:
        algo = GCRY_CIPHER_AES128;
        break;
    case 24:
        algo = GCRY_CIPHER_AES192;
        break;
    case 32:
        algo = GCRY_CIPHER_AES256;
        break;
    default:
        abort();
        break;
    }

    if (gcry_cipher_open(&cipher, algo, GCRY_CIPHER_MODE_ECB, 0) != 0)
        return -1;

    if (gcry_cipher_setkey(cipher, key, key_len) != 0)
    {
        r = 1;
        goto err;
    }

    memcpy(ctr, iv, 16);

    bp = buffer;
    bkp = nks_0110_base_key;

    for (n = 0; 16 * n < len; n++)
    {
        if (gcry_cipher_encrypt(cipher, bp, 16, ctr, 16) != 0)
        {
            r = 1;
            goto err;
        }

        for (m = 0; m < 16; m++)
            bp[m] ^= bkp[m];

        increment_counter(ctr, 16);

        bp += 16;
        bkp += 16;
    }

    gcry_cipher_close(cipher);

    return 0;

err:
    gcry_cipher_close(cipher);
    return r;
}

static int
extract_encrypted_file_entry_to_fd(Nks *nks, int out_fd)
{
    char buffer[16384];
    const uint8_t *key;
    size_t count;
    size_t size;
    size_t to_read;
    size_t key_length;
    off_t key_pos;
    size_t x;
    int r;

    NksSetKey *set_key = NULL;
    if (set_key == NULL)
    {
        set_key = (NksSetKey *)malloc(sizeof(*set_key));
        set_key->set_id = 709;

        uint8_t key[] = {0xAB, 0x90, 0x62, 0x62, 0x5F, 0x0C, 0x27, 0x75, 0x2B, 0x5C, 0x2A, 0xC8, 0x19, 0x1A, 0xB3, 0x1E, 0xDA, 0x72, 0x07, 0x42, 0xCB, 0x0B, 0x22, 0xF5, 0xB4, 0x5E, 0xB8, 0x96, 0xB9, 0x9C, 0x0B, 0xD2};
        size_t key_len = sizeof(key);

        uint8_t iv[] = {0xA7, 0xFA, 0xC4, 0x1D, 0x35, 0x21, 0x49, 0x59, 0x8D, 0x91, 0xE6, 0x0D, 0xAE, 0xF9, 0x99, 0xDE};
        size_t iv_len = sizeof(iv);

        r = nks_create_0110_key(key, key_len, iv, iv_len, set_key->data, sizeof(set_key->data));
        if (r != 0)
        {
            return r;
        }
    }

    key = set_key->data;
    key_length = 0x10000;
    key_pos = 0;

    size = 140244;

    // allocate_file_space(out_fd, size);

    if (lseek(nks->fd, 803, SEEK_SET) < 0)
        return -1;

    while (size > 0)
    {
        to_read = MIN(sizeof(buffer), size);
        count = read(nks->fd, buffer, to_read);
        if (count != to_read)
            return -1;

        for (x = 0; x < to_read; x++)
        {
            key_pos %= key_length;
            buffer[x] ^= key[key_pos];
            key_pos++;
        }

        count = write(out_fd, buffer, to_read);
        if (count != to_read)
            return -1;

        size -= to_read;
    }

    return 0;
}

int main()
{
    Nks *nks;

    int in_fd, out_fd;
    int r;

    in_fd = open("C:/Users/perner/Amazon Drive/Documents/My Projects/Native Instruments GmbH/Instruments/Neo-Soul Keys/NSR.nkx", O_RDONLY | O_BINARY);
    if (in_fd < 0)
        return -errno;

    nks = (Nks *)malloc(sizeof(*nks));
    nks->root_entry.name = "/";
    nks->root_entry.type = NKS_ENT_DIRECTORY;
    nks->root_entry.offset = 0;
    nks->fd = in_fd;

    out_fd = open("C:/Users/perner/My Projects/Temp/BUMPTHUMPLAYER 2C3-C3-2NEG.ncw", O_WRONLY | O_CREAT | O_BINARY, 0666);
    if (out_fd < 0)
        return -errno;

    r = extract_encrypted_file_entry_to_fd(nks, out_fd);

    close(in_fd);
    close(out_fd);

    return r;
}