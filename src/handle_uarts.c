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
#include <libgnss/gnss.h>
#include "artibeus.h"
#include "comm.h"
#include "query.h"
#include "handle_uarts.h"
#include "backup.h"

// Variables we update in these functions
uint16_t __nv expt_msg_id_pending = 0;
uint8_t __nv expt_ack_pending = 0;
uint16_t __nv comm_msg_id_pending = 0;
uint8_t __nv comm_ack_pending = 0;

uint16_t __nv libartibeus_msg_id = 0;

// Leave volatile
uint8_t libartibeus_uartlink2_pkt_error = 0;

__nv buffer_t UART1_BUFFERS[UART1_BUFFER_CNT];
__nv buffer_t UART0_BUFFERS[UART0_BUFFER_CNT];

#ifndef BIT_FLIP
#define BIT_FLIP(port,bit) \
	P##port##OUT |= BIT##bit; \
	P##port##DIR |= BIT##bit; \
	P##port##OUT &= ~BIT##bit;
#endif
// This is the Comm-side UART.
// We return everything here with an LST_RELAY command since if it showed up
// here it came from the ground.
//We need to look for telemetry requests and kill keys. We also do ACK.
int process_uart0() {
  // Check for active messages
  int ret_val = -1;
  for (int i = 0; i < UART0_BUFFER_CNT; i++) {
    if (UART0_BUFFERS[i].active == 0 || UART0_BUFFERS[i].complete != COMPLETE) {
      continue;
    }
    // Want these updates to be atomic
    write_to_log(cur_ctx,&(UART0_BUFFERS[i].complete),sizeof(uint8_t));
    write_to_log(cur_ctx,&(UART0_BUFFERS[i].active),sizeof(uint8_t));
    // if there is a message, check if it's for us
    if (GET_TO(UART0_BUFFERS[i].pkt.msg[DEST_OFFSET]) == DEST_CTRL) {
      // If it is, process it
      LOG("Got pkt!");
       switch(UART0_BUFFERS[i].pkt.msg[CMD_OFFSET]) {
        case ACK:
          // Check if this is the responding ack
          if (comm_ack_pending) {
            uint16_t cur_seqnum = TRANSLATE_SEQNUM(UART0_BUFFERS[i].pkt.msg[SEQ_NUM_OFFSET]);
            if (cur_seqnum == comm_msg_id_pending) {
              ret_val = RCVD_PENDING_ACK;
              break;
            }
          }
          // Return Ack
          comm_return_ack(&(UART0_BUFFERS[i]));
          ret_val =  ACK;
          break;
        case GET_TELEM:
          comm_return_telem(&(UART0_BUFFERS[i]));
          ret_val =  GET_TELEM;
          break;
        case ASCII:
          ret_val =  ASCII;
          switch(UART0_BUFFERS[i].pkt.msg[SUB_CMD_OFFSET]) {
            // Kill keys here
            case RF_KILL:
              // Intentionally leaving unprotected, it's nbd if we have an extra
              // jump or 2 on the kill count
              update_rf_kill_count(&(UART0_BUFFERS[i]));
              break;
            // Score repeat here
            case SCORE:
              // We'll re-run this if it fails, so we'll only have goofed
              // atomicity for a window that isn't visible
              update_score(&(UART0_BUFFERS[i]));
              break;
            case TELEM_ASCII:
              // pops off of the telem stack
              artibeus_send_telem_ascii_pkt(&(UART0_BUFFERS[i]));
              ret_val = RCVD_TELEM_ASCII;
              break;
            case BUFF_REQ_ASCII:
              BIT_FLIP(1,1);
              BIT_FLIP(1,1);
              BIT_FLIP(1,1);
              BIT_FLIP(1,1);
              // pops off of special expt data statck
              artibeus_send_ascii_pkt(&(UART0_BUFFERS[i]));
              ret_val = RCVD_BUFF_REQ_ASCII;
              break;
            default:
              break;
          }
          break;
        default:
          // Command not supported
          break;
      }
    }
    else {
      EXP_ENABLE;
      // If it's not, send it to the experiment board
      // Just a write, no atomicity issues
      expt_send_raw(&(UART0_BUFFERS[i]));
      //TODO figure out when to disable
    }
    UART0_BUFFERS[i].complete = 0;
    UART0_BUFFERS[i].active = 0;
  }
  return ret_val;
}

int process_uart1() {
  // Check for active messages
  //BIT_FLIP(1,1);
  int ret_val = -1;
  for (int i = 0; i < UART1_BUFFER_CNT; i++) {
    //BIT_FLIP(1,1);
    if (UART1_BUFFERS[i].active == 0 || UART1_BUFFERS[i].complete != COMPLETE) {
      continue;
    }
    // Want these updates to be atomic
    write_to_log(cur_ctx,&(UART0_BUFFERS[i].complete),sizeof(uint8_t));
    write_to_log(cur_ctx,&(UART0_BUFFERS[i].active),sizeof(uint8_t));
    // if there is a message, check if it's for us
    if (GET_TO(UART1_BUFFERS[i].pkt.msg[DEST_OFFSET]) == DEST_CTRL) {
      //BIT_FLIP(1,1);
      // If it is, process it
      LOG("Got pkt!");
      switch(UART1_BUFFERS[i].pkt.msg[CMD_OFFSET]) {
        case ACK:
          // Check if this is the responding ack
          if (expt_ack_pending) {
            uint16_t cur_seqnum = TRANSLATE_SEQNUM(UART1_BUFFERS[i].pkt.msg[SEQ_NUM_OFFSET]);
            if (cur_seqnum == expt_msg_id_pending) {
              ret_val =  RCVD_PENDING_ACK;
              break;
            }
          }
          // Return Ack
          expt_return_ack(&(UART1_BUFFERS[i]));
          ret_val =  ACK;
          break;
        case GET_TELEM:
          break;
        case GET_TIME:{
          uint8_t* time_date[ARTIBEUS_TIME_DATE_SIZE];
          uint8_t* time = artibeus_get_time();
          uint8_t* date = artibeus_get_date();
          memcpy(time_date,time,3);
          memcpy(time_date + 3,date,3);
          libartibeus_msg_id = UART1_BUFFERS[i].pkt.msg[SEQ_NUM_OFFSET];
          expt_set_time(time_date);
          }
          ret_val =  GET_TIME;
          break;
        case GET_TIME_UTC:{
          uint8_t* time_date[ARTIBEUS_TIME_DATE_SIZE];
          uint8_t* time = artibeus_get_time();
          uint8_t* date = artibeus_get_date();
          memcpy(time_date,time,3);
          memcpy(time_date + 3,date,3);
          libartibeus_msg_id = UART1_BUFFERS[i].pkt.msg[SEQ_NUM_OFFSET];
          expt_set_time_utc(time_date);
          }
          ret_val =  GET_TIME;
          break;
        case BOOTLOADER_ACK:  {
          if (expt_ack_pending) {
            //BIT_FLIP(1,1);
            //BIT_FLIP(1,1);
            //BIT_FLIP(1,1);
            //BIT_FLIP(1,1);
            uint16_t cur_seqnum = TRANSLATE_SEQNUM(UART1_BUFFERS[i].pkt.msg[SEQ_NUM_OFFSET]);
            if (cur_seqnum == expt_msg_id_pending) {
              ret_val =  RCVD_PENDING_BOOTLOADER_ACK;
              break;
            }
          }
          }
          break;
        case ASCII:{
          // Push into buffer
          artibeus_push_ascii_pkt(&UART1_BUFFERS[i].pkt.msg);
          ret_val = ASCII;
          }
          // Put data in ring buffer, export down eventually
          // Copy all bytes after first 3 bytes into ring buffer
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
  return ret_val;
}

// Returns 1 if out of space, 0 if ok
int handle_progress_uart0(uint8_t data) {
  uint8_t buffer_cnt = UART0_BUFFER_CNT;
  buffer_t *buffers = UART0_BUFFERS;
  static incoming_status_t progress;
  static uint16_t prog_len = 0;
  static uint8_t prog_counter = 0;
  static int buffer_num = -1;
  switch(progress) {
    case wait_esp0: // Waiting for start1
      prog_counter = 0;
      if (data == ESP_BYTE0) {
        progress = wait_esp1;
      }
      break;
    case wait_esp1: // Waiting for start2
      prog_counter = 0;
      if (data == ESP_BYTE1) {
        prog_len = 0;
        progress = wait_len;
      } else { // if byte rx'd after ESP_BYTE0 isn't ESP_BYTE1, then start over
        progress = wait_esp0;
      }
      break;
    case wait_len:
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
  static int buffer_num = -1;
  //BIT_FLIP(1,1);
  switch(progress) {
    case wait_esp0: // Waiting for start1
      //BIT_FLIP(1,1);
      prog_counter = 0;
      if (data == ESP_BYTE0) {
        progress = wait_esp1;
      }
      break;
    case wait_esp1: // Waiting for start2
      //BIT_FLIP(1,1);
      //BIT_FLIP(1,1);
      prog_counter = 0;
      if (data == ESP_BYTE1) {
        prog_len = 0;
        progress = wait_len;
      } else { // if byte rx'd after ESP_BYTE1 isn't ESP_BYTE1, then start over
        progress = wait_esp0;
      }
      break;
    case wait_len:
      //BIT_FLIP(1,1);
      //BIT_FLIP(1,1);
      //BIT_FLIP(1,1);
      prog_len = data;

      // Give up if packet is too long or too short
      if (prog_len > UARTLINK_MAX_PAYLOAD_SIZE || prog_len < 6) {
        progress = wait_esp0;
        //BIT_FLIP(1,1);
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
        //BIT_FLIP(1,1);
        //BIT_FLIP(1,1);
        //BIT_FLIP(1,1);
        //BIT_FLIP(1,1);
        //BIT_FLIP(1,1);
        progress = wait_esp0;
        return 1;
      }
      buffers[buffer_num].active = 1;
      buffers[buffer_num].full_len = data;
      progress = receive_data;
      break;
    case receive_data:
      buffers[buffer_num].pkt.msg[prog_counter+PRE_HEADER_LEN] = data;
      prog_counter++;
      if (prog_counter >= prog_len) {
        //Mark as complete
        buffers[buffer_num].complete = COMPLETE;
        prog_counter = 0;
        prog_len = 0;
        //BIT_FLIP(1,1);
        //BIT_FLIP(1,1);
        //BIT_FLIP(1,1);
        //BIT_FLIP(1,1);
        //BIT_FLIP(1,1);
        //BIT_FLIP(1,1);
        //for (int i = 0; i < buffer_num; i++) { BIT_FLIP(1,1); }
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

int handle_progress_uart2(uint8_t data) {
  int pkt_done = 0;
  int pkt_error = 1;
  int need_fix = -1;
  //PRINTF("|got: %u, %c |\r\n",gnss_pkt_counter, data);
  if (data == '$') {
    gnss_pkt_counter = 0;
    gnss_pkt_type = IN_PROGRESS;
    gnss_active_pkt = 0;
    LOG("Starting!\r\n");
    return pkt_done;
  }
  else if (gnss_pkt_type == IN_PROGRESS) {
    // Try to find packet header
    LOG("finding type\r\n");
    get_sentence_type(data);
    return pkt_done;
  }
  else if (gnss_active_pkt) {
    LOG("Active pkt!\r\n");
    pkt_done = get_sentence_pkt(data);
    if (!pkt_done) {
      return pkt_done;
    }
  }
  else {
    return pkt_done;
  }
  gps_data *next_gps_data;
  if (pkt_done) {
    gnss_active_pkt = 0;
    gnss_pkt_counter = 0;
    next_gps_data = (cur_gps_data == &gps_data1) ?
      &gps_data2 : &gps_data1;
    // Actually a fairly substantial operation...
    // TODO can we reduce the worst case execution time here somehow?
    pkt_error = process_sentence_pkt(cur_gnss_ptr, next_gps_data);
  }
  if (!pkt_error && next_gps_data->fix[0] == FIX_OK) {
    //Update latest gps coordinates if time is newer
    need_fix = 0;
    libartibeus_uartlink2_pkt_error = 0;
    int time_temp = time_compare(next_gps_data, cur_gps_data);
    if ( time_temp >= 0) {
      cur_gps_data = next_gps_data;
      LOG("comp: %i\r\n",cur_gps_data->fix[0] == FIX_OK);
    }
    else {
      LOG("No time change!\r\n");
    }
  }
  else {
    BIT_FLIP(1,2);
    BIT_FLIP(1,1);
    BIT_FLIP(1,2);
    LOG("Pkt error!\r\n");
    libartibeus_uartlink2_pkt_error = 1;
  }
  return pkt_done;
}
