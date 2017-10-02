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

// sensor server settings
const uint8_t server_ip[] = {192, 168, 0, 40};
const uint16_t server_port = 50000;

// timer overflow counters
uint8_t pwm_timer_ovf_count = 0;
uint16_t dust_timer_ovf_count = 0;
uint8_t env_timer_ovf_count = 0;
uint8_t dhcp_timer_ovf_count = 0;

// measurement flags controlled by interrupts
bool env_measurement_pending = false;
bool dust_measurement_ready = false;

// physical link check flag
bool phy_link_check_pending = false;

// data send flag
bool data_send_pending = false;

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
	//usb_write_line("[S]");
	PORTB &= ~(1 << PORTB6);
}

void wizchip_deselect(void) {
	//usb_write_line("");
	//usb_write_line("[D]");
	PORTB |= (1 << PORTB6);
}

void general_timer_enable(void) {
	// enable first timer
	TIMSK3 |= (1 << TOIE3);

	// set initial value to 0
	TCNT3 = 0x00;

	// start timer with 1/1 scaling
	// this gives overflows every (1/16e6)*65536 = 4.096 ms
	TCCR3B = (1 << CS30);
}

void pwm_timer_enable(void) {
	// enable first timer
	TIMSK0 |= (1 << TOIE0);

	// set initial value to 0
	TCNT0 = 0x00;

	// start timer with 1/1 scaling
	// this gives overflows every (1/16e6)*256 = 16 us
	TCCR0B = (1 << CS00);
}

/*
 *  DHCP and sensor timer interrupt.
 *
 *  This counts overflows of the AVR's TIMER3. It is responsible for triggering
 *  the DHCP library's second handler, measuring the dust sensor low pulse
 *  occupancy and flagging measurements to be made from the BME280.
 */
ISR(TIMER3_OVF_vect) {
	// triggers every 4.096 ms

	// increment overflow counters
	dust_timer_ovf_count++;
	env_timer_ovf_count++;
	dhcp_timer_ovf_count++;

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
		env_measurement_pending = true;

		// reset timer
		env_timer_ovf_count = 0;
	}

	// approx 1 second
	if (dhcp_timer_ovf_count >= 244) {
		// call DHCP second handler
		DHCP_time_handler();

		// reset DHCP overflow counter
		dhcp_timer_ovf_count = 0;

		// set physical link check flag
		phy_link_check_pending = true;

		// FIXME: move this out to its own counter
		data_send_pending = true;
	}
}

/*
 *  PWM timer interrupt.
 */
ISR(TIMER0_OVF_vect) {
	// increment overflow counter
	pwm_timer_ovf_count++;

	if (pwm_timer_ovf_count >= 125) {
		// switch on
		PORTB |= (1 << PORTB4);
	} else {
		// switch off
		PORTB &= ~(1 << PORTB4);
	}

	if (pwm_timer_ovf_count >= 128) {
		// reset
		pwm_timer_ovf_count = 0;
	}
}

/** Main program entry point.
 *
 */
int main(void)
{
	// DHCP data buffer
	uint8_t dhcp_buf[DATA_BUF_SIZE];

	// data buffer
	uint8_t data_buffer[DATA_BUF_SIZE];

	int8_t sock_status;

	// current and previous DHCP states
	int8_t previous_dhcp_state;
	int8_t current_dhcp_state;

	// socket buffer sizes, in kB
	uint8_t wiznet_buf_size[] = {2, 2, 2, 2, 2, 2, 2, 2};

	// IP address
	uint8_t ip[] = {0, 0, 0, 0};

	uint16_t dust_1_copy;
	uint16_t dust_2_copy;
	float env_t;
	double env_p;
	float env_h;
	uint16_t env_l;

	// network status flags
	bool dhcp_ready = false;
	bool phy_link_ready = false;

	hardware_init();
	pwm_timer_enable();

	wizchip_init(wiznet_buf_size, wiznet_buf_size);
	wizchip_setnetinfo(&net_info);
	setSHAR(net_info.mac);

	reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
	reg_wizchip_spi_cbfunc(spi_receive, spi_send);

	DHCP_init(DHCP_SOCKET, dhcp_buf);

	general_timer_enable();

	for (;;)
	{
		// remember previous DHCP state
		previous_dhcp_state = current_dhcp_state;

		// run DHCP to acquire or maintain IP address
		current_dhcp_state = DHCP_run();

		switch (current_dhcp_state) {
			case DHCP_IP_ASSIGN:
			  usb_write_line("[info] IP assigned");
				break;
			case DHCP_IP_CHANGED:
				usb_write_line("[info] IP changed");
				break;
			case DHCP_IP_LEASED:
				// DHCP negotiation successful

				// signal DHCP ready
			  dhcp_ready = true;

				break;
			case DHCP_FAILED:
				// DHCP negotiation failed

				// signal DHCP not ready
			  dhcp_ready = false;

				usb_write_line("[warning] DHCP failed");

				break;
			default:
				// DHCP negotiation in progress

				// signal DHCP not ready
			  dhcp_ready = false;

				break;
		}

		if (current_dhcp_state != previous_dhcp_state) {
			// state has changed

			if (current_dhcp_state == DHCP_IP_LEASED) {
				// avoid interrupts while reading IP
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
					getIPfromDHCP(ip);
				}

				usb_write_line("[info] IP leased: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
			}
		}

		if (phy_link_check_pending) {
			// check physical link
			if (wizphy_getphylink() == PHY_LINK_OFF) {
				usb_write_line("[warning] physical link offline");

				// signal physical link not ready
				phy_link_ready = false;
			} else {
				// signal physical link ready
				phy_link_ready = true;
			}

			phy_link_check_pending = false;
		}

		if (dhcp_ready && phy_link_ready) {
			if (dust_measurement_ready) {
				// avoid interrupts while reading sensor values
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
					dust_1_copy = dust_1;
					dust_2_copy = dust_2;
				}

				// reset
				dust_measurement_ready = false;

				usb_write_line("Dust 1: %u", dust_1_copy);
				usb_write_line("Dust 2: %u", dust_2_copy);
			}

			if (env_measurement_pending) {
				// take measurements
				env_t = bme280_read_temperature();
				env_p = bme280_read_pressure();
				env_h = bme280_read_humidity();
				env_l = analog_read(ADC_8);

				// reset
				env_measurement_pending = false;

				// float support below needs linker flags: -Wl,-u,vfprintf -lprintf_flt
				usb_write_line("Temperature: %.2f", env_t);
				usb_write_line("Pressure: %.2f", env_p);
				usb_write_line("Humidity: %.2f", env_h);
				usb_write_line("Light: %u", env_l);
			}

			if (data_send_pending) {
				sock_status = socket(SENSOR_DATA_SOCKET, Sn_MR_TCP, NULL, NULL);

				if (connect(SENSOR_DATA_SOCKET, server_ip, server_port)) {
					PORTC &= ~(1 << PC6);

					uint8_t msg[] = "hi there\0";

					int32_t send_len = send(SENSOR_DATA_SOCKET, msg, strlen(msg));
			  } else {
					PORTC |= (1 << PC6);
				}

				data_send_pending = false;
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

	// set LED pins as outputs
	DDRC |= (1 << PC6) | (1 << PC7);

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

void usb_write_line(const char *str, ...) {
	// USB message buffer
	static char usb_buf[1024];

	// list to store variable arguments
	va_list args;

	// start of variable arguments, with format the last required argument
	va_start(args, str);

	// variable argument print, (overflow safe)
	vsnprintf(usb_buf, sizeof(usb_buf), str, args);

	// end of variable arguments
	va_end(args);

	// append newline characters
	strcat(usb_buf, "\r\n");

	// send USB string
	usb_serial_putstr(usb_buf);
}
