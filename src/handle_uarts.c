#include <msp430.h>
#include <stdio.h>

#include <libmspware/driverlib.h>
#include <libmspware/gpio.h>

#include <libmsp/watchdog.h>
#include <libmsp/clock.h>
#include <libmsp/gpio.h>
#include <libmsp/periph.h>
#include <libmsp/sleep.h>
#include <libmsp/mem.h>
#include <libmsp/uart.h>

#include <libio/console.h>
#include <libmspuartlink/uartlink.h>
#include "artibeus.h"
#include "comm.h"
#include "handle_uarts.h"

__nv buffer_t UART1_BUFFERS[UART1_BUFFER_CNT];
__nv buffer_t UART0_BUFFERS[UART0_BUFFER_CNT];

#ifndef BIT_FLIP
#define BIT_FLIP(port,bit) \
	P##port##OUT |= BIT##bit; \
	P##port##DIR |= BIT##bit; \
	P##port##OUT &= ~BIT##bit;
#endif
int process_uart0() {
  // Check for active messages
  BIT_FLIP(1,1);
  uint8_t msg1[1] = {UART0_BUFFER_CNT + 0x30};
  //uartlink_send_basic(1,msg1,1);
  for (int i = 0; i < UART0_BUFFER_CNT; i++) {
    if (UART0_BUFFERS[i].active == 0 || UART0_BUFFERS[i].complete != COMPLETE) {
      //char msg[8] = "No pkt\r\n";
      //uartlink_send_basic(1,msg,8);
      continue;
    }
    // if there is a message, check if it's for us
    // TODO change to dest
    if ((uint16_t) UART0_BUFFERS[i].pkt.msg[HWID_OFFSET] == HWID_CTRL) {
      // If it is, process it
      PRINTF("Got pkt!");
      char msg[9] = "Got pkt\r\n";
      //uartlink_send_basic(1,msg,9);
       switch(UART0_BUFFERS[i].pkt.msg[CMD_OFFSET]) {
        case ACK:
          break;
        case GET_TELEM:
          break;
        case ASCII:
          // Kill keys here
          // Score repeat here
          break;
        default:
          // Command not supported
          break;
      }
    }
    else {
      EXP_ENABLE;
      // If it's not, send it to the experiment board
      expt_send_raw(&(UART0_BUFFERS[i]));
      //TODO figure out when to disable
    }
    UART0_BUFFERS[i].complete = 0;
    UART0_BUFFERS[i].active = 0;
  }
  return 0;
}

int process_uart1() {
  // Check for active messages
  BIT_FLIP(1,1);
  uint8_t msg1[1] = {UART1_BUFFER_CNT + 0x30};
  //uartlink_send_basic(1,msg1,1);
  for (int i = 0; i < UART1_BUFFER_CNT; i++) {
    if (UART1_BUFFERS[i].active == 0 || UART1_BUFFERS[i].complete != COMPLETE) {
      //char msg[8] = "No pkt\r\n";
      //uartlink_send_basic(1,msg,8);
      continue;
    }
    // if there is a message, check if it's for us
    // TODO change to dest
    if ((uint16_t) UART1_BUFFERS[i].pkt.msg[HWID_OFFSET] == HWID_CTRL) {
      // If it is, process it
      PRINTF("Got pkt!");
      char msg[9] = "Got pkt\r\n";
      //uartlink_send_basic(1,msg,9);
       switch(UART1_BUFFERS[i].pkt.msg[CMD_OFFSET]) {
        case ACK:
          break;
        case GET_TELEM:
          break;
        case ASCII:
          // Kill keys here
          // Score repeat here
          break;
        default:
          // Command not supported
          break;
      }
    }
    else {
      COMM_ENABLE;
      // If it's not, send it to the experiment board
      comm_send_raw(&(UART1_BUFFERS[i]));
      //TODO figure out when to disable
    }
    UART1_BUFFERS[i].complete = 1;
    UART1_BUFFERS[i].active = 0;
  }
  return 0;
}

// Returns 1 if out of space, 0 if ok
int handle_progress_uart0(uint8_t data) {
  uint8_t buffer_cnt = UART0_BUFFER_CNT;
  buffer_t *buffers = UART0_BUFFERS;
  static incoming_status_t progress;
  static uint16_t prog_len = 0;
  static uint8_t prog_counter = 0;
  static buffer_num = -1;
  BIT_FLIP(1,2);
  switch(progress) {
    case wait_esp0: // Waiting for start1
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      prog_counter = 0;
      if (data == ESP_BYTE0) {
        progress = wait_esp1;
      }
      break;
    case wait_esp1: // Waiting for start2
      prog_counter = 0;
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      if (data == ESP_BYTE1) {
        prog_len = 0;
        progress = wait_len;
      } else { // if byte rx'd after ESP_BYTE0 isn't ESP_BYTE1, then start over
        progress = wait_esp0;
      }
      break;
    case wait_len:
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      prog_len = data;
      // Give up if packet is too long or too short
      if (prog_len > UARTLINK_MAX_PAYLOAD_SIZE || prog_len < 6) {
        progress = wait_esp0;
        return 1;
      }
      buffer_num = -1;
      prog_counter = 0;
      for(int i = 0; i < buffer_cnt; i++) {
        if(buffers[i].active && buffers[i].complete == COMPLETE) { continue;}
        else { buffer_num = i; break;}
      }
      if (buffer_num < 0) {
        // Out of buffers
        progress = wait_esp0;
        return 1;
      }
      buffers[buffer_num].active = 1;
      buffers[buffer_num].full_len = data;
      progress = receive_data;
      break;
    case receive_data:
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      buffers[buffer_num].pkt.msg[prog_counter+PRE_HEADER_LEN] = data;
      prog_counter++;
      if (prog_counter >= prog_len) {
        //Mark as complete
        buffers[buffer_num].complete = COMPLETE;
        prog_counter = 0;
        prog_len = 0;
        progress = wait_esp0;
      }
      else {
        progress = receive_data;
      }
      break;
    default:
      progress = wait_esp0;
      break;
  }
  return 0;
}

int handle_progress_uart1(uint8_t data) {
  uint8_t buffer_cnt = UART1_BUFFER_CNT;
  buffer_t *buffers = UART1_BUFFERS;
  static incoming_status_t progress;
  static uint16_t prog_len = 0;
  static uint8_t prog_counter = 0;
  static buffer_num = -1;
  BIT_FLIP(1,2);
  switch(progress) {
    case wait_esp0: // Waiting for start1
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      prog_counter = 0;
      if (data == ESP_BYTE0) {
        progress = wait_esp1;
      }
      break;
    case wait_esp1: // Waiting for start2
      prog_counter = 0;
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      if (data == ESP_BYTE1) {
        prog_len = 0;
        progress = wait_len;
      } else { // if byte rx'd after ESP_BYTE1 isn't ESP_BYTE1, then start over
        progress = wait_esp0;
      }
      break;
    case wait_len:
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      prog_len = data;
      // Give up if packet is too long or too short
      if (prog_len > UARTLINK_MAX_PAYLOAD_SIZE || prog_len < 6) {
        progress = wait_esp0;
        return 1;
      }
      buffer_num = -1;
      prog_counter = 0;
      for(int i = 0; i < buffer_cnt; i++) {
        if(buffers[i].active && buffers[i].complete == COMPLETE) { continue;}
        else { buffer_num = i; break;}
      }
      if (buffer_num < 0) {
        // Out of buffers
        progress = wait_esp0;
        return 1;
      }
      buffers[buffer_num].active = 1;
      buffers[buffer_num].full_len = data;
      progress = receive_data;
      break;
    case receive_data:
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      BIT_FLIP(1,2);
      buffers[buffer_num].pkt.msg[prog_counter+PRE_HEADER_LEN] = data;
      prog_counter++;
      if (prog_counter >= prog_len) {
        //Mark as complete
        buffers[buffer_num].complete = COMPLETE;
        prog_counter = 0;
        prog_len = 0;
        progress = wait_esp0;
      }
      else {
        progress = receive_data;
      }
      break;
    default:
      progress = wait_esp0;
      break;
  }
  return 0;
}

