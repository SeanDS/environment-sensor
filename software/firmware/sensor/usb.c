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
#include <util/delay.h>
#include <limits.h>

#include "usb.h"
#include "i2c.h"
#include "bme280.h"
#include "ftoa.h"

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

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	float a;
  float b;
  float c;

	char buf[30];

	SetupHardware();

	i2c_init();
	bme280_init();

	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

	GlobalInterruptEnable();

	unsigned int i = 0;

	for (;;)
	{
		if (i >= UINT_MAX) {
			i = 0;
		}

		if (i % 100000 == 0) {
			a = bme280_read_temperature();
    	b = bme280_read_pressure();
    	c = bme280_read_humidity();

			// float support below needs linker flags: -Wl,-u,vfprintf -lprintf_flt
			usb_send_message("Temperature: %.2f\n", a);
			usb_send_message("Pressure: %.2f\n", b);
			usb_send_message("Humidity: %.2f\n", c);
		}

		i++;

		/* Must throw away unused bytes from the host, or it will lock up while waiting for the device */
		CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);

		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	USB_Init();
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

// USB serial message with fprintf support
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
