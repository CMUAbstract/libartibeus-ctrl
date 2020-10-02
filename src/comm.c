// C file of all the basic artibeus functions
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
#include "comm.h"
#include "artibeus.h"

#define COMM_TEST_LEN 128

static cmd_pkt comm_msg = {.byte0 = ESP_BYTE0, .byte1 = ESP_BYTE1};
static cmd_pkt comm_resp;

 __nv uint8_t RF_KILL_KEYS[16] =
 {'F','O','R','B','E', 'S','a','v','e','P','A','1', '5' ,'2' ,'1','3',};
 __nv uint8_t EXPT_WAKE_KEYS[8] = {'S','c','o','t','t','y','T','A'};

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

unsigned comm_ack_check() {
  openlst_cmd ack_cmd;
  ack_cmd.hwid = HWID;
  //TODO make this a global counter
  ack_cmd.seqnum = 0x0000;
  //ack_cmd.dest = LST;
  ack_cmd.dest = LST_RELAY;
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


void comm_rf_check() {
  openlst_cmd ack_cmd;
  ack_cmd.hwid = HWID;
  //ack_cmd.hwid = 0x0001;
  //TODO make this a global counter
  ack_cmd.seqnum = 0x0000;
  ack_cmd.dest = LST_RELAY;
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
  ascii_cmd.dest = LST_RELAY;
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
  ack_cmd.dest = LST_RELAY;
  ack_cmd.cmd = EXPT_LISTENING;
  ack_cmd.cmd_len = 1;
  uint8_t msg_test[1];
  ack_cmd.payload = msg_test;
  // magic number
  msg_test[0] = 0x64;
  comm_send_cmd(&ack_cmd);
  return;
}
