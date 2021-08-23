#ifndef _LIBARTIBEUS_QUERY_H_
#define _LIBARTIBEUS_QUERY_H_

#include <msp430.h>
#include <stdint.h>
#include <libmsp/mem.h>

// Size definitions for different telemetry components
#define ARTIBEUS_AVG_IMU_SIZE 3
#define ARTIBEUS_AVG_PWR_SIZE 4
#define ARTIBEUS_GPS_SIZE 12
#define ARTIBEUS_TIME_SIZE 3
#define ARTIBEUS_IMU_SIZE 9
#define ARTIBEUS_PWR_SIZE 4
#define ARTIBEUS_FULL_TELEM_SIZE 41

// Functions for accessing the query-able data

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
uint16_t* artibeus_get_gps();
// Returns pointer to latest time
uint16_t* artibeus_get_time();
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
// Sets latest telemetry packet and returns pointer
uint8_t * artibeus_set_telem_pkt(void);

#endif
