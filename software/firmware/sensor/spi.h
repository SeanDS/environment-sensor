#ifndef _SPI_H_
#define _SPI_H_

#include <avr/io.h>

void spi_init(void);
uint8_t spi_transfer_byte(uint8_t);
void spi_send(uint8_t);
uint8_t spi_receive(void);

#endif /* _SPI_H_ */
