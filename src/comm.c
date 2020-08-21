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


void  comm_send_cmd(openlst_cmd *pkt) {
    uint8_t payload[OPENLST_PAYLOAD_LEN];
    // Set hwid
    payload[0] = pkt->hwid >> 8;
    payload[1] = pkt->hwid & 0xFF;
    // Set seqnum
    payload[2] = pkt->seqnum >> 8;
    payload[3] = pkt->seqnum & 0xFF;
    // Dest
    payload[4] = pkt->dest;
    // Cmd
    payload[5] = pkt->cmd;
    for (size_t i = 0; i < pkt->command_len && i < OPENLST_PAYLOAD_LEN - 6; i++) {
      payload[i + 6] = pkt->payload[i];
    }
    uartlink_send_basic(LIBMSPUARTLINK0_UART_IDX, payload, 6 + pkt->command_len);
    return;
}

unsigned comm_ack_check() {
  openlst_cmd ack_cmd;
  ack_cmd.hwid = 0x0001;
  //TODO make this a global counter
  ack_cmd.seqnum = 0x0000;
  //TODO figure out what this should be!!
  ack_cmd.dest = LST_RELAY;
  ack_cmd.cmd = ACK;
  ack_cmd.command_len = 0;
  comm_send_cmd(&ack_cmd);
  //TODO figure out how many bytes OpenLST protocol will return
  uint8_t response[8];
  unsigned count;
  while(!count) {
    count = uartlink_receive_basic(LIBMSPUARTLINK0_UART_IDX,response,8);
  }
  //TODO implement a function that checks for ACK vs NACK
  return count;
}
