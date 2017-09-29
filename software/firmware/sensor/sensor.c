/** \file
 *
 *  Temperature/pressure/humidity readout over USB virtual serial port.
 *
 *
 *  Sean Leavey
 *  electronics@attackllama.com
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <limits.h>
#include <socket.h>
#include <dhcp.h>

#include "sensor.h"
#include "usb.h"
#include "adc.h"
#include "i2c.h"
#include "bme280.h"

// DHCP data buffer
// NOTE: must be global
uint8_t dhcp_buf[DATA_BUF_SIZE];

// variable for IP addresses
uint8_t ip[4];

// DHCP timer overflow counter
uint8_t timer_ovf_count = 0;

wiz_NetInfo net_info = { .mac 	= {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},	// Mac address
												.ip 	= {192, 168, 0, 101},					// IP address
												.sn 	= {255, 255, 255, 0},					// Subnet mask
												.gw 	= {192, 168, 0, 1},		  			// Gateway address
												.dhcp = NETINFO_DHCP };

void wizchip_select(void) {
	//usb_send_message("[S]\n");
	PORTB &= ~(1 << PORTB6);
}

void wizchip_deselect(void) {
	//usb_send_message("\n[D]\n");
	PORTB |= (1 << PORTB6);
}

/** Transfer byte to or from SPI slave.
 *  This is used to both read from and write to the slave.
 */
uint8_t spi_transfer_byte(uint8_t data) {
	SPDR = data;

	while ((SPSR & _BV(SPIF)) == 0);

	return SPDR;
}

void spi_send(uint8_t data) {
	//usb_send_message("[send]%i", data);
	spi_transfer_byte(data);
}

uint8_t spi_receive(void) {
	uint8_t data = spi_transfer_byte(0);
	//usb_send_message("[receive]%i", data);

	return data;
}

void dhcp_timer_enable(void) {
	// enable first timer
	TIMSK1 |= (1 << TOIE1);

	// set initial value to 0
	TCNT1 = 0x00;

	// start timer with 1/1 scaling
	// this gives overflows every (1/16e6)*65536 = 4.096 ms
	TCCR1B = (1 << CS12);
}

/*  DHCP 1 second timer interrupt.
 *  This counts overflows of the AVR's TIMER1. After enough overflows, it fires
 *  the DHCP time handler.
 */
ISR(TIMER1_OVF_vect) {
	timer_ovf_count++;

	// 244 * (1 / 16M) * 65536 = 0.999424 s
	if (timer_ovf_count >= 244) {
		// fire
		DHCP_time_handler();

		// reset overflow counter
		timer_ovf_count = 0;
	}
}

/** Main program entry point.
 *
 */
int main(void)
{
	// socket buffer sizes, in kB
	uint8_t wiznet_buf_size[] = {2, 2, 2, 2, 2, 2, 2, 2};

	// set MOSI (PORTB3), SCK (PORTB1) and PORTB6 (SS) as output, others as input
	DDRB = (1 << PORTB2) | (1 << PORTB1) | (1 << PORTB6);

	// REQUIRED!!! set unused SPI SS pin to output to avoid master issues
	DDRB |= (1 << PORTB0);

  // disable W5500 chip select
  PORTB |= (1 << PORTB6);

  // enable SPI, master mode 0, set the clock rate fck / 16
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);

	// set SPI mode 0
	SPCR &= ~(1 << CPOL);
	SPCR &= ~(1 << CPHA);

	// set MSB first
	SPCR &= ~(1 << DORD);

	reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
	reg_wizchip_spi_cbfunc(spi_receive, spi_send);

	wizchip_init(wiznet_buf_size, wiznet_buf_size);
	setSHAR(net_info.mac);

	float a, b, c;

	hardware_init();

	dhcp_timer_enable();
	DHCP_init(0, dhcp_buf);

	uint16_t i = 0;

	for (;;)
	{
		switch(DHCP_run())
		{
			case DHCP_IP_ASSIGN:
				usb_send_message("IP assigned\n");
				break;
			case DHCP_IP_CHANGED:
				usb_send_message("IP changed\n");
				break;
			case DHCP_IP_LEASED:
				if (i >= UINT_MAX) {
					i = 0;
				}

				if (i % 10000 == 0) {
					a = bme280_read_temperature();
					b = bme280_read_pressure();
					c = bme280_read_humidity();

					// float support below needs linker flags: -Wl,-u,vfprintf -lprintf_flt
					usb_send_message("Temperature: %.2f\n", a);
					usb_send_message("Pressure: %.2f\n", b);
					usb_send_message("Humidity: %.2f\n", c);

					getIPfromDHCP(ip);
					usb_send_message("IP = %i.%i.%i.%i\n", ip[0], ip[1], ip[2], ip[3]);

					usb_send_message("Light sensor: %u\n", analog_read(ADC_8));
					usb_send_message("uC temperature: %u\n", analog_read(ADC_TEMPERATURE_SENSOR));

					//W5500_status();
				}

				i++;

				break;
			case DHCP_FAILED:
				//usb_send_message("DHCP failed\n");
				break;
			default:
				break;
		}
	}
}

void hardware_init(void)
{
	// disable watchdog if enabled by bootloader/fuses
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// disable clock division
	clock_prescale_set(clock_div_1);

	// USB hardware initialisation
	usb_init();

	// wait until USB host sets configuration
	while (!usb_configured())
	_delay_ms(1000);

	// wait until host opens terminal
	//while (!(usb_serial_get_control() & USB_SERIAL_DTR))

	// discard anything that was received prior.  Sometimes the
	// operating system or other software will send a modem
	// "AT command", which can still be buffered.
	usb_serial_flush_input();

	// initialise analog to digital converter
	adc_init();

	// initialise I2C bus and BME280 sensors
	i2c_init();
	bme280_init();

	// take some throw-away measurements with the BME280 (the first few are
	// usually wrong)
	bme280_read_temperature();
	bme280_read_pressure();
	bme280_read_humidity();
	bme280_read_temperature();
	bme280_read_pressure();
	bme280_read_humidity();
}

void usb_serial_putstr(char* str)
{
	for (int i = 0; i < strlen(str); i++) {
  	usb_serial_putchar(str[i]);
	}
}

void usb_send_message(const char *format, ...) {
	// buffer for formatted string
	char buf[1000];

	// list to store variable arguments
	va_list args;

	// start of variable arguments, with format the last required argument
	va_start(args, format);

	// variable argument print, (overflow safe)
	vsnprintf(buf, sizeof(buf), format, args);

	// end of variable arguments
	va_end(args);

	// send USB string
	usb_serial_putstr(buf);
}
