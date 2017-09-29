#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>

// DHCP buffer size
#define DATA_BUF_SIZE 2048

// DHCP host name
#define DCHP_HOST_NAME "envsensor\0"

void hardware_init(void);

void wizchip_select(void);
void wizchip_deselect(void);
uint8_t spi_transfer_byte(uint8_t);
void spi_send(uint8_t);
uint8_t spi_receive(void);

void usb_serial_putstr(char*);
void usb_send_message(const char*, ...);

#endif /* _SENSOR_H_ */
