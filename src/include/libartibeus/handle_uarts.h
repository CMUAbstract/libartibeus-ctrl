#ifndef LIBARTIBEUS_HANDLE_UARTS_
#define LIBARTIBEUS_HANDLE_UARTS_

#include "comm.h"

int handle_progress_uart1(uint8_t);
int handle_progress_uart0(uint8_t);


typedef union buffer_data_t_ {
  uint8_t msg[OPENLST_MAX_PAYLOAD_LEN+PRE_HEADER_LEN];
  openlst_cmd pkt;   
} buffer_data_t;

typedef struct buffer_t_ {
  uint8_t active;
  uint8_t complete;
  buffer_data_t pkt;
} buffer_t;

#endif
