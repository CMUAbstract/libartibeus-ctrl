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

#endif // VERSION

void artibeus_init();

#endif // _ARTIBEUS_H_
