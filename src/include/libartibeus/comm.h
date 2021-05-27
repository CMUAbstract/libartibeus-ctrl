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
#define SET_TIME 0x14
#define SCORE 0x57

#define GET_TIME 0x13
#define SET_TIME 0x14

#define BOOTLOADER_PING 0x00
#define BOOTLOADER_ERASE 0x0c
#define BOOTLOADER_WRITE_PAGE 0x02
#define BOOTLOADER_ACK 0x01
#define BOOTLOADER_NACK 0x0f
#define BOOTLOADER_JUMP 0x0b

#define EXPT_LISTENING 0x07
#define EXPT_WAKE 0x27
#define EXPT_DONE 0x28

#define RF_KILL  0x3F

#define ASCII 0x11
#define AES_KEY_SIZE 16

#define OPENLST_MAX_PAYLOAD_LEN 240
#define OPENLST_ERROR 0X00

#define ESP_BYTE0 34
#define ESP_BYTE1 105

#ifndef LIBARTIBEUS_HWID
#error "No HWID defined! Set LIBARTIBEUS_HWID"
#else
#define HWID LIBARTIBEUS_HWID
#endif

#define MAX_KILL_COUNT 5

/*
 * @brief: Describes the program level details for a packet. We'll handle the
 * secret sauce bytes elsewhere
 * @details: hwid is programmed in the bootloader
 *           seqnum increments for each
 *           pkt in a series associated with a single cmd
 *           dest indicates either the radio or rf frontend
 *           cmd is one of the commands listed above
 *           cmd_len is the length in bytes of the command payload NOT INCLUDING
 *           THE COMMAND BYTE
 */
typedef struct openlst_cmd_ {
  uint16_t hwid;
  uint16_t seqnum;
  uint8_t dest;
  uint8_t cmd;
  uint8_t cmd_len;
  uint8_t *payload;
} openlst_cmd;

//TODO: MAKE THE FOLLOWING TYPEDEFS PRIVATE TO COMM.C
/*
 * @brief: struct that drills down into the command header for a packet to make
 * processing packets sane
 */
typedef struct cmd_header_ {
  uint8_t hwid0;
  uint8_t hwid1;
  uint8_t seqnum0;
  uint8_t seqnum1;
  uint8_t dest;
  uint8_t cmd;
} cmd_header;

/*
 * @brief: struct to happily smush together command header and data
 */
 typedef struct __attribute__ ((__packed__)) cmd_msg_ {
   cmd_header header;
   uint8_t msg[OPENLST_MAX_PAYLOAD_LEN];
} cmd_msg;
/*
 * @brief: just a type for when we transfer the cmd into a formatted packet
 * that's ready to be transmitted
 */
typedef struct __attribute__ ((__packed__)) cmd_pkt_ {
  uint8_t byte0;
  uint8_t byte1;
  uint8_t total_len;
  cmd_msg msg;
} cmd_pkt;

extern uint8_t RF_KILL_KEYS[16];
extern uint8_t EXPT_WAKE_KEYS[8];

unsigned comm_ack_check(void);
void comm_rf_check(void);
void comm_transmit_pkt(char *pkt, uint16_t len);
void comm_transmit_ready(void);

unsigned expt_ack_check(void);
void expt_write_program();
void expt_write_jump();
void expt_set_time(uint8_t *);
#endif //_LIBARTIBEUS_COMM_H_
