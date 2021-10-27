#ifndef LIBARTIBEUS_HANDLE_UARTS_
#define LIBARTIBEUS_HANDLE_UARTS_

#include "comm.h"

int handle_progress_uart2(uint8_t);
int handle_progress_uart1(uint8_t);
int handle_progress_uart0(uint8_t);
int process_uart0();
int process_uart1();
int process_uart2();

extern uint16_t __nv expt_msg_id_pending;
extern uint8_t __nv expt_ack_pending;
extern uint16_t __nv comm_msg_id_pending;
extern uint8_t __nv comm_ack_pending;
extern uint16_t __nv libartibeus_msg_id;
// Leave volatile
extern uint8_t libartibeus_uartlink2_pkt_error;

#endif
