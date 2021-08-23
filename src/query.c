// C file with all of the query-able variable declarations as well as the
// functions to update those variables and functions to pass off
#include "query.h"

// We tack the count on the end of the buffer for convenience
__nv int16_t artibeus_xl_avgs_0[ARTIBEUS_AVG_IMU_SIZE + 1] = {0};
__nv int16_t artibeus_g_avgs_0[ARTIBEUS_AVG_IMU_SIZE + 1] = {0};
__nv int16_t artibeus_m_avgs_0[ARTIBEUS_AVG_IMU_SIZE + 1] = {0};
__nv uint16_t artibeus_pwr_avgs_0[ARTIBEUS_AVG_PWR_SIZE + 1] = {0};
__nv int16_t artibeus_xl_avgs_1[ARTIBEUS_AVG_IMU_SIZE + 1] = {0};
__nv int16_t artibeus_g_avgs_1[ARTIBEUS_AVG_IMU_SIZE + 1] = {0};
__nv int16_t artibeus_m_avgs_1[ARTIBEUS_AVG_IMU_SIZE + 1] = {0};
__nv uint16_t artibeus_pwr_avgs_1[ARTIBEUS_AVG_PWR_SIZE + 1] = {0};

__nv int16_t artibeus_last_imu_0[ARTIBEUS_IMU_SIZE] = {0};
__nv uint16_t artibeus_last_pwr_0[ARTIBEUS_PWR_SIZE] = {0};
__nv uint8_t artibeus_last_gps_0[ARTIBEUS_GPS_SIZE] = {0};
__nv uint8_t artibeus_last_time_0[ARTIBEUS_TIME_SIZE] = {0};
__nv int16_t artibeus_last_imu_1[ARTIBEUS_IMU_SIZE] = {0};
__nv uint16_t artibeus_last_pwr_1[ARTIBEUS_PWR_SIZE] = {0};
__nv uint8_t artibeus_last_gps_1[ARTIBEUS_GPS_SIZE] = {0};
__nv uint8_t artibeus_last_time_1[ARTIBEUS_TIME_SIZE] = {0};


__nv int16_t *artibeus_last_imu = artibeus_last_imu_0;
__nv uint16_t *artibeus_last_pwr = artibeus_last_pwr_0;
__nv int16_t *artibeus_avg_xl = artibeus_xl_avgs_0;
__nv int16_t *artibeus_avg_g = artibeus_g_avgs_0;
__nv int16_t *artibeus_avg_m = artibeus_m_avgs_0;
__nv uint16_t *artibeus_avg_pwr = artibeus_pwr_avgs_0;
__nv int8_t *artibeus_gps = artibeus_last_gps_0;
__nv int8_t *artibeus_time = artibeus_last_time_0;

// This is just nv because that's where it'll end up anyway
__nv uint8_t artibeus_telem_pkt[ARTIBEUS_FULL_TELEM_SIZE];

// Returns pointer to latest imu data
// The format is xlX,xlY,xlZ,gX,gY,gZ,mX,mY,mZ,
int16_t * artibeus_get_imu() {
  return artibeus_last_imu;
}

// Returns pointer to avg xl data 
int16_t * artibeus_get_avg_xl() {
  return artibeus_avg_xl;
}

// Returns pointer to avg gyro data 
int16_t * artibeus_get_avg_g() {
  return artibeus_avg_g;
}

// Returns pointer to avg magnetometer data 
int16_t * artibeus_get_avg_m() {
  return artibeus_avg_m;
}

// Returns pointer to latest power data
uint16_t* artibeus_get_pwr() {
  return artibeus_avg_pwr;
}

// Returns pointer to latest gps location
uint16_t* artibeus_get_gps() {
  return artibeus_gps;
}

// Returns pointer to latest time
uint16_t* artibeus_get_time() {
  return artibeus_time;
}

// Sets the latest xl data and updates avg
void artibeus_set_xl(int16_t x, int16_t y, int16_t z) {
  // Update latest xl values and swap buffer
  int16_t *next_buf = (artibeus_last_imu == artibeus_last_imu_0) ?
                      artibeus_last_imu_1 : artibeus_last_imu_0;
  next_buf[0] = x; next_buf[1] = y; next_buf[2] = z;
  artibeus_last_imu = next_buf;
  // We'll do a rolling average here,
  next_buf = (artibeus_avg_xl == artibeus_xl_avgs_0) ? 
             artibeus_xl_avgs_1 : artibeus_xl_avgs_0;
  uint16_t count = (uint16_t)artibeus_avg_xl[ARTIBEUS_IMU_SIZE];
  // (Old val * count + new_val)/(count + 1)
  int16_t new_val = (artibeus_avg_xl[0] * count + x)/(count + 1);
  // update other buffer
  next_buf[0] = new_val;
  new_val = (artibeus_avg_xl[1] * count + y)/(count + 1);
  next_buf[1] = new_val;
  new_val = (artibeus_avg_xl[2] * count + z)/(count + 1); 
  next_buf[2] = new_val;
  // Update count in buffer
  next_buf[ARTIBEUS_AVG_IMU_SIZE] = count + 1;
  // When finished, switch buffer
  artibeus_avg_xl = next_buf;
}

// Sets the latest gyro data and updates avg
void artibeus_set_g(int16_t x, int16_t y, int16_t z) {
  // Update latest g values and swap buffer
  int16_t *next_buf = (artibeus_last_imu == artibeus_last_imu_0) ?
                      artibeus_last_imu_1 : artibeus_last_imu_0;
  next_buf[3] = x; next_buf[4] = y; next_buf[5] = z;
  artibeus_last_imu = next_buf;
  // We'll do a rolling average here,
  next_buf = (artibeus_avg_g == artibeus_g_avgs_0) ? 
             artibeus_g_avgs_1 : artibeus_g_avgs_0;
  uint16_t count = (uint16_t)artibeus_avg_g[ARTIBEUS_IMU_SIZE];
  // (Old val * count + new_val)/(count + 1)
  int16_t new_val = (artibeus_avg_g[0] * count + x)/(count + 1);
  // update other buffer
  next_buf[0] = new_val;
  new_val = (artibeus_avg_g[1] * count + y)/(count + 1);
  next_buf[1] = new_val;
  new_val = (artibeus_avg_g[2] * count + z)/(count + 1); 
  next_buf[2] = new_val;
  // Update count in buffer
  next_buf[ARTIBEUS_AVG_IMU_SIZE] = count + 1;
  // When finished, switch buffer
  artibeus_avg_g = next_buf;
}

// Sets the latest magnetometer data and updates avg
void artibeus_set_m(int16_t x, int16_t y, int16_t z) {
  // Update latest g values and swap buffer
  int16_t *next_buf = (artibeus_last_imu == artibeus_last_imu_0) ?
                      artibeus_last_imu_1 : artibeus_last_imu_0;
  next_buf[6] = x; next_buf[7] = y; next_buf[8] = z;
  artibeus_last_imu = next_buf;
  // We'll do a rolling average here,
  next_buf = (artibeus_avg_m == artibeus_m_avgs_0) ? 
             artibeus_m_avgs_1 : artibeus_m_avgs_0;
  uint16_t count = (uint16_t)artibeus_avg_m[ARTIBEUS_IMU_SIZE];
  // (Old val * count + new_val)/(count + 1)
  int16_t new_val = (artibeus_avg_m[0] * count + x)/(count + 1);
  // update other buffer
  next_buf[0] = new_val;
  new_val = (artibeus_avg_m[1] * count + y)/(count + 1);
  next_buf[1] = new_val;
  new_val = (artibeus_avg_m[2] * count + z)/(count + 1); 
  next_buf[2] = new_val;
  // Update count in buffer
  next_buf[ARTIBEUS_AVG_IMU_SIZE] = count + 1;
  // When finished, switch buffer
  artibeus_avg_m = next_buf;
}

// Sets the latest power data and updates avg
void artibeus_set_pwr(uint16_t *pwr_vals) {
  // Update latest g values and swap buffer
  int16_t *next_buf = (artibeus_last_pwr == artibeus_last_pwr_0) ?
                      artibeus_last_pwr_1 : artibeus_last_pwr_0;
  // Current set up is vcap, vcap deriv., power in, power in deriv.
  int16_t vcap_der = pwr_vals[0] - artibeus_last_pwr[0];
  int16_t hrv_der = pwr_vals[2] - artibeus_last_pwr[2];
  next_buf[0] = pwr_vals[0];
  next_buf[1] = vcap_der;
  next_buf[2] = pwr_vals[2];
  next_buf[3] = hrv_der;
  artibeus_last_pwr = next_buf;
  // We'll do a rolling average here,
  next_buf = (artibeus_avg_pwr == artibeus_pwr_avgs_0) ? 
             artibeus_pwr_avgs_1 : artibeus_pwr_avgs_0;
  //TODO do something with this casting
  uint16_t count = artibeus_avg_pwr[ARTIBEUS_PWR_SIZE];
  // (Old val * count + new_val)/(count + 1)
  int16_t new_val = (artibeus_avg_pwr[0] * count + pwr_vals[0])/(count + 1);
  // update other buffer
  next_buf[0] = new_val;
  new_val = (artibeus_avg_pwr[1] * count + vcap_der)/(count + 1);
  next_buf[1] = new_val;
  new_val = (artibeus_avg_pwr[2] * count + pwr_vals[1])/(count + 1); 
  next_buf[2] = new_val;
  new_val = (artibeus_avg_pwr[3] * count + hrv_der)/(count + 1); 
  next_buf[3] = new_val;
  // Update count in buffer
  next_buf[ARTIBEUS_AVG_PWR_SIZE] = count + 1;
  // When finished, switch buffer
  artibeus_avg_pwr = next_buf;
}

// Sets the latest gps location
void artibeus_set_gps(uint8_t* new_val) {
  // Just do a double buffer swap
  uint8_t *next_buf = (artibeus_gps == artibeus_last_gps_0) ?
                      artibeus_last_gps_1 : artibeus_last_gps_0;
  for(int i = 0; i < ARTIBEUS_GPS_SIZE; i++) {
    next_buf[i] = new_val[i];
  }
  artibeus_gps = next_buf;
  return;
}

// Sets the latest time
void artibeus_set_time(uint8_t* new_val) {
  // Just do a double buffer swap
  uint8_t *next_buf = (artibeus_time == artibeus_last_time_0) ?
                      artibeus_last_time_1 : artibeus_last_time_0;
  for(int i = 0; i < ARTIBEUS_TIME_SIZE; i++) {
    next_buf[i] = new_val[i];
  }
  artibeus_time = next_buf;
  return;
}

// Just grabs the latest telemetry data available and updates the full packet
// TODO fix the bit banging to be less magic-numbery
// We call this _set_ instead of _get_ even though it returns telemetry because
// it updates a single buffer with the latest telemetry values. You need to
// transmit that buffer atomically with calling this function. So we call it set
// to confer the idea that you're updating memory.
uint8_t * artibeus_set_telem_pkt(void) {
  // Patch in xl
  int16_t *xl = artibeus_get_avg_xl();
  *((int16_t *) (artibeus_telem_pkt + 1)) = xl[0];
  *((int16_t *) (artibeus_telem_pkt + 3)) = xl[1];
  *((int16_t *) (artibeus_telem_pkt + 5)) = xl[2];
  // Patch in g
  int16_t *g = artibeus_get_avg_g();
  *((int16_t *) (artibeus_telem_pkt + 7)) = g[0];
  *((int16_t *) (artibeus_telem_pkt + 9)) = g[1];
  *((int16_t *) (artibeus_telem_pkt + 11)) = g[2];
  // Patch in m
  int16_t *m = artibeus_get_avg_m();
  *((int16_t *) (artibeus_telem_pkt + 13)) = m[0];
  *((int16_t *) (artibeus_telem_pkt + 15)) = m[1];
  *((int16_t *) (artibeus_telem_pkt + 17)) = m[2];
  // Patch in pwr data
  uint16_t *pwr = artibeus_get_pwr();
  *((uint16_t *) (artibeus_telem_pkt + 19)) = pwr[0];
  *((uint16_t *) (artibeus_telem_pkt + 21)) = pwr[1];
  *((uint16_t *) (artibeus_telem_pkt + 23)) = pwr[2];
  *((uint16_t *) (artibeus_telem_pkt + 25)) = pwr[3];
  // Patch in gps data
  int8_t *gps = artibeus_get_gps();
  for (int i = 0; i < ARTIBEUS_GPS_SIZE; i++) {
    *((int8_t *)(artibeus_telem_pkt + 26 + i)) = gps[i];
  }
  // Patch in time
  int8_t *time = artibeus_get_time();
  for (int i = 0; i < ARTIBEUS_TIME_SIZE; i++) {
    *((int8_t *)(artibeus_telem_pkt + 26 + ARTIBEUS_GPS_SIZE + i)) = time[i];
  }
  return artibeus_telem_pkt;
}

