#ifndef _SENSOR_H_
#define _SENSOR_H_

// DHCP buffer size
#define DATA_BUF_SIZE 2048

// DHCP host name
//#define DCHP_HOST_NAME "ENVSENSOR\0"

void wizchip_select(void);
void wizchip_deselect(void);

void dhcp_timer_enable(void);
void sensor_timer_enable(void);

void hardware_init(void);

void usb_serial_putstr(char*);
void usb_send_message(const char*, ...);

#endif /* _SENSOR_H_ */
