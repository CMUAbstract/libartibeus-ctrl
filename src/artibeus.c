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
#include "artibeus.h"

EUSCI_B_I2C_initMasterParam params = {
	.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK,
  //.dataRate = EUSCI_B_I2C_SET_DATA_RATE_400KBPS,
  .dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS,
	.byteCounterThreshold = 0,
  .autoSTOPGeneration = EUSCI_B_I2C_NO_AUTO_STOP
};


void artibeus_init() {
#ifdef CONFIG_WATCHDOG
  msp_watchdog_enable(CONFIG_WDT_BITS);
#else
  msp_watchdog_disable();
#endif
  msp_gpio_unlock();
  __enable_interrupt();
  msp_clock_setup();
#if defined(CONSOLE) && ~defined(LIBARTIBEUS_RUN_UARTLINKS)
  INIT_CONSOLE();
#endif
#ifdef LIBARTIBEUS_RUN_UARTLINKS
  uartlink_open(0);
  uartlink_open(1);
  uartlink_open(2);

#endif
#ifdef LIBARTIBEUS_RUN_I2C
  params.i2cClk = CS_getSMCLK();
	GPIO_setAsPeripheralModuleFunctionInputPin(
			GPIO_PORT_P1,
			GPIO_PIN6 + GPIO_PIN7,
			GPIO_SECONDARY_MODULE_FUNCTION
			);
	EUSCI_B_I2C_initMaster(EUSCI_B0_BASE, &params);
#endif
  __enable_interrupt();
}

static __nv uint8_t libartibeus_done_burn = 0;

void artibeus_burn_wire() {
  if (libartibeus_done_burn) {
    return;
  }
  GPIO(LIBARTIBEUS_PORT_BURN_WIRE, DIR) |= LIBARTIBEUS_PIN_BURN_WIRE;
  GPIO(LIBARTIBEUS_PORT_BURN_WIRE, OUT) |= LIBARTIBEUS_PIN_BURN_WIRE;
  __delay_cycles(LIBARTIBEUS_BURN_WIRE_CYCLES);
  libartibeus_done_burn = 1;
  return;
}


void artibeus_first_init() {
#ifdef CONFIG_WATCHDOG
  msp_watchdog_enable(CONFIG_WDT_BITS);
#else
  msp_watchdog_disable();
#endif
  msp_gpio_unlock();
  __enable_interrupt();
  msp_clock_setup();
  artibeus_burn_wire();
#if defined(CONSOLE) && ~defined(LIBARTIBEUS_RUN_UARTLINKS)
  INIT_CONSOLE();
#endif
#ifdef LIBARTIBEUS_RUN_UARTLINKS
  uartlink_open(0);
  uartlink_open(1);
  uartlink_open(2);
#endif
#ifdef LIBARTIBEUS_RUN_I2C
  params.i2cClk = CS_getSMCLK();
	GPIO_setAsPeripheralModuleFunctionInputPin(
			GPIO_PORT_P1,
			GPIO_PIN6 + GPIO_PIN7,
			GPIO_SECONDARY_MODULE_FUNCTION
			);
	EUSCI_B_I2C_initMaster(EUSCI_B0_BASE, &params);
#endif
  __enable_interrupt();
}
