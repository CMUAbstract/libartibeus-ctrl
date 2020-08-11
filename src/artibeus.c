// C file of all the basic artibeus functions


void artibeus_init() {
  msp_watchdog_disable();
  msp_gpio_unlock();
  __enable_interrupt();
  msp_clock_setup();
  INIT_CONSOLE();
}
