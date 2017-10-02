#ifndef _SENSOR_H_
#define _SENSOR_H_

// DHCP packet buffer size
#define DATA_BUF_SIZE 2048

// DHCP socket on W5500
#define DHCP_SOCKET 0

// DHCP host name
//#define DCHP_HOST_NAME "ENVSENSOR\0"

#define SENSOR_DATA_SOCKET 5

void wizchip_select(void);
void wizchip_deselect(void);

void general_timer_enable(void);
void pwm_timer_enable(void);

void hardware_init(void);

void usb_serial_putstr(char*);
void usb_write_line(const char*, ...);

#endif /* _SENSOR_H_ */
