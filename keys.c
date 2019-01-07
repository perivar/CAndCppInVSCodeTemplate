#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
// #include <glib.h>

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

int main(int argc, char **argv)
{
  // if (nks_0110_base_key == NULL)
  //   generate_0110_base_key();

  // int i;
  // for (i = 0; i < 0x100; i++)
  // {
  //   printf("%d ", (int)nks_0110_base_key[i]);
  // }
  // printf("\n");

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

  return 0;
}