#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <gcrypt.h>

uint32_t
rand_ms(uint32_t *seedp)
{
  *seedp = *seedp * UINT32_C(0x343fd) + UINT32_C(0x269ec3);
  return (*seedp >> 16);
}

static uint8_t *nks_0110_base_key;

static void
generate_0110_base_key(void)
{
  int n;
  uint32_t seed;

  if (nks_0110_base_key != NULL)
    return;

  // nks_0110_base_key = (uint8_t *) g_malloc(0x10000);
  nks_0110_base_key = (uint8_t *)malloc(0x10000);
  seed = UINT32_C(0x608da0a2);

  for (n = 0; n < 0x10000; n++)
    nks_0110_base_key[n] = rand_ms(&seed) & 0xff;
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
test_generate_0110_base_key(void)
{
  if (nks_0110_base_key == NULL)
    generate_0110_base_key();

  int i;
  for (i = 0; i < 0x100; i++)
  {
    printf("%d ", (int)nks_0110_base_key[i]);
  }
  printf("\n");
}

static void
test_increment_counter(void)
{
  uint8_t *tmp_array = (uint8_t *)malloc(16);
  uint32_t seed = UINT32_C(0x608da0a2);

  int n;
  for (n = 0; n < 16; n++)
  {
    tmp_array[n] = rand_ms(&seed) & 0xff;
  }
  tmp_array[15] = 0xff;
  tmp_array[14] = 0xff;
  tmp_array[13] = 0xff;

  printf("Original array:\n");
  int i;
  for (i = 0; i < 16; i++)
  {
    printf("%d ", (int)tmp_array[i]);
  }
  printf("\n");

  for (n = 0; n < 5; n++)
  {
    increment_counter(tmp_array, 16);

    printf("Array after increment_counter:\n");
    for (i = 0; i < 16; i++)
    {
      printf("%d ", (int)tmp_array[i]);
    }
    printf("\n");
  }
}

static int
initialise_gcrypt(void)
{
  if (gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P))
    return 0;

  if (!gcry_check_version(GCRYPT_VERSION))
  {
    fprintf(stderr, "Error: Incompatible gcrypt version.\n");
    return 1;
  }

  gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
  gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

  return 0;
}

static void
print_array(uint8_t *num, size_t len)
{
  size_t n;
  for (n = 0; n < len; n++)
  {
    printf("%d ", (int)num[n]);
  }
}

static void
test_pointer_increment(void)
{
  size_t len = 64;

  gcry_cipher_hd_t cipher;
  uint8_t ctr[16];
  uint8_t *bkp;
  size_t n, m;
  int algo;
  int err;

  uint8_t key[] = {0xAB, 0x90, 0x62, 0x62, 0x5F, 0x0C, 0x27, 0x75, 0x2B, 0x5C, 0x2A, 0xC8, 0x19, 0x1A, 0xB3, 0x1E, 0xDA, 0x72, 0x07, 0x42, 0xCB, 0x0B, 0x22, 0xF5, 0xB4, 0x5E, 0xB8, 0x96, 0xB9, 0x9C, 0x0B, 0xD2};
  size_t key_len = sizeof(key);

  uint8_t iv[] = {0xA7, 0xFA, 0xC4, 0x1D, 0x35, 0x21, 0x49, 0x59, 0x8D, 0x91, 0xE6, 0x0D, 0xAE, 0xF9, 0x99, 0xDE};
  size_t iv_len = sizeof(iv);

  printf("key:\n");
  print_array(key, key_len);
  printf("\n");

  printf("iv:\n");
  print_array(iv, iv_len);
  printf("\n");

  initialise_gcrypt();

  // set bp = buffer
  uint8_t *bp = (uint8_t *)malloc(64);
  uint32_t seed = UINT32_C(0x608da0a2);
  // for (n = 0; n < len; n++)
  // {
  // bp[n] = rand_ms(&seed) & 0xff;
  // }
  printf("\n");
  printf("bp (buffer) orig:\n");
  print_array(bp, len);
  printf("\n");

  if (nks_0110_base_key == NULL)
    generate_0110_base_key();

  switch (key_len)
  {
  case 16:
    algo = GCRY_CIPHER_AES128;
    printf("AES128\n");
    break;
  case 24:
    printf("AES192\n");
    algo = GCRY_CIPHER_AES192;
    break;
  case 32:
    printf("AES256\n");
    algo = GCRY_CIPHER_AES256;
    break;
  default:
    break;
  }

  // Open cipher
  printf("Opening AES cipher...\n");
  err = gcry_cipher_open(&cipher, algo, GCRY_CIPHER_MODE_ECB, 0);
  if (err)
  {
    printf("Failed opening AES cipher: %s\n", gpg_strerror(err));
    return;
  }

  // Set key for cipher
  printf("Setting the key...\n");
  err = gcry_cipher_setkey(cipher, key, key_len);
  if (err)
  {
    printf("Failed setting the key: %s\n", gpg_strerror(err));
    return;
  }

  memcpy(ctr, iv, 16);

  bkp = nks_0110_base_key;

  for (n = 0; 16 * n < len; n++)
  {
    // Encrypt
    printf("\n");
    printf("Ctr before encryption...\n");
    print_array(ctr, 16);
    printf("\n");

    err = gcry_cipher_encrypt(cipher, bp, 16, ctr, 16);
    if (err)
    {
      printf("Encryption failed: %s\n", gpg_strerror(err));
      return;
    }

    // printf("ctr after encryption...\n");
    // print_array(ctr, 16);
    // printf("\n");

    printf("After encryption...\n");
    for (m = 0; m < 16; m++)
    {
      printf("buffer: %d ", (int)bp[m]);
      printf(", basekeyp: %d ", (int)bkp[m]);

      bp[m] ^= bkp[m];

      printf(" => bp after ^= : %d\n", (int)bp[m]);
    }

    increment_counter(ctr, 16);

    printf("Ctr after increment ...\n");
    print_array(ctr, 16);
    printf("\n");

    bp += 16;
    bkp += 16;
  }

  gcry_cipher_close(cipher);
}

int main(int argc, char **argv)
{
  // test_generate_0110_base_key();

  // test_increment_counter();

  test_pointer_increment();

  return 0;
}