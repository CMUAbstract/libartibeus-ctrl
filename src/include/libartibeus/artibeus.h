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

// Burn wire pin
#define LIBARTIBEUS_PORT_BURN_WIRE 1
#define LIBARTIBEUS_PIN_BURN_WIRE 3
//TODO find a more elegant way to check that we've burned through the wire
#define LIBARTIBEUS_BURN_WIRE_CYCLES 8000000

// Watchdog configuration
#define CONFIG_WDT_BITS WATCHDOG_BITS(WATCHDOG_CLOCK, WATCHDOG_INTERVAL)

#endif // VERSION

// This needs to be less than 1 word to guarantee that we can write to it in one
// go. We're using it as part of our recovery strategy
typedef enum artibeus_mode_ {
  ADC,
  IMU,
  GNSS,
  COMM,
  EXPT,
  XFER
} artibeus_mode;


void artibeus_init();
void artibeus_first_init();


#endif // _ARTIBEUS_H_
