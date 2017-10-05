/**
 *  Temperature/pressure/humidity readout over ethernet with USB virtual serial
 *  port debugging.
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

// redefine DHCP host name (first defined in dhcp.h)
#define DCHP_HOST_NAME "ENVSENSOR"

// USB stream
FILE usb_stream = FDEV_SETUP_STREAM(usb_serial_putchar, usb_serial_getchar, _FDEV_SETUP_RW);

// DHCP data buffer
uint8_t dhcp_buf[DHCP_BUF_SIZE];

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

// dust sensor low pulse occupancies
uint32_t p1 = 0;
uint32_t p2 = 0;

// dust measurements
volatile uint16_t dust_1 = 0;
volatile uint16_t dust_2 = 0;

void wizchip_select(void) {
	PORTB &= ~(1 << PORTB6);
}

void wizchip_deselect(void) {
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

		// reset counter
		dust_timer_ovf_count = 0;
	}

	// approx 1 second
	if (env_timer_ovf_count >= 244) {
		env_measurement_pending = true;

		// reset counter
		env_timer_ovf_count = 0;
	}

	// approx 1 second
	if (dhcp_timer_ovf_count >= 244) {
		// call DHCP second handler
		DHCP_time_handler();

		// reset counter
		dhcp_timer_ovf_count = 0;

		// set physical link check flag
		phy_link_check_pending = true;
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
	// current and previous DHCP states
	uint8_t previous_dhcp_state = DHCP_STOPPED;
	uint8_t current_dhcp_state = DHCP_STOPPED;

	// for IP address, once obtained via DHCP
	uint8_t ip[] = {0, 0, 0, 0};

	// sensor measurement variables
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

	wizchip_init((uint8_t*) WIZNET_BUF_SIZE, (uint8_t*) WIZNET_BUF_SIZE);
	setSHAR((uint8_t*) mac_addr);

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
			  printf_P(PSTR("[info] IP assigned\r\n"));
				break;
			case DHCP_IP_CHANGED:
				printf_P(PSTR("[info] IP changed\r\n"));
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

				printf_P(PSTR("[warning] DHCP failed\r\n"));

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

				printf_P(PSTR("[info] IP leased: %u.%u.%u.%u\r\n"), ip[0], ip[1], ip[2], ip[3]);
			}
		}

		if (phy_link_check_pending) {
			// check physical link
			if (wizphy_getphylink() == PHY_LINK_OFF) {
				printf_P(PSTR("[warning] physical link offline\r\n"));

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

				printf_P(PSTR("Dust 1: %u\r\n"), dust_1_copy);
				printf_P(PSTR("Dust 2: %u\r\n"), dust_2_copy);

				send_dust_http_post(dust_1_copy, dust_2_copy);
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
				printf_P(PSTR("Temperature: %.2f\r\n"), env_t);
				printf_P(PSTR("Pressure: %.2f\r\n"), env_p);
				printf_P(PSTR("Humidity: %.2f\r\n"), env_h);
				printf_P(PSTR("Light: %u\r\n"), env_l);

				send_env_http_post(env_t, env_p, env_h, env_l);
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

	// redirect stdin and stdout to USB virtual serial
	stdout = &usb_stream;
	stdin = &usb_stream;

	// wait until USB host sets configuration
	//while (!usb_configured())
	//_delay_ms(1000);

	// wait until host opens terminal
	//while (!(usb_serial_get_control() & USB_SERIAL_DTR))

	// discard anything that was received prior.  Sometimes the
	// operating system or other software will send a modem
	// "AT command", which can still be buffered.
	//usb_serial_flush_input();

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

void send_data(const uint8_t* dest_ip, const uint16_t dest_port, char* msg) {
	// status variables for network functions
	int8_t sck_status;
	int8_t con_status;
	int32_t send_status;
	int8_t close_status;

	// create a TCP socket
	if ((sck_status = socket(SENSOR_DATA_SOCKET, Sn_MR_TCP, 0, 0)) == SENSOR_DATA_SOCKET) {
		// connect to the server
		if ((con_status = connect(SENSOR_DATA_SOCKET, (uint8_t*) dest_ip, (uint16_t) dest_port)) == SOCK_OK) {
			if ((send_status = send(SENSOR_DATA_SOCKET, (uint8_t*) msg, strlen(msg))) != strlen(msg)) {
				printf_P(PSTR("[error] unable to send: %" PRIi32 "\r\n"), send_status);
			}
		} else {
			printf_P(PSTR("[error] connection not available: %i\r\n"), con_status);
		}

		// close the socket
		if ((close_status = close(SENSOR_DATA_SOCKET)) == SOCK_OK) {
			// do nothing
		} else {
			printf_P(PSTR("[error] unable to close socket: %i\r\n"), close_status);
		}
	} else {
		printf_P(PSTR("[error] unable to create socket: %" PRIi8 "\r\n"), sck_status);
	}
}

void send_http_post(const uint8_t* dest_ip, const uint16_t dest_port, const char* path, char* msg) {
	// buffer for HTTP request
	char http_msg[HTTP_BUF_SIZE];

	// construct HTTP request (must use HTTP 1.0 because we cannot define the
	// HTTP/1.1 required host header)
	sprintf_P(http_msg, PSTR(
		"POST %s HTTP/1.0\r\n"
		"Content-Type: text/html; charset=utf-8\r\n"
		"Connection: close\r\n"
		"\r\n"
		"%s"),
		path, msg);

	return send_data(dest_ip, dest_port, http_msg);
}

void send_dust_http_post(uint16_t dust_1, uint16_t dust_2) {
	// sensor payload buffer
	char payload[PAYLOAD_BUF_SIZE];

	sprintf_P(payload, PSTR("{"
		"\"version\":" SOFTWARE_VERSION ","
		"\"mac\":\"%02x:%02x:%02x:%02x:%02x:%02x\","
		"\"dust1\":%u,"
		"\"dust2\":%u"
		"}"),
		mac_addr[0], mac_addr[1], mac_addr[2],
		mac_addr[3], mac_addr[4], mac_addr[5],
		dust_1, dust_2
	);

	send_http_post(SERVER_IP, SERVER_PORT, DUST_PATH, payload);
}

void send_env_http_post(float env_t, double env_p, float env_h, uint16_t env_l) {
	// sensor payload buffer
	char payload[PAYLOAD_BUF_SIZE];

	sprintf_P(payload, PSTR("{"
		"\"version\":" SOFTWARE_VERSION ","
		"\"mac\":\"%02x:%02x:%02x:%02x:%02x:%02x\","
		"\"temperature\":%.2f,"
		"\"pressure\":%.2f,"
		"\"humidity\":%.2f,"
		"\"light\":%u"
		"}"),
		mac_addr[0], mac_addr[1], mac_addr[2],
		mac_addr[3], mac_addr[4], mac_addr[5],
		env_t, env_p, env_h, env_l
	);

	send_http_post(SERVER_IP, SERVER_PORT, ENV_PATH, payload);
}
