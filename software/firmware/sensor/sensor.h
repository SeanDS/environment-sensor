#ifndef _SENSOR_H_
#define _SENSOR_H_

#define SOFTWARE_VERSION "0.9.3"

// this unit's physical address
const uint8_t mac_addr[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// sensor server settings
const uint8_t SERVER_IP[] = {192, 168, 0, 1};
const uint16_t SERVER_PORT = 50000;

// server paths for posting data
const char DUST_PATH[] = "/monitor/dust";
const char ENV_PATH[] = "/monitor/env";

// string buffer sizes
#define DHCP_BUF_SIZE 1024
#define HTTP_BUF_SIZE 512
#define PAYLOAD_BUF_SIZE 256

// LED status flags
#define LED_OFF 0
#define LED_FLASHING 1
#define LED_ON 2

// socket buffer sizes, in kB
const uint8_t WIZNET_BUF_SIZE[] = {1, 8, 1, 1, 1, 1, 1, 1};

// socket numbers on W5500
#define DHCP_SOCKET 0
#define SENSOR_DATA_SOCKET 1

void set_led_1(uint8_t);
void set_led_2(uint8_t);

void wizchip_enable(void);
void wizchip_disable(void);
void wizchip_cycle(void);
void wizchip_select(void);
void wizchip_deselect(void);

void general_timer_enable(void);
void pwm_timer_enable(void);

void hardware_init(void);

void send_data(const uint8_t*, const uint16_t, char*);
void send_json_http_post(const uint8_t*, const uint16_t, const char*, char*);
void send_dust_http_post(uint16_t, uint16_t);
void send_env_http_post(float, double, float, uint16_t);

void dust_measurement(void);
void env_measurement(void);

#endif /* _SENSOR_H_ */
