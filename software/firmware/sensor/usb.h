#ifndef _USB_H_
#define _USB_H_

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>

#include "descriptors.h"
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

// DHCP buffer size
#define DATA_BUF_SIZE 2048

// DHCP host name
#define DCHP_HOST_NAME "envsensor\0"

void SetupHardware(void);

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);

void wizchip_select(void);
void wizchip_deselect(void);
uint8_t spi_transfer_byte(uint8_t data);
void spi_send(uint8_t data);
uint8_t spi_receive(void);

int16_t usb_receive_byte(void);
uint16_t usb_bytes_received(void);
void usb_receive_message(char*, uint16_t);
void usb_send_byte(uint8_t);
void usb_send_data(const char *const, const uint16_t);
void usb_send_message(const char*, ...);
void _usb_send_string(const char*);

#endif
