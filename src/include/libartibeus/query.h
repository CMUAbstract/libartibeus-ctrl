#ifndef _LIBARTIBEUS_QUERY_H_
#define _LIBARTIBEUS_QUERY_H_

#include <msp430.h>
#include <stdint.h>
#include <libmsp/mem.h>
#include "comm.h"

// Size definitions for different telemetry components
#define ARTIBEUS_AVG_IMU_SIZE 3
#define ARTIBEUS_AVG_PWR_SIZE 4
#define ARTIBEUS_GPS_SIZE 12
#define ARTIBEUS_TIME_SIZE 3
#define ARTIBEUS_DATE_SIZE 3
#define ARTIBEUS_TIME_DATE_SIZE 6
#define ARTIBEUS_IMU_SIZE 9
#define ARTIBEUS_PWR_SIZE 4
#define ARTIBEUS_FULL_TELEM_SIZE 44
#define ARTIBEUS_MAX_ASCII_SIZE 32
#define ARTIBEUS_ASCII_ENTRIES 32

// Struct defs
typedef uint8_t artibeus_telem_t[ARTIBEUS_FULL_TELEM_SIZE+1];
typedef uint8_t artibeus_ascii_t[ARTIBEUS_MAX_ASCII_SIZE];

// Structures
extern __nv artibeus_telem_t artibeus_latest_telem_pkt;
extern __nv artibeus_ascii_t expt_ascii_buffer[ARTIBEUS_ASCII_ENTRIES];
extern uint8_t __nv expt_ascii_tail;
// Functions for accessing the query-able data

/*-----------------------------------------------------*/
// Safe from power interruptions

// Returns pointer to latest imu data
int16_t * artibeus_get_imu();
// Returns pointer to avg xl data 
int16_t * artibeus_get_xl();
// Returns pointer to avg gyro data 
int16_t * artibeus_get_g();
// Returns pointer to avg magnetometer data 
int16_t * artibeus_get_m();
// Returns pointer to latest power data
uint16_t* artibeus_get_pwr();
// Returns pointer to latest gps location
uint8_t* artibeus_get_gps();
// Returns pointer to latest time
uint8_t* artibeus_get_time();
// Returns pointer to latest date
uint8_t* artibeus_get_date();
// Sets accel. values. Updates avg and latest imu data
void artibeus_set_xl(int16_t x, int16_t y, int16_t z);
// Sets gyro values. Updates avg and latest imu data
void artibeus_set_g(int16_t x, int16_t y, int16_t z);
// Sets mag. values. Updates avg and latest imu data
void artibeus_set_m(int16_t x, int16_t y, int16_t z);
// Sets power values. Updates avgs and latest
void artibeus_set_pwr(uint16_t*);
// Sets gps values.
void artibeus_set_gps(uint8_t*);
// Sets time
void artibeus_set_time(uint8_t*);
// Sets date
void artibeus_set_date(uint8_t*);


// Pushes to ascii buffer
uint8_t artibeus_push_ascii_pkt(buffer_t *buff);
// Separately updates pointers
void artibeus_pop_update_ascii_ptrs();
// Checks if anything is in buffer
int artibeus_ascii_is_empty();
/*--------------------------------------------------------*/
// Not safe from power interruptions

// Sets latest telemetry packet and returns pointer
// Does *not* double buffer, it's susceptible to atomicity violations if you
// don't double buffer the pointer you pass in
uint8_t * artibeus_set_telem_pkt(uint8_t *);

// Pops from ascii buffer
// Needs to be paired with pop_update_ascii_ptrs to commit change
uint8_t *artibeus_pop_ascii_pkt();


#endif
