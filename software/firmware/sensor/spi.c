#include "spi.h"

void spi_init(void) {
  // set MOSI (PORTB2) and SCK (PORTB1) as outputs
  DDRB |= (1 << PORTB2) | (1 << PORTB1);

  // set MISO (PORTB2) as input
  DDRB &= ~(1 << PORTB3);

  // REQUIRED!!! set unused SPI SS pin to output to avoid master issues
  DDRB |= (1 << PORTB0);

  // enable SPI, master mode 0, set the clock rate fck / 16
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);

  // set SPI mode 0
  SPCR &= ~(1 << CPOL);
  SPCR &= ~(1 << CPHA);

  // set MSB first
  SPCR &= ~(1 << DORD);
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
	spi_transfer_byte(data);
}

uint8_t spi_receive(void) {
	uint8_t data = spi_transfer_byte(0);

	return data;
}
