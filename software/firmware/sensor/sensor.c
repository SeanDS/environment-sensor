/** \file
 *
 *  Temperature/pressure/humidity readout over USB virtual serial port.
 *
 *
 *  Sean Leavey
 *  electronics@attackllama.com
 */

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <w5500.h>
#include <socket.h>
#include <dhcp.h>

#include "sensor.h"
#include "usb.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "bme280.h"

// server
//uint8_t server_ip[4] = {192, 168, 0, 40};
//uint16_t server_port = 50000;

// timer overflow counters
uint16_t dhcp_timer_ovf_count = 0;
uint16_t env_timer_ovf_count = 0;
uint16_t dust_timer_ovf_count = 0;

// whether a new measurements have been made
bool env_measurement_ready = false;
bool dust_measurement_ready = false;

// environment sensor values
volatile float env_t = 0;
volatile double env_p = 0;
volatile float env_h = 0;
volatile uint16_t env_l = 0;

// dust sensor low pulse occupancies
uint32_t p1 = 0;
uint32_t p2 = 0;

// dust measurements
volatile uint16_t dust_1 = 0;
volatile uint16_t dust_2 = 0;

wiz_NetInfo net_info = { .mac 	= {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},
												 .ip 	= {192, 168, 0, 101},
												 .sn 	= {255, 255, 255, 0},
												 .gw 	= {192, 168, 0, 1},
												 .dns = {0, 0, 0, 0},
												 .dhcp = NETINFO_DHCP };

void wizchip_select(void) {
	//usb_send_message("[S]\n");
	PORTB &= ~(1 << PORTB6);
}

void wizchip_deselect(void) {
	//usb_send_message("\n[D]\n");
	PORTB |= (1 << PORTB6);
}

void dhcp_timer_enable(void) {
	// enable first timer
	TIMSK1 |= (1 << TOIE1);

	// set initial value to 0
	TCNT1 = 0x00;

	// start timer with 1/1 scaling
	// this gives overflows every (1/16e6)*65536 = 4.096 ms
	TCCR1B = (1 << CS10);
}

void sensor_timer_enable(void) {
	// enable first timer
	TIMSK3 |= (1 << TOIE3);

	// set initial value to 0
	TCNT3 = 0x00;

	// start timer with 1/1 scaling
	// this gives overflows every (1/16e6)*65536 = 4.096 ms
	TCCR3B = (1 << CS30);
}

/*  DHCP 1 second timer interrupt.
 *  This counts overflows of the AVR's TIMER1. After enough overflows, it fires
 *  the DHCP time handler.
 */
ISR(TIMER1_OVF_vect) {
	dhcp_timer_ovf_count++;

	if (dhcp_timer_ovf_count >= 244) {
		// trigger every ~1 second
		DHCP_time_handler();

		dhcp_timer_ovf_count = 0;
	}
}

ISR(TIMER3_OVF_vect) {
	// triggers every 4.096 ms

	dust_timer_ovf_count++;
	env_timer_ovf_count++;

	// measure dust triggers: add 1 if the dust sensor output is pulled low
	// (indicating dust), else add 0
	p1 += (PIND & (1 << PIND6)) == 0 ? 1 : 0;
	p2 += (PIND & (1 << PIND7)) == 0 ? 1 : 0;

	// approx 30 seconds
	if (dust_timer_ovf_count > 7324) {
		// record pulse counts
		dust_1 = p1;
		dust_2 = p2;

		dust_measurement_ready = true;

		// reset pulse counters
		p1 = 0;
		p2 = 0;

		// reset timer
		dust_timer_ovf_count = 0;
	}

	// approx 1 second
	if (env_timer_ovf_count >= 244) {
		// take measurements
		env_t = bme280_read_temperature();
		env_p = bme280_read_pressure();
		env_h = bme280_read_humidity();
		env_l = analog_read(ADC_8);

		env_measurement_ready = true;

		// reset timer
		env_timer_ovf_count = 0;
	}
}

/** Main program entry point.
 *
 */
int main(void)
{
	// DHCP data buffer
	uint8_t dhcp_buf[DATA_BUF_SIZE];

	// socket buffer sizes, in kB
	uint8_t wiznet_buf_size[] = {2, 2, 2, 2, 2, 2, 2, 2};

	int8_t sck_status = 0;
	uint8_t ip_leased = 0;
	uint8_t socket_opened = 0;

	uint16_t dust_1_copy;
	uint16_t dust_2_copy;
	float env_t_copy;
	double env_p_copy;
	float env_h_copy;
	uint16_t env_l_copy;

	bool run_user_applications = false;

	hardware_init();

	sensor_timer_enable();

	wizchip_init(wiznet_buf_size, wiznet_buf_size);
	wizchip_setnetinfo(&net_info);
	setSHAR(net_info.mac);

	reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
	reg_wizchip_spi_cbfunc(spi_receive, spi_send);

	DHCP_init(0, dhcp_buf);

	dhcp_timer_enable();

	for (;;)
	{
		switch (DHCP_run())
		{
			case DHCP_IP_ASSIGN:
				usb_send_message("IP assigned\n");
				break;
			case DHCP_IP_CHANGED:
				usb_send_message("IP changed\n");
				break;
			case DHCP_IP_LEASED:
			  run_user_applications = true;
				break;
			case DHCP_FAILED:
				usb_send_message("DHCP failed\n");
				break;
			default:
				break;
		}

		if (run_user_applications) {
			// if ((ip_leased == 1) && (socket_opened == 0)) {
			// 	sck_status = socket(1, Sn_MR_TCP, 80, 0x0);
			// 	usb_serial_write("IP leased; opened socket 1\n", 27);
			//
			// 	socket_opened = 1;
			// }
			//
			// if (ip_leased == 1) {
			// 	usb_send_message("Leased\n");
			// }
			//
			// if (socket_opened == 1) {
			// 	usb_send_message("Socket opened\n");
			// }

			if (dust_measurement_ready) {
				// avoid interrupts while reading sensor values
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
					dust_1_copy = dust_1;
					dust_2_copy = dust_2;
				}

				// reset
				dust_measurement_ready = false;

				usb_send_message("Dust 1: %u\nDust 2: %u\n", dust_1_copy, dust_2_copy);
			}

			if (env_measurement_ready) {
				// avoid interrupts while reading sensor values
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
					env_t_copy = env_t;
					env_p_copy = env_p;
					env_h_copy = env_h;
					env_l_copy = env_l;
				}

				// reset
				env_measurement_ready = false;

				// float support below needs linker flags: -Wl,-u,vfprintf -lprintf_flt
				usb_send_message("Temperature: %.2f\nPressure: %.2f\nHumidity: %.2f\nLight: %u\n",
				  env_t_copy, env_p_copy, env_h_copy, env_l_copy);

				// if ((ip_leased == 1) && (socket_opened == 1)) {
				// 	char msg[] = "data\0";
				// 	usb_send_message("sck: %i\n", sck_status);
				// 	int8_t c_status = connect(1, server_ip, server_port);
				// 	usb_send_message("connect: %i\n", c_status);
				// 	int32_t s_status = send(1, msg, strlen(msg));
				// 	usb_send_message("send: %i\n", s_status);
				// 	disconnect(1);
				// }
			}
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

	// set W5500 chip select as output
	DDRB |= (1 << PORTB6);

	// disable W5500 chip select
  PORTB |= (1 << PORTB6);

	// set dust sensor P1 and P2 inputs
	DDRD &= ~(1 << PORTD6) | ~(1 << PORTD7);

	// set dust sensor threshold as output
	DDRB |= (1 << PORTB4);

	// USB hardware initialisation
	usb_init();

	// wait until USB host sets configuration
	while (!usb_configured())
	_delay_ms(1000);

	// wait until host opens terminal
	while (!(usb_serial_get_control() & USB_SERIAL_DTR))

	// discard anything that was received prior.  Sometimes the
	// operating system or other software will send a modem
	// "AT command", which can still be buffered.
	usb_serial_flush_input();

	// initialise analog to digital converter
	adc_init();

	// initialise SPI
	spi_init();

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
	// USB message buffer
	static char usb_buf[1024];

	// list to store variable arguments
	va_list args;

	// start of variable arguments, with format the last required argument
	va_start(args, format);

	// variable argument print, (overflow safe)
	vsnprintf(usb_buf, sizeof(usb_buf), format, args);

	// end of variable arguments
	va_end(args);

	// send USB string
	usb_serial_putstr(usb_buf);
}
