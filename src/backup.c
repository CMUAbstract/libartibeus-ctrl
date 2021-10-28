#include "backup.h"
#include <libmsp/mem.h>
#include <libio/console.h>
#include <libmspware/driverlib.h>

__nv artibeus_ctx ctx0 = { ADC, -1 };
__nv artibeus_ctx ctx1 = { ADC, -1 };
__nv artibeus_ctx *cur_ctx = &ctx0;

// We're only going to reference these here
static __nv uint8_t backup_data[BACKUP_DATA_LEN]; // Buffer where we'll put data
// Data structure for each double buffered entry
static __nv dbl_buffer_entry double_buffer[DBL_BUFF_LEN];

void restore_from_backup(artibeus_ctx *ctx) {
  LOG2("Got %i items to restore\r\n",ctx->dbl_buffer_count);
  while(ctx->dbl_buffer_count > -1) {
    // Memcpy data
    LOG2("Restoring!\r\n");
    uint8_t *source = double_buffer[ctx->dbl_buffer_count].source;
    uint8_t *dest = double_buffer[ctx->dbl_buffer_count].dest;
    size_t len = double_buffer[ctx->dbl_buffer_count].len;
    memcpy(source, dest, len);
    ctx->dbl_buffer_count--;
  }
  return;
}

int write_to_log(artibeus_ctx *ctx, uint8_t *data, size_t len) {
  int16_t index = ctx->dbl_buffer_count;
  index++;
  if (index > DBL_BUFF_LEN) {
    return -1;
  }
  //PRINTF("Writing! %i, len = %i\r\n", *data, len);
  // Get address of last entry in buffer
  uint8_t *last_addr;
  if (index > 0) {
    last_addr = double_buffer[index-1].dest + double_buffer[index-1].len;
  }
  else {
    // Set address to start of buffer if first entry
    last_addr = backup_data;
  }
  // Add one to get to very next available address
  last_addr++;
  memcpy(last_addr, data, len);
  // This is fine because we only log __nv data
  double_buffer[index].source = data;
  double_buffer[index].dest = last_addr;
  double_buffer[index].len = len;
  ctx->dbl_buffer_count++;
  return 0;
}

