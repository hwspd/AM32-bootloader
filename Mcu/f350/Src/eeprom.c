#include "eeprom.h"

#include <string.h>

#include "gd32f3x0_fmc.h"

#define FLASH_PAGE_SIZE 0x400U

bool save_flash_nolib(const uint8_t* data, uint32_t length, uint32_t add)
{
  if ((add & 0x3U) != 0U || (length & 0x3U) != 0U) {
    return false;
  }

  bool success = true;
  fmc_unlock();

  if ((add % FLASH_PAGE_SIZE) == 0U) {
    success = fmc_page_erase(add) == FMC_READY;
  }

  for (uint32_t index = 0U; success && index < length / 4U; index++) {
    uint32_t word;
    memcpy(&word, data + index * 4U, sizeof(word));
    success = fmc_word_program(add + index * 4U, word) == FMC_READY;
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR);
  }

  fmc_lock();
  return success && memcmp(data, (const void*)add, length) == 0;
}

void read_flash_bin(uint8_t* data, uint32_t add, int out_buff_len)
{
  memcpy(data, (const void*)add, out_buff_len);
}
