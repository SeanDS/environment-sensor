/** \file
 *
 *  Temperature/pressure/humidity readout over USB virtual serial port.
 *
 *  Based on LUFA VirtualSerial example.
 *
 *  Sean Leavey
 *  electronics@attackllama.com
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <limits.h>
#include <socket.h>
#include <dhcp.h>

#include "usb.h"
#include "adc.h"
#include "i2c.h"
#include "bme280.h"
//#include "network.h"

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
				.DataINEndpoint           =
					{
						.Address          = CDC_TX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.DataOUTEndpoint =
					{
						.Address          = CDC_RX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.NotificationEndpoint =
					{
						.Address          = CDC_NOTIFICATION_EPADDR,
						.Size             = CDC_NOTIFICATION_EPSIZE,
						.Banks            = 1,
					},
			},
	};

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs.
 */
static FILE USBSerialStream;

// DHCP data buffer
// NOTE: must be global
uint8_t dhcp_buf[DATA_BUF_SIZE];

// variable for IP addresses
uint8_t ip[4];

// DHCP timer overflow counter
uint8_t timer_ovf_count = 0;

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
	uint8_t wizbuf[] = {2, 2, 2, 2, 2, 2, 2, 2};

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

	wizchip_init(wizbuf, wizbuf);

  wiz_NetInfo netInfo = { .mac 	= {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},	// Mac address
                          .ip 	= {192, 168, 0, 101},					// IP address
                          .sn 	= {255, 255, 255, 0},					// Subnet mask
                          .gw 	= {192, 168, 0, 1},		  			// Gateway address
													.dhcp = NETINFO_DHCP };
  //wizchip_setnetinfo(&netInfo);
	setSHAR(netInfo.mac);

	float a;
  float b;
  float c;

	char buf[30];

	SetupHardware();

	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

	GlobalInterruptEnable();

	dhcp_timer_enable();
	DHCP_init(0, dhcp_buf);

	unsigned int i = 0;

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

					//getIPfromDHCP(ip);
					//usb_send_message("IP = %i.%i.%i.%i\n", ip[0], ip[1], ip[2], ip[3]);

					usb_send_message("Light sensor: %u\n", analog_read(ADC_8));

					//W5500_status();
				}

				i++;

				break;
			case DHCP_FAILED:
				usb_send_message("DHCP failed\n");
				break;
			default:
				break;
		}

		/* Must throw away unused bytes from the host, or it will lock up while waiting for the device */
		CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);

		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	// disable watchdog if enabled by bootloader/fuses
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// disable clock division
	clock_prescale_set(clock_div_1);

	// USB hardware initialisation
	USB_Init();

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

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{

}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{

}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

/** CDC class driver callback function the processing of changes to the virtual
 *  control lines sent from the host..
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo)
{
	/* You can get changes to the virtual CDC lines in this callback; a common
	   use-case is to use the Data Terminal Ready (DTR) flag to enable and
	   disable CDC communications in your application when set to avoid the
	   application blocking while waiting for a host to become ready and read
	   in the pending data from the USB endpoints.
	*/
	bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;
}

// returns -1 when nothing is received
int16_t usb_receive_byte(void) {
	return CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
}

uint16_t usb_bytes_received(void) {
	return CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface);
}

void usb_receive_message(char* buf, uint16_t length) {
	// number of received bytes in buffer
	uint16_t n = CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface);

	uint16_t i;

	// only read characters up to buffer size
	if (length < n) {
		n = length;
	}

	for (i = 0; i < n; i++) {
		buf[i] = (uint8_t) usb_receive_byte();
	}

	buf[i] = '\0';
}

void usb_send_byte(uint8_t data) {
  CDC_Device_SendByte(&VirtualSerial_CDC_Interface, data);
}

/*  USB serial message with fprintf support.
 *
 */
void usb_send_message(const char *format, ...) {
	// buffer for formatted string
	unsigned char* buf[1000];

	// list to store variable arguments
	va_list args;

	// start of variable arguments, with format the last required argument
	va_start(args, format);

	// variable argument print, (overflow safe)
	vsnprintf(buf, sizeof buf, format, args);

	// end of variable arguments
	va_end(args);

	// send USB string
	_usb_send_string(buf);
}

void usb_send_data(const char *const buf, const uint16_t length) {
	CDC_Device_SendData(&VirtualSerial_CDC_Interface, buf, length);
}

void _usb_send_string(const char *string) {
	CDC_Device_SendString(&VirtualSerial_CDC_Interface, string);
}
