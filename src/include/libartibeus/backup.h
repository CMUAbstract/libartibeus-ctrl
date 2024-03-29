#ifndef _ARTIBEUS_BACKUP_H_
#define _ARTIBEUS_BACKUP_H_

#include <msp430.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h>


#include <libartibeus/artibeus.h>

#define DBL_BUFF_LEN 16
#define BACKUP_DATA_LEN 128


typedef struct dbl_buffer_entry_ {
  // Original data source address
  uint8_t *source;
  // Destination address in buffer from copy during log write
  uint8_t *dest;
  // Length in bytes
  size_t len;
} dbl_buffer_entry;

typedef struct artibeus_ctx_ {
  artibeus_mode cur_task;
  int16_t dbl_buffer_count;
} artibeus_ctx;

void restore_from_backup(artibeus_ctx *ctx);
int write_to_log(artibeus_ctx *ctx, uint8_t *data, size_t len);

extern artibeus_ctx ctx0;
extern artibeus_ctx ctx1;
extern artibeus_ctx *cur_ctx;

#endif //ARTIBEUS_BACKUP_H_
