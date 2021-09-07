#ifndef LIBARTIBEUS_HANDLE_UARTS_
#define LIBARTIBEUS_HANDLE_UARTS_

#include "comm.h"

int handle_progress_uart2(uint8_t);
int handle_progress_uart1(uint8_t);
int handle_progress_uart0(uint8_t);
int process_uart0();
int process_uart1();
int process_uart2();


#endif
