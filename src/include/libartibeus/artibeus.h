#ifndef _ARTIBEUS_H_
#define _ARTIBEUS_H_

#if BOARD_MAJOR == 0

// Enable/disable pins for other boards and modules
#define LIBARTIBEUS_PORT_EXP_EN 4
#define LIBARTIBEUS_PIN_EXP_EN 0

#define LIBARTIBEUS_PORT_COMM_EN 7
#define LIBARTIBEUS_PIN_COMM_EN 4

#define LIBARTIBEUS_PORT_GNSS_EN 2
#define LIBARTIBEUS_PIN_GNSS_EN 7

// Debug pins!
#define LIBARTIBEUS_PORT_DBG0 1
#define LIBARTIBEUS_PIN_DBG0 0

#define LIBARTIBEUS_PORT_DBG1 1
#define LIBARTIBEUS_PIN_DBG1 1

#define LIBARTIBEUS_PORT_DBG2 1
#define LIBARTIBEUS_PIN_DBG2 2

// GNSS nRST PIN
#define LIBARTIBEUS_PORT_GNSS_nRST 2
#define LIBARTIBEUS_PIN_GNSS_nRST 3

// Burn wire pin
#define LIBARTIBEUS_PORT_BURN_WIRE 1
#define LIBARTIBEUS_PIN_BURN_WIRE 3
//TODO find a more elegant way to check that we've burned through the wire
#define LIBARTIBEUS_BURN_WIRE_CYCLES 16000000

// Watchdog configuration
#define CONFIG_WDT_BITS WATCHDOG_BITS(WATCHDOG_CLOCK, WATCHDOG_INTERVAL)

#endif // VERSION

// Couple of definitions to enable/disable the other boards
#define EXP_ENABLE do{\
  GPIO(LIBARTIBEUS_PORT_EXP_EN, DIR) |= BIT(LIBARTIBEUS_PIN_EXP_EN); \
  GPIO(LIBARTIBEUS_PORT_EXP_EN, OUT) |= BIT(LIBARTIBEUS_PIN_EXP_EN);} while(0); \

#define EXP_DISABLE do{\
  GPIO(LIBARTIBEUS_PORT_EXP_EN, OUT) &= ~BIT(LIBARTIBEUS_PIN_EXP_EN);} while(0);

#define GNSS_ENABLE do{\
  GPIO(LIBARTIBEUS_PORT_GNSS_EN, DIR) |= BIT(LIBARTIBEUS_PIN_GNSS_EN); \
  GPIO(LIBARTIBEUS_PORT_GNSS_EN, OUT) |= BIT(LIBARTIBEUS_PIN_GNSS_EN);} while(0); \

#define GNSS_DISABLE do{\
  GPIO(LIBARTIBEUS_PORT_GNSS_EN, OUT) &= ~BIT(LIBARTIBEUS_PIN_GNSS_EN);} while(0);

#define COMM_ENABLE do{\
  GPIO(LIBARTIBEUS_PORT_COMM_EN, DIR) |= BIT(LIBARTIBEUS_PIN_COMM_EN); \
  GPIO(LIBARTIBEUS_PORT_COMM_EN, OUT) |= BIT(LIBARTIBEUS_PIN_COMM_EN);} while(0); \

#define COMM_DISABLE do{\
  GPIO(LIBARTIBEUS_PORT_COMM_EN, OUT) &= ~BIT(LIBARTIBEUS_PIN_COMM_EN);} while(0);

// Handlful of extra defines

#define  RESET_GNSS do{\
  GPIO(LIBARTIBEUS_PORT_GNSS_nRST, DIR) |= BIT(LIBARTIBEUS_PIN_GNSS_nRST);\
  GPIO(LIBARTIBEUS_PORT_GNSS_nRST, OUT) &= ~BIT(LIBARTIBEUS_PIN_GNSS_nRST);\
  GPIO(LIBARTIBEUS_PORT_GNSS_nRST, DIR) &= ~BIT(LIBARTIBEUS_PIN_GNSS_nRST);\
}while(0);


#ifndef LIBARTIBEUS_CTRL_HWID
#error "No HWID defined! Set LIBARTIBEUS_CTRL_HWID"
#else
#define HWID_CTRL LIBARTIBEUS_CTRL_HWID
#endif

// This needs to be less than 1 word to guarantee that we can write to it in one
// go. We're using it as part of our recovery strategy
typedef enum artibeus_mode_ {
  ADC,
  IMU,
  GNSS,
  COMM,
  EXPT,
  XFER,
  RECORD_TELEM,
  GET_UART0,
  GET_UART1,
  GET_UART2,
  CHECK_GNSS_TIMER,
  CHECK_ASCII_TIMER,
  SLEEP
} artibeus_mode;

void artibeus_init();
void artibeus_first_init();


#endif // _ARTIBEUS_H_
