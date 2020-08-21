#ifndef _LIBARTIBEUS_COMM_H_
#define _LIBARTIBEUS_COMM_H_

// LST commands
#define LST 0x01
#define LST_RELAY 0x11

#define ACK 0x10
#define NACK 0xff
#define REBOOT 0x12

#define GET_CALLSIGN 0x19
#define SET_CALLSIGN 0x1a
#define CALLSIGN 0x1b
#define GET_TELEM 0x17
#define TELEM 0x18

#define GET_TIME 0x13
#define SET_TIME 0x14

#define BOOTLOADER_PING 0x00
#define BOOTLOADER_ERASE 0x0c
#define BOOTLOADER_WRITE_PAGE 0x02
#define BOOTLOADER_ACK 0x01
#define BOOTLOADER_NACK 0x0f

#define ASCII 0x11
#define AES_KEY_SIZE 16

#define OPENLST_PAYLOAD_LEN 32

typedef struct __attribute__((__packed__)) openlst_cmd_ {
  uint16_t hwid;
  uint16_t seqnum;
  uint8_t dest;
  uint8_t command_len;
  uint8_t cmd;
  uint8_t *payload;
} openlst_cmd;

unsigned comm_ack_check(void);

#endif //_LIBARTIBEUS_COMM_H_
