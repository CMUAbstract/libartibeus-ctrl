// C file of all the basic artibeus functions
#include <msp430.h>
#include <stdio.h>
#include <string.h>

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
#include "handle_uarts.h"
#include "comm.h"
#include "artibeus.h"
#include "query.h"

#define COMM_TEST_LEN 128

// We want this volatile!!
uint8_t rf_kill_count = 0;
__nv uint8_t libartibeus_rf_dead = 0;

static cmd_pkt comm_msg = {.byte0 = ESP_BYTE0, .byte1 = ESP_BYTE1};
static cmd_pkt comm_resp;

static cmd_pkt expt_msg = {.byte0 = ESP_BYTE0, .byte1 = ESP_BYTE1};
static cmd_pkt expt_resp;

 __nv uint8_t RF_KILL_KEYS[16] ={'S','T','O','P',' ','b','r','o','a','d','C','A','S','T','!','!',};
 __nv uint8_t EXPT_WAKE_KEYS[8] = {'W','a','K','e','U','p','T','A'};
__nv uint8_t score_msg[32];

int comm_format_pkt(openlst_cmd *cmd) {
  // First things firs,t Size check
  if (cmd->cmd_len + sizeof(cmd_header) > OPENLST_MAX_PAYLOAD_LEN) {
    return -1;
  }
  // Set len-- we add 6 bytes to the cmd_len to factor in the hwid, etc
  comm_msg.total_len = sizeof(cmd_header) + cmd->cmd_len;
  // Set hwid
  comm_msg.msg.header.hwid0 = cmd->hwid & 0xFF;
  comm_msg.msg.header.hwid1 = cmd->hwid >> 8;
  // Set seqnum
  comm_msg.msg.header.seqnum0 = cmd->seqnum & 0xFF;
  comm_msg.msg.header.seqnum1 = cmd->seqnum >> 8;
  // Dest
  comm_msg.msg.header.dest = cmd->dest;
  // Cmd
  comm_msg.msg.header.cmd = cmd->cmd;
  for (size_t i = 0; i < cmd->cmd_len; i++) {
    comm_msg.msg.msg[i] = *(cmd->payload + i);
  }
  return 0;
}

int expt_format_pkt(openlst_cmd *cmd) {
  // First things firs,t Size check
  if (cmd->cmd_len + sizeof(cmd_header) > OPENLST_MAX_PAYLOAD_LEN) {
    return -1;
  }
  // Set len-- we add 6 bytes to the cmd_len to factor in the hwid, etc
  expt_msg.total_len = sizeof(cmd_header) + cmd->cmd_len;
  cmd->cmd_len += sizeof(cmd_header);//TODO check if comm pakets need this!!!
  // Set hwid
  expt_msg.msg.header.hwid0 = cmd->hwid & 0xFF;
  expt_msg.msg.header.hwid1 = cmd->hwid >> 8;
  // Set seqnum
  expt_msg.msg.header.seqnum0 = cmd->seqnum & 0xFF;
  expt_msg.msg.header.seqnum1 = cmd->seqnum >> 8;
  // Dest
  expt_msg.msg.header.dest = cmd->dest;
  // Cmd
  expt_msg.msg.header.cmd = cmd->cmd;
  for (size_t i = 0; i < cmd->cmd_len; i++) {
    expt_msg.msg.msg[i] = *(cmd->payload + i);
  }
  return 0;
}

uint8_t comm_decode_response(void) {
  // Check that the initial bytes are correct
  if (comm_resp.byte0 != ESP_BYTE0 || comm_resp.byte1 != ESP_BYTE1) {
    return OPENLST_ERROR;
  }
  // TODO use the sequence number to process multiple requests in flight
  // if (comm_seqnum_check) { return OPENLST_ERROR;}

  // TODO add a check for packet length n'at
  return comm_resp.msg.header.cmd;
}

void  comm_send_cmd(openlst_cmd *pkt) {
  comm_format_pkt(pkt);
  // TODO fold 9 in as a macro or something, this major number nonsense is
  // annoying
  uartlink_send_basic(LIBMSPUARTLINK0_UART_IDX, &comm_msg, 9 + pkt->cmd_len);
  return;
}

void  expt_send_cmd(openlst_cmd *pkt) {
  expt_format_pkt(pkt);
  // TODO fold 9 in as a macro or something, this major number nonsense is
  // annoying
  uartlink_send_basic(LIBMSPUARTLINK1_UART_IDX, &expt_msg, 3 + pkt->cmd_len);
  return;
}

void expt_send_raw(buffer_t* raw_pkt) {
  raw_pkt->pkt.msg[0] = 0x22;
  raw_pkt->pkt.msg[1] = 0x69;
  raw_pkt->pkt.msg[2] = raw_pkt->full_len;
  uartlink_send_basic(LIBMSPUARTLINK1_UART_IDX,raw_pkt->pkt.msg,PRE_HEADER_LEN+raw_pkt->full_len);
}

void comm_send_raw(buffer_t* raw_pkt) {
  raw_pkt->pkt.msg[0] = 0x22;
  raw_pkt->pkt.msg[1] = 0x69;
  raw_pkt->pkt.msg[2] = raw_pkt->full_len;
  uartlink_send_basic(LIBMSPUARTLINK0_UART_IDX,raw_pkt->pkt.msg,PRE_HEADER_LEN+raw_pkt->full_len);
}

unsigned comm_ack_check() {
  openlst_cmd ack_cmd;
  ack_cmd.hwid = HWID;
  //TODO make this a global counter
  ack_cmd.seqnum = 0x0000;
  //ack_cmd.dest = LST;
  ack_cmd.dest = FROM_CTRL + DEST_COMM;
  ack_cmd.cmd = ACK;
  ack_cmd.cmd_len = 0;
  comm_send_cmd(&ack_cmd);
  unsigned count;
  /*while(!count) {
    //TODO make a general recieve function so we don't have to predict what the
    //packet length will be
    count = uartlink_receive_basic(LIBMSPUARTLINK0_UART_IDX,&comm_resp,9);
  }
  uint8_t resp_cmd = comm_decode_response();*/
  return count;
}


unsigned expt_ack_check() {
  openlst_cmd ack_cmd;
  ack_cmd.hwid = HWID;
  //TODO make this a global counter
  ack_cmd.seqnum = 0x0000;
  ack_cmd.dest = FROM_CTRL + DEST_EXPT;
  ack_cmd.cmd = BOOTLOADER_ACK;
  ack_cmd.cmd_len = 0;
  uartlink_open_tx(1);
  expt_send_cmd(&ack_cmd);
  uartlink_close(1);
  uartlink_open_rx(1);
  unsigned count = 0;
  count = uartlink_receive_basic(LIBMSPUARTLINK1_UART_IDX,&expt_resp,9);
  uartlink_close(1);
  /*while(!count) {
    //TODO make a general recieve function so we don't have to predict what the
    //packet length will be
    count = uartlink_receive_basic(LIBMSPUARTLINK1_UART_IDX,&expt_resp,9);
  }*/
  //uint8_t resp_cmd = comm_decode_response();
  return count;
}

void comm_rf_check() {
  openlst_cmd ack_cmd;
  ack_cmd.hwid = HWID;
  //ack_cmd.hwid = 0x0001;
  //TODO make this a global counter
  ack_cmd.seqnum = 0x0000;
  ack_cmd.dest = FROM_CTRL + DEST_TERM;
  ack_cmd.cmd = ASCII;
  ack_cmd.cmd_len = COMM_TEST_LEN;
  uint8_t msg_test[COMM_TEST_LEN];
  ack_cmd.payload = msg_test;
  for(int i = 0; i < COMM_TEST_LEN; i++) {
    msg_test[i] = 0xFA;
  }
  comm_send_cmd(&ack_cmd);
  return;
}

void comm_transmit_pkt(char *pkt, uint16_t len) {
  openlst_cmd ascii_cmd;
  ascii_cmd.hwid = HWID;
  ascii_cmd.dest = FROM_CTRL + DEST_TERM;
  ascii_cmd.cmd = ASCII;
  uint8_t seqnum =0;
  uint16_t index = 0;
  while(len > OPENLST_MAX_PAYLOAD_LEN) {
    // Pet watchdog
    msp_watchdog_kick();
    ascii_cmd.seqnum = seqnum;
    ascii_cmd.cmd_len = OPENLST_MAX_PAYLOAD_LEN ;
    ascii_cmd.payload = pkt + (len - OPENLST_MAX_PAYLOAD_LEN); 
    comm_send_cmd(&ascii_cmd);
    len -= OPENLST_MAX_PAYLOAD_LEN;
    index += OPENLST_MAX_PAYLOAD_LEN;
    seqnum++;
  }
  ascii_cmd.seqnum = seqnum;
  ascii_cmd.cmd_len = len;
  ascii_cmd.payload = pkt + index; 
  comm_send_cmd(&ascii_cmd);
  return;
}


void comm_transmit_ready() {
  openlst_cmd ack_cmd;
  ack_cmd.hwid = HWID;
  ack_cmd.seqnum = 0x0000;
  ack_cmd.dest = FROM_CTRL + DEST_TERM;
  ack_cmd.cmd = EXPT_LISTENING;
  ack_cmd.cmd_len = 1;
  uint8_t msg_test[1];
  ack_cmd.payload = msg_test;
  // magic number
  msg_test[0] = 0x64;
  comm_send_cmd(&ack_cmd);
  return;
}

void expt_write_jump() {
  openlst_cmd write_pg_cmd;
  write_pg_cmd.hwid = HWID;
  //TODO make this a global counter
  write_pg_cmd.seqnum = 0x0000;
  expt_msg_id_pending = write_pg_cmd.seqnum;
  write_pg_cmd.dest = FROM_CTRL + DEST_EXPT;
  write_pg_cmd.cmd = BOOTLOADER_JUMP;
  write_pg_cmd.cmd_len = 0;
  expt_send_cmd(&write_pg_cmd);
}

void expt_set_time_utc(uint8_t *time_date) {
  openlst_cmd time_cmd;
  time_cmd.hwid = HWID;
  time_cmd.seqnum = TRANSLATE_SEQNUM(libartibeus_msg_id); 
  expt_msg_id_pending = time_cmd.seqnum;
  time_cmd.dest = FROM_CTRL + DEST_EXPT;
  time_cmd.cmd = SET_TIME_UTC;
  time_cmd.cmd_len = 0x6; // Total of 0xE
  time_cmd.payload = time_date;
  expt_send_cmd(&time_cmd);
}

void expt_set_time(uint8_t *time_date) {
  openlst_cmd time_cmd;
  time_cmd.hwid = HWID;
  // Swap bytes, lsb is first
  time_cmd.seqnum = TRANSLATE_SEQNUM(libartibeus_msg_id);
  expt_msg_id_pending = time_cmd.seqnum;
  time_cmd.dest = FROM_CTRL + DEST_EXPT;
  time_cmd.cmd = SET_TIME;
  int32_t year = (int32_t)time_date[2] + 2000;
  int32_t month = (int32_t)time_date[1];
  int32_t day = (int32_t)time_date[0];

  int32_t hour = (int32_t)time_date[0];
  int32_t minute = (int32_t)time_date[1];
  int32_t second = (int32_t)time_date[2];

  int32_t jd =
   day-32075+1461*(year+4800+(month-14)/12)/4
   +367*(month-2-(month-14)/12*12)/12-3
   *((year+4900+(month-14)/12)/100)/4;
  int32_t sec =
   86400*(jd-2451545)+60*(60*hour+minute)+second-43135-1;
  uint8_t temp_arr[8] = {0};
  memcpy(temp_arr,&sec,4);
  time_cmd.cmd_len = 0x8; // Total of 0xE
  time_cmd.payload = temp_arr;
  expt_send_cmd(&time_cmd);
}

void comm_return_nack(buffer_t *raw_pkt) {
  openlst_cmd ack_cmd;
  ack_cmd.hwid = LIBARTIBEUS_COMM_HWID;
  ack_cmd.seqnum = raw_pkt->pkt.msg[SEQ_NUM_OFFSET];
  ack_cmd.dest = FROM_CTRL + GET_FROM(raw_pkt->pkt.msg[DEST_OFFSET]);
  ack_cmd.cmd = NACK;
  ack_cmd.cmd_len = 0;
  comm_send_cmd(&ack_cmd);
}

void comm_return_ack(buffer_t *raw_pkt) {
  openlst_cmd ack_cmd;
  ack_cmd.hwid = LIBARTIBEUS_COMM_HWID;
  ack_cmd.seqnum = raw_pkt->pkt.msg[SEQ_NUM_OFFSET];
  ack_cmd.dest = FROM_CTRL + GET_FROM(raw_pkt->pkt.msg[DEST_OFFSET]);
  ack_cmd.cmd = ACK;
  ack_cmd.cmd_len = 0;
  comm_send_cmd(&ack_cmd);
}

void expt_return_ack(buffer_t *raw_pkt) {
  openlst_cmd ack_cmd;
  ack_cmd.hwid = LIBARTIBEUS_EXPT_HWID;
  ack_cmd.seqnum = raw_pkt->pkt.msg[SEQ_NUM_OFFSET];
  ack_cmd.dest = FROM_CTRL + GET_FROM(raw_pkt->pkt.msg[DEST_OFFSET]);
  ack_cmd.cmd = ACK;
  ack_cmd.cmd_len = 0;
  expt_send_cmd(&ack_cmd);
}

void expt_return_nack(buffer_t *raw_pkt) {
  openlst_cmd ack_cmd;
  ack_cmd.hwid = LIBARTIBEUS_EXPT_HWID;
  ack_cmd.seqnum = raw_pkt->pkt.msg[SEQ_NUM_OFFSET];
  ack_cmd.dest = FROM_CTRL + GET_FROM(raw_pkt->pkt.msg[DEST_OFFSET]);
  ack_cmd.cmd = NACK;
  ack_cmd.cmd_len = 0;
  expt_send_cmd(&ack_cmd);
}

void comm_return_telem(buffer_t *raw_pkt) {
  uint8_t *telem = artibeus_set_telem_pkt(artibeus_latest_telem_pkt);
  openlst_cmd telem_cmd;
  telem_cmd.hwid = LIBARTIBEUS_COMM_HWID;
  telem_cmd.seqnum = raw_pkt->pkt.msg[SEQ_NUM_OFFSET];
  telem_cmd.dest = FROM_CTRL + GET_FROM(raw_pkt->pkt.msg[DEST_OFFSET]);
  telem_cmd.cmd = TELEM;
  telem_cmd.cmd_len = 1 + ARTIBEUS_FULL_TELEM_SIZE;
  telem_cmd.payload = telem;
  comm_send_cmd(&telem_cmd);
}

int update_rf_kill_count(buffer_t *raw_pkt) {
  int got_key = 1;
  for (int i = 0; i < KILL_KEYS_LEN; i++) {
    if (raw_pkt->pkt.msg[SUB_CMD_OFFSET + 1 + i] != RF_KILL_KEYS[i]) {
      got_key = 0;
      break;
    }
  }
  if (got_key) {
    rf_kill_count++;
  }
  if (rf_kill_count > MAX_KILL_COUNT) {
    libartibeus_rf_dead = 1;
  }
  return rf_kill_count;
}

void update_score(buffer_t *raw_pkt) {
  //TODO need to make this resilient to failures
  for (int i = 0; i < SCORE_LEN; i++) {
    score_msg[i] = raw_pkt->pkt.msg[SUB_CMD_OFFSET+1+i];
  }
  return;
}

#if 0
#define  BYTES_PER_CMD 129
const uint8_t SUBPAGE_00[BYTES_PER_CMD] = {
 0x00,
 0x00, 0x00, 0x04, 0x20,
 0xd5, 0x84, 0x00, 0x08,
 0xd3, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00,
 0xd3, 0x84, 0x00, 0x08,
 0xd3, 0x84, 0x00, 0x08,
 0x00, 0x00, 0x00, 0x00,
 0xd3, 0x84, 0x00, 0x08,
 0xd3, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08
};
const uint8_t SUBPAGE_01[BYTES_PER_CMD] = {
 0x01,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08
};
const uint8_t SUBPAGE_02[BYTES_PER_CMD] = {
 0x02,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08
};
const uint8_t SUBPAGE_03[BYTES_PER_CMD] = {
 0x03,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0xd1, 0x84, 0x00, 0x08,
 0x13, 0xb5, 0x02, 0x20,
 0x00, 0xf0, 0x18, 0xf9,
 0x02, 0x20, 0x00, 0xf0,
 0xe9, 0xf8, 0x01, 0x20,
 0x00, 0xf0, 0x3e, 0xf9,
 0x00, 0x20, 0x00, 0xf0,
 0x59, 0xf9, 0x04, 0x20,
 0x00, 0xf0, 0x4c, 0xf9,
 0x00, 0x20, 0x00, 0x24,
 0x00, 0xf0, 0x3e, 0xf9,
 0x00, 0xf0, 0x5e, 0xf8,
 0x04, 0x20, 0x00, 0xf0,
 0x63, 0xf8, 0x00, 0xf0,
 0x6b, 0xf8, 0x00, 0xf0,
 0x71, 0xf8, 0x23, 0x46,
 0x28, 0x22, 0x04, 0x21,
 0xcd, 0xe9, 0x00, 0x44,
 0x02, 0x20, 0x00, 0xf0,
 0x4b, 0xf9, 0x20, 0x46,
 0x00, 0xf0, 0xf4, 0xf8,
 0x20, 0x46, 0x00, 0xf0
};
const uint8_t SUBPAGE_04[BYTES_PER_CMD] = {
 0x04,
 0xc5, 0xf8, 0x03, 0x20,
 0x00, 0xf0, 0x1a, 0xf9,
 0x20, 0x46, 0x00, 0xf0,
 0xc7, 0xf8, 0x1a, 0x4a,
 0x1a, 0x4b, 0x13, 0x60,
 0x1a, 0x4a, 0x1b, 0x49,
 0x11, 0x60, 0x1b, 0x4a,
 0x40, 0xf6, 0x82, 0x10,
 0x13, 0x60, 0x00, 0xf0,
 0x47, 0xf9, 0x22, 0x46,
 0x18, 0x48, 0x4f, 0xf4,
 0x80, 0x63, 0x01, 0x21,
 0x00, 0xf0, 0x60, 0xf8,
 0x22, 0x46, 0x4f, 0xf4,
 0x80, 0x53, 0x14, 0x48,
 0x13, 0x4c, 0x01, 0x21,
 0x00, 0xf0, 0x58, 0xf8,
 0x11, 0x48, 0x4f, 0xf4,
 0x80, 0x61, 0x00, 0xf0,
 0x45, 0xf8, 0x0f, 0x48,
 0x4f, 0xf4, 0x80, 0x51,
 0x00, 0xf0, 0x42, 0xf8,
 0x0d, 0x4b, 0x00, 0xbf,
 0x01, 0x3b, 0xfc, 0xd1,
 0x20, 0x46, 0x4f, 0xf4,
 0x80, 0x61, 0x00, 0xf0,
 0x3c, 0xf8, 0x4f, 0xf4,
 0x80, 0x51, 0x20, 0x46,
 0x00, 0xf0, 0x37, 0xf8,
 0xf0, 0xe7, 0x00, 0xbf,
 0x00, 0x00, 0x00, 0x20,
 0x00, 0xb4, 0xc4, 0x04
};
const uint8_t SUBPAGE_05[BYTES_PER_CMD] = {
 0x05,
 0x04, 0x00, 0x00, 0x20,
 0x00, 0x5a, 0x62, 0x02,
 0x08, 0x00, 0x00, 0x20,
 0x00, 0x08, 0x00, 0x48,
 0x00, 0x09, 0x3d, 0x00,
 0x02, 0x4a, 0x13, 0x68,
 0x43, 0xf4, 0x80, 0x73,
 0x13, 0x60, 0x70, 0x47,
 0x00, 0x20, 0x02, 0x40,
 0x03, 0x4a, 0x13, 0x68,
 0x23, 0xf0, 0x07, 0x03,
 0x18, 0x43, 0x10, 0x60,
 0x70, 0x47, 0x00, 0xbf,
 0x00, 0x20, 0x02, 0x40,
 0x02, 0x4a, 0x13, 0x68,
 0x43, 0xf4, 0x80, 0x63,
 0x13, 0x60, 0x70, 0x47,
 0x00, 0x20, 0x02, 0x40,
 0x02, 0x4a, 0x13, 0x68,
 0x43, 0xf4, 0x00, 0x73,
 0x13, 0x60, 0x70, 0x47,
 0x00, 0x20, 0x02, 0x40,
 0x81, 0x61, 0x70, 0x47,
 0x09, 0x04, 0x81, 0x61,
 0x70, 0x47, 0x43, 0x69,
 0x01, 0xea, 0x03, 0x02,
 0x21, 0xea, 0x03, 0x01,
 0x41, 0xea, 0x02, 0x41,
 0x81, 0x61, 0x70, 0x47,
 0x2d, 0xe9, 0xf0, 0x41,
 0x05, 0x68, 0xc4, 0x68,
 0x00, 0x26, 0x4f, 0xf0
};
const uint8_t SUBPAGE_06[BYTES_PER_CMD] = {
 0x06,
 0x03, 0x0e, 0x43, 0xfa,
 0x06, 0xf7, 0xff, 0x07,
 0x0d, 0xd5, 0x77, 0x00,
 0x0e, 0xfa, 0x07, 0xfc,
 0x01, 0xfa, 0x07, 0xf8,
 0x25, 0xea, 0x0c, 0x05,
 0x24, 0xea, 0x0c, 0x04,
 0x02, 0xfa, 0x07, 0xf7,
 0x48, 0xea, 0x05, 0x05,
 0x3c, 0x43, 0x01, 0x36,
 0x10, 0x2e, 0xea, 0xd1,
 0x05, 0x60, 0xc4, 0x60,
 0xbd, 0xe8, 0xf0, 0x81,
 0x06, 0x28, 0x1f, 0xd8,
 0xdf, 0xe8, 0x00, 0xf0,
 0x04, 0x09, 0x0e, 0x13,
 0x18, 0x1a, 0x1c, 0x00,
 0x0d, 0x4b, 0x18, 0x68,
 0xc0, 0xf3, 0x40, 0x60,
 0x70, 0x47, 0x0b, 0x4b,
 0x18, 0x68, 0xc0, 0xf3,
 0x40, 0x40, 0x70, 0x47,
 0x08, 0x4b, 0x18, 0x68,
 0xc0, 0xf3, 0x80, 0x20,
 0x70, 0x47, 0x06, 0x4b,
 0x18, 0x68, 0xc0, 0xf3,
 0x40, 0x00, 0x70, 0x47,
 0x04, 0x4b, 0xf9, 0xe7,
 0x04, 0x4b, 0xf7, 0xe7,
 0x04, 0x4b, 0xf5, 0xe7,
 0x00, 0x20, 0x70, 0x47,
 0x00, 0x10, 0x02, 0x40
};
const uint8_t SUBPAGE_07[BYTES_PER_CMD] = {
 0x07,
 0x90, 0x10, 0x02, 0x40,
 0x94, 0x10, 0x02, 0x40,
 0x98, 0x10, 0x02, 0x40,
 0x08, 0xb5, 0x02, 0x46,
 0x10, 0x46, 0xff, 0xf7,
 0xcf, 0xff, 0x00, 0x28,
 0xfa, 0xd0, 0x08, 0xbd,
 0x03, 0x28, 0x08, 0xd8,
 0xdf, 0xe8, 0x00, 0xf0,
 0x16, 0x0f, 0x08, 0x02,
 0x0d, 0x4a, 0x13, 0x68,
 0x13, 0xf0, 0x0c, 0x0f,
 0xfb, 0xd1, 0x70, 0x47,
 0x0a, 0x4a, 0x13, 0x68,
 0xc3, 0xf3, 0x81, 0x03,
 0x01, 0x2b, 0xfa, 0xd1,
 0x70, 0x47, 0x07, 0x4a,
 0x13, 0x68, 0xc3, 0xf3,
 0x81, 0x03, 0x02, 0x2b,
 0xfa, 0xd1, 0x70, 0x47,
 0x03, 0x4a, 0x13, 0x68,
 0xc3, 0xf3, 0x81, 0x03,
 0x03, 0x2b, 0xfa, 0xd1,
 0x70, 0x47, 0x00, 0xbf,
 0x08, 0x10, 0x02, 0x40,
 0x06, 0x28, 0x0a, 0xd8,
 0xdf, 0xe8, 0x00, 0xf0,
 0x04, 0x0a, 0x0f, 0x14,
 0x19, 0x1b, 0x1d, 0x00,
 0x0d, 0x4a, 0x13, 0x68,
 0x43, 0xf0, 0x80, 0x73,
 0x13, 0x60, 0x70, 0x47
};
const uint8_t SUBPAGE_08[BYTES_PER_CMD] = {
 0x08,
 0x0a, 0x4a, 0x13, 0x68,
 0x43, 0xf4, 0x80, 0x33,
 0xf8, 0xe7, 0x08, 0x4a,
 0x13, 0x68, 0x43, 0xf4,
 0x80, 0x73, 0xf3, 0xe7,
 0x05, 0x4a, 0x13, 0x68,
 0x43, 0xf0, 0x01, 0x03,
 0xee, 0xe7, 0x04, 0x4a,
 0xf9, 0xe7, 0x04, 0x4a,
 0xf7, 0xe7, 0x04, 0x4a,
 0xf5, 0xe7, 0x00, 0xbf,
 0x00, 0x10, 0x02, 0x40,
 0x90, 0x10, 0x02, 0x40,
 0x94, 0x10, 0x02, 0x40,
 0x98, 0x10, 0x02, 0x40,
 0x03, 0x4a, 0x13, 0x68,
 0x23, 0xf0, 0x03, 0x03,
 0x18, 0x43, 0x10, 0x60,
 0x70, 0x47, 0x00, 0xbf,
 0x08, 0x10, 0x02, 0x40,
 0x03, 0x4a, 0x13, 0x68,
 0x23, 0xf4, 0x60, 0x53,
 0x43, 0xea, 0xc0, 0x20,
 0x10, 0x60, 0x70, 0x47,
 0x08, 0x10, 0x02, 0x40,
 0x03, 0x4a, 0x13, 0x68,
 0x23, 0xf4, 0xe0, 0x63,
 0x43, 0xea, 0x00, 0x20,
 0x10, 0x60, 0x70, 0x47,
 0x08, 0x10, 0x02, 0x40,
 0x03, 0x4a, 0x13, 0x68,
 0x23, 0xf0, 0xf0, 0x03
};
const uint8_t SUBPAGE_09[BYTES_PER_CMD] = {
 0x09,
 0x43, 0xea, 0x00, 0x10,
 0x10, 0x60, 0x70, 0x47,
 0x08, 0x10, 0x02, 0x40,
 0x10, 0xb5, 0x03, 0x9c,
 0x01, 0x39, 0x64, 0x06,
 0x44, 0xea, 0x01, 0x14,
 0x20, 0x43, 0x02, 0x9c,
 0x40, 0xea, 0x44, 0x50,
 0x03, 0x43, 0x43, 0xea,
 0x02, 0x22, 0x02, 0x4b,
 0x42, 0xf0, 0x80, 0x72,
 0x1a, 0x60, 0x10, 0xbd,
 0x0c, 0x10, 0x02, 0x40,
 0x43, 0x09, 0x03, 0xf1,
 0x80, 0x43, 0x03, 0xf5,
 0x04, 0x33, 0x00, 0xf0,
 0x1f, 0x00, 0x19, 0x68,
 0x01, 0x22, 0x02, 0xfa,
 0x00, 0xf0, 0x08, 0x43,
 0x18, 0x60, 0x70, 0x47,
 0xfe, 0xe7, 0x70, 0x47,
 0x38, 0xb5, 0x1a, 0x4a,
 0x1a, 0x4b, 0x1b, 0x49,
 0x8b, 0x42, 0x1b, 0xd3,
 0x1a, 0x4a, 0x00, 0x21,
 0x93, 0x42, 0x1c, 0xd3,
 0x19, 0x4a, 0x1a, 0x4c,
 0x13, 0x68, 0x1a, 0x4d,
 0x43, 0xf4, 0x00, 0x73,
 0x13, 0x60, 0x53, 0x6f,
 0x43, 0xf4, 0x70, 0x03,
 0x53, 0x67, 0xac, 0x42
};
const uint8_t SUBPAGE_0A[BYTES_PER_CMD] = {
 0x0A,
 0x12, 0xd3, 0x16, 0x4c,
 0x16, 0x4d, 0xac, 0x42,
 0x12, 0xd3, 0xff, 0xf7,
 0x4f, 0xfe, 0x15, 0x4c,
 0x15, 0x4d, 0xac, 0x42,
 0x10, 0xd3, 0x38, 0xbd,
 0x52, 0xf8, 0x04, 0x0b,
 0x43, 0xf8, 0x04, 0x0b,
 0xdc, 0xe7, 0x43, 0xf8,
 0x04, 0x1b, 0xdd, 0xe7,
 0x54, 0xf8, 0x04, 0x3b,
 0x98, 0x47, 0xe6, 0xe7,
 0x54, 0xf8, 0x04, 0x3b,
 0x98, 0x47, 0xe6, 0xe7,
 0x54, 0xf8, 0x04, 0x3b,
 0x98, 0x47, 0xe8, 0xe7,
 0x6c, 0x85, 0x00, 0x08,
 0x00, 0x00, 0x00, 0x20,
 0x0c, 0x00, 0x00, 0x20,
 0x0c, 0x00, 0x00, 0x20,
 0x14, 0xed, 0x00, 0xe0,
 0x6c, 0x85, 0x00, 0x08,
 0x6c, 0x85, 0x00, 0x08,
 0x6c, 0x85, 0x00, 0x08,
 0x6c, 0x85, 0x00, 0x08,
 0x6c, 0x85, 0x00, 0x08,
 0x6c, 0x85, 0x00, 0x08,
 0x00, 0x09, 0x3d, 0x00,
 0x00, 0x09, 0x3d, 0x00,
 0x00, 0x09, 0x3d, 0x00,
 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff
};

void expt_write_page(uint8_t *data) {
  openlst_cmd write_pg_cmd;
  write_pg_cmd.hwid = HWID;
  //TODO make this a global counter
  write_pg_cmd.seqnum = 0x0000;
  write_pg_cmd.dest = LST;
  write_pg_cmd.cmd = BOOTLOADER_WRITE_PAGE;
  write_pg_cmd.cmd_len = 0x81;
  write_pg_cmd.payload = data;
  expt_send_cmd(&write_pg_cmd);
  return;
}


void expt_write_program() {
  char msg[0x7];
  unsigned count;
  // Start
  uartlink_open_tx(1);
  // Write page
  expt_write_page(SUBPAGE_00);
  uartlink_close(1);
  uartlink_open_rx(1);
  __delay_cycles(8000000);
  count = uartlink_receive_basic(1,msg ,0x7);
  if (count == 0x7) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  }
  // End
  // Start
  uartlink_close(1);
  uartlink_open_tx(1);
  // Write page
  expt_write_page(SUBPAGE_01);
  uartlink_close(1);
  uartlink_open_rx(1);
  count = uartlink_receive_basic(1,msg ,0x7);
  if (count == 0x7) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  }
  // End
  __delay_cycles(8000000);
  // Start
  uartlink_close(1);
  uartlink_open_tx(1);
  // Write page
  expt_write_page(SUBPAGE_02);
  uartlink_close(1);
  uartlink_open_rx(1);
  count = uartlink_receive_basic(1,msg ,0x7);
  if (count == 0x7) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  }
  // End
  __delay_cycles(8000000);
  // Start
  uartlink_close(1);
  uartlink_open_tx(1);
  // Write page
  expt_write_page(SUBPAGE_03);
  uartlink_close(1);
  uartlink_open_rx(1);
  count = uartlink_receive_basic(1,msg ,0x7);
  if (count == 0x7) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  }
  // End
  __delay_cycles(8000000);
  // Start
  uartlink_close(1);
  uartlink_open_tx(1);
  // Write page
  expt_write_page(SUBPAGE_04);
  uartlink_close(1);
  uartlink_open_rx(1);
  count = uartlink_receive_basic(1,msg ,0x7);
  if (count == 0x7) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  }
  // End
  __delay_cycles(8000000);
  // Start
  uartlink_close(1);
  uartlink_open_tx(1);
  // Write page
  expt_write_page(SUBPAGE_05);
  uartlink_close(1);
  uartlink_open_rx(1);
  count = uartlink_receive_basic(1,msg ,0x7);
  if (count == 0x7) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  }
  // End
  __delay_cycles(8000000);
  // Start
  uartlink_close(1);
  uartlink_open_tx(1);
  // Write page
  expt_write_page(SUBPAGE_06);
  uartlink_close(1);
  uartlink_open_rx(1);
  count = uartlink_receive_basic(1,msg ,0x7);
  if (count == 0x7) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  }
  // End
  __delay_cycles(8000000);
  // Start
  uartlink_close(1);
  uartlink_open_tx(1);
  // Write page
  expt_write_page(SUBPAGE_07);
  uartlink_close(1);
  uartlink_open_rx(1);
  count = uartlink_receive_basic(1,msg ,0x7);
  if (count == 0x7) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  }
  // End
  __delay_cycles(8000000);
  // Start
  uartlink_close(1);
  uartlink_open_tx(1);
  // Write page
  expt_write_page(SUBPAGE_08);
  uartlink_close(1);
  uartlink_open_rx(1);
  count = uartlink_receive_basic(1,msg ,0x7);
  if (count == 0x7) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  }
  // End
  __delay_cycles(8000000);
  // Start
  uartlink_close(1);
  uartlink_open_tx(1);
  // Write page
  expt_write_page(SUBPAGE_09);
  uartlink_close(1);
  uartlink_open_rx(1);
  count = uartlink_receive_basic(1,msg ,0x7);
  if (count == 0x7) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  }
  // End
  __delay_cycles(8000000);
  // Start
  uartlink_close(1);
  uartlink_open_tx(1);
  // Write page
  expt_write_page(SUBPAGE_0A);
  uartlink_close(1);
  uartlink_open_rx(1);
  count = uartlink_receive_basic(1,msg ,0x7);
  if (count == 0x7) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  }
  // End
  __delay_cycles(8000000);
  uartlink_close(1);
  uartlink_open_tx(1);
  expt_write_jump();
  uartlink_close(1);
  uartlink_open_rx(1);
  count = uartlink_receive_basic(1,msg ,0x7);
  if (count == 0x7) {
    P1OUT |= BIT1;
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
  }
  uartlink_close(1);
  return;
}



#endif

