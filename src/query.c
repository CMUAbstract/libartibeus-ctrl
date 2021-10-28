// C file with all of the query-able variable declarations as well as the
// functions to update those variables and functions to pass off
#include "query.h"
#include "comm.h"
#include "artibeus.h"
#include "handle_uarts.h"
#include "backup.h"

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
__nv uint8_t artibeus_last_date_0[ARTIBEUS_DATE_SIZE] = {0};
__nv int16_t artibeus_last_imu_1[ARTIBEUS_IMU_SIZE] = {0};
__nv uint16_t artibeus_last_pwr_1[ARTIBEUS_PWR_SIZE] = {0};
__nv uint8_t artibeus_last_gps_1[ARTIBEUS_GPS_SIZE] = {0};
__nv uint8_t artibeus_last_time_1[ARTIBEUS_TIME_SIZE] = {0};
__nv uint8_t artibeus_last_date_1[ARTIBEUS_DATE_SIZE] = {0};


__nv int16_t *artibeus_last_imu = artibeus_last_imu_0;
__nv uint16_t *artibeus_last_pwr = artibeus_last_pwr_0;
__nv int16_t *artibeus_avg_xl = artibeus_xl_avgs_0;
__nv int16_t *artibeus_avg_g = artibeus_g_avgs_0;
__nv int16_t *artibeus_avg_m = artibeus_m_avgs_0;
__nv uint16_t *artibeus_avg_pwr = artibeus_pwr_avgs_0;
__nv uint8_t *artibeus_gps = artibeus_last_gps_0;
__nv uint8_t *artibeus_time = artibeus_last_time_0;
__nv uint8_t *artibeus_date = artibeus_last_date_0;

// This is just nv because that's where it'll end up anyway
__nv artibeus_telem_t artibeus_latest_telem_pkt;

// Ring buffer for ascii messages collected from experiment board
__nv artibeus_ascii_t expt_ascii_buffer[ARTIBEUS_ASCII_ENTRIES];
__nv uint8_t expt_ascii_tail = 0;
static __nv uint8_t expt_ascii_head = 0;
static __nv uint8_t expt_ascii_full = 0;

__nv artibeus_telem_t telem_buffer[ARTIBEUS_TELEM_BUFF_SIZE]; 
__nv uint8_t telem_buffer_tail = 0;
static __nv uint8_t telem_buffer_head = 0;
static __nv uint8_t telem_buffer_full = 0;


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
uint8_t* artibeus_get_gps() {
  return artibeus_gps;
}

// Returns pointer to latest time
uint8_t* artibeus_get_time() {
  return artibeus_time;
}

// Returns pointer to latest date
uint8_t* artibeus_get_date() {
  return artibeus_date;
}

// Sets the latest xl data and updates avg
void artibeus_set_xl(int16_t x, int16_t y, int16_t z) {
  // Update latest xl values and swap buffer
  int16_t *next_buf = (artibeus_last_imu == artibeus_last_imu_0) ?
                      artibeus_last_imu_1 : artibeus_last_imu_0;
  next_buf[0] = x; next_buf[1] = y; next_buf[2] = z;
  artibeus_last_imu = next_buf;
  // We'll do a rolling average here,
  // TODO we're taking over the rolling average, need to put it back in
  // correctly
  next_buf = (artibeus_avg_xl == artibeus_xl_avgs_0) ? 
             artibeus_xl_avgs_1 : artibeus_xl_avgs_0;
  next_buf[0] = x; next_buf[1] = y; next_buf[2] = z;
#if 0
  uint16_t count = (uint16_t)artibeus_avg_xl[ARTIBEUS_AVG_IMU_SIZE];
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
   When finished, switch buffer
#endif
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
  // TODO not taking average right now
  next_buf = (artibeus_avg_g == artibeus_g_avgs_0) ? 
             artibeus_g_avgs_1 : artibeus_g_avgs_0;
  next_buf[0] = x; next_buf[1] = y; next_buf[2] = z;
#if 0
  uint16_t count = (uint16_t)artibeus_avg_g[ARTIBEUS_AVG_IMU_SIZE];
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
#endif
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
  // TODO we're not actually putting an average in here
  next_buf = (artibeus_avg_m == artibeus_m_avgs_0) ? 
             artibeus_m_avgs_1 : artibeus_m_avgs_0;
  uint16_t count = (uint16_t)artibeus_avg_m[ARTIBEUS_AVG_IMU_SIZE];
  // (Old val * count + new_val)/(count + 1)
  int16_t new_val = (artibeus_avg_m[0] * count + x)/(count + 1);
  next_buf[0] = x; next_buf[1] = y; next_buf[2] = z;
#if 0
  // update other buffer
  next_buf[0] = new_val;
  new_val = (artibeus_avg_m[1] * count + y)/(count + 1);
  next_buf[1] = new_val;
  new_val = (artibeus_avg_m[2] * count + z)/(count + 1); 
  next_buf[2] = new_val;
  // Update count in buffer
  next_buf[ARTIBEUS_AVG_IMU_SIZE] = count + 1;
  // When finished, switch buffer
#endif
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

void artibeus_set_date(uint8_t* new_val) {
  // Just do a double buffer swap
  uint8_t *next_buf = (artibeus_date == artibeus_last_date_0) ?
                      artibeus_last_date_1 : artibeus_last_date_0;
  for(int i = 0; i < ARTIBEUS_DATE_SIZE; i++) {
    next_buf[i] = new_val[i];
  }
  artibeus_date = next_buf;
  return;
}

// Just grabs the latest telemetry data available and updates the full packet
// TODO fix the bit banging to be less magic-numbery
// We call this _set_ instead of _get_ even though it returns telemetry because
// it updates a single buffer with the latest telemetry values. You need to
// transmit that buffer atomically with calling this function. So we call it set
// to confer the idea that you're updating memory.
uint8_t * artibeus_set_telem_pkt(uint8_t *artibeus_telem_pkt_in) {
  // Patch in xl
  uint8_t * artibeus_telem_pkt = artibeus_telem_pkt_in + 1;
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
  // Patch in time and date
  uint8_t *time = artibeus_get_time();
  uint8_t *date = artibeus_get_date();
  for (int i = 0; i < ARTIBEUS_DATE_SIZE; i++) {
    *((uint8_t *)(artibeus_telem_pkt + 26 + ARTIBEUS_GPS_SIZE + i)) = date[i];
  }
  for (int i = 0; i < ARTIBEUS_TIME_SIZE; i++) {
    *((uint8_t *)(artibeus_telem_pkt + 26 + ARTIBEUS_DATE_SIZE + ARTIBEUS_GPS_SIZE + i)) = time[i];
  }
  return artibeus_telem_pkt;
}
  
uint8_t artibeus_push_ascii_pkt(buffer_t *buff) {
  write_to_log(cur_ctx,&expt_ascii_tail,sizeof(uint8_t));
  write_to_log(cur_ctx,&expt_ascii_head,sizeof(uint8_t));
  write_to_log(cur_ctx,&expt_ascii_full,sizeof(uint8_t));
  // Write to buffer
  uint8_t temp_cnt = expt_ascii_tail;
  memcpy(expt_ascii_buffer + temp_cnt, buff->pkt.msg + CMD_OFFSET,
    (buff->pkt.msg[LEN_OFFSET]) - 8);
  // Update temporary tail
  temp_cnt++;
  if (temp_cnt >= ARTIBEUS_ASCII_ENTRIES) {
    temp_cnt = 0;
  }
  // Update full
  expt_ascii_full = (temp_cnt == expt_ascii_head) ? 1 : 0;
  // Update tail
  expt_ascii_tail = temp_cnt;
  return temp_cnt;
}

uint8_t * artibeus_pop_ascii_pkt() {
  // Return pointer to head of ring
  return &(expt_ascii_buffer[expt_ascii_tail]);
}

void artibeus_pop_update_ascii_ptrs() {
  if (expt_ascii_head <= ARTIBEUS_ASCII_ENTRIES - 1) { 
    expt_ascii_head++;
  }
  else {
    expt_ascii_head = 0;
  }
  return;
}

int artibeus_ascii_is_empty() {
  if (expt_ascii_tail == expt_ascii_head && expt_ascii_full == 0) {
    return 1;
  }
  return 0;
}

void artibeus_send_ascii_pkt(buffer_t *raw_pkt) {
  // Return nack if empty
  if (artibeus_ascii_is_empty()) {
    // Return nack
    comm_return_nack(raw_pkt);
  }
  else { // Else pop packet
    uint8_t *ascii_ptr = artibeus_pop_ascii_pkt();
    libartibeus_msg_id = expt_ascii_tail;
    comm_transmit_pkt(ascii_ptr,ARTIBEUS_MAX_ASCII_SIZE);
    __delay_cycles(80000);
    artibeus_pop_update_ascii_ptrs();
  }
}


uint8_t * artibeus_pop_telem_pkt() {
  // Return pointer to head of ring
  return &(telem_buffer[telem_buffer_tail-1]);
}

// A function you call _after_ the telemetry packet has definitely been
// transmitted, as long as there's only one variable, you don't have to double
// buffer it
void artibeus_pop_update_telem_ptrs() {
  if (telem_buffer_head <= ARTIBEUS_TELEM_BUFF_SIZE - 1) { 
    telem_buffer_head++;
  }
  else {
    telem_buffer_head = 0;
  }
}

int artibeus_telem_buffer_is_empty() {
  if (telem_buffer_tail == telem_buffer_head && telem_buffer_full == 0) {
    return 1;
  }
  return 0;
}

void artibeus_send_telem_ascii_pkt(buffer_t *raw_pkt) {
  if (artibeus_telem_buffer_is_empty()) {
    // return nack
    comm_return_nack(raw_pkt);
  }
  else {
  BIT_FLIP(1,2);
  BIT_FLIP(1,1);
  BIT_FLIP(1,2);
    uint8_t *telem_ptr = artibeus_pop_telem_pkt();
    libartibeus_msg_id = telem_buffer_tail;
    comm_transmit_pkt(telem_ptr,sizeof(artibeus_telem_t) + 1);
    __delay_cycles(80000); // We'll add a little delay so transmission finishes
    artibeus_pop_update_telem_ptrs();
  }
}

uint8_t artibeus_push_telem_pkt() {
  write_to_log(cur_ctx,&telem_buffer_tail,sizeof(uint8_t));
  write_to_log(cur_ctx,&telem_buffer_head,sizeof(uint8_t));
  write_to_log(cur_ctx,&telem_buffer_full,sizeof(uint8_t));
  uint8_t temp_cnt = telem_buffer_tail;
  artibeus_set_telem_pkt(telem_buffer + temp_cnt);
  // For squishing into an ascii packet instead of a telem packet
  *((uint8_t *)(telem_buffer + temp_cnt)) = TELEM_ASCII;
  temp_cnt++;
  if (temp_cnt >= ARTIBEUS_TELEM_BUFF_SIZE) {
    temp_cnt = 0;
  }
  // Set full flag
  telem_buffer_full = (temp_cnt == telem_buffer_head) ? 1 : 0;
  // Update tail
  telem_buffer_tail = temp_cnt;
  return temp_cnt;
}
