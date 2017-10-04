#ifndef _SENSOR_H_
#define _SENSOR_H_

// DHCP packet buffer size
#define DATA_BUF_SIZE 2048

// DHCP socket on W5500
#define DHCP_SOCKET 0

// DHCP host name
#define DCHP_HOST_NAME "ENVSENSOR\0"

#define SENSOR_DATA_SOCKET 1

// sensor server settings
const uint8_t SERVER_IP[] = {192, 168, 0, 40};
const uint16_t SERVER_PORT = 50000;
const char dust_target[] = "/monitor/dust/post";
const char env_target[] = "/monitor/env/post";

void wizchip_select(void);
void wizchip_deselect(void);

void general_timer_enable(void);
void pwm_timer_enable(void);

void hardware_init(void);

void send_data(uint8_t*, uint8_t*, uint16_t);

#endif /* _SENSOR_H_ */
