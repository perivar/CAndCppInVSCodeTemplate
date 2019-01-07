#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <gcrypt.h>
#include <glib.h>

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
  // if (gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P))
  //   return 0;

  // if (!gcry_check_version(GCRYPT_VERSION))
  // {
  //   fprintf(stderr, "Error: Incompatible gcrypt version.\n");
  //   return 1;
  // }

  // gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
  // gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

  return 0;
}

static void
test_pointer_increment(void)
{
  gcry_cipher_hd_t cipher;
  size_t len = 64;
  uint8_t ctr[16];
  uint8_t *bkp;
  size_t n, m;

  initialise_gcrypt();

  uint8_t *bp = (uint8_t *)malloc(64);
  uint32_t seed = UINT32_C(0x608da0a2);
  printf("bp orig:\n");
  for (n = 0; n < len; n++)
  {
    bp[n] = rand_ms(&seed) & 0xff - 5;
    printf("%d ", (int)bp[n]);
  }
  printf("\n");

  if (nks_0110_base_key == NULL)
    generate_0110_base_key();

  bkp = nks_0110_base_key;

  for (n = 0; 4 * n < len; n++)
  {
    // set bp from ctr

    for (m = 0; m < 4; m++)
    {
      printf("bp: %d ", (int)bp[m]);
      printf(", bkp: %d ", (int)bkp[m]);

      bp[m] ^= bkp[m];

      printf(" = %d\n", (int)bp[m]);
    }

    // increment_counter(ctr, 16);

    bp += 4;
    bkp += 4;
  }
}

int main(int argc, char **argv)
{
  // test_generate_0110_base_key();

  // test_increment_counter();

  test_pointer_increment();

  return 0;
}