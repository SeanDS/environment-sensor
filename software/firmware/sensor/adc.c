#include <avr/io.h>

#include "adc.h"

void adc_init(void) {
  // enable ADC, divide clock by 128
  ADCSRA  = (1 << ADEN) | (0b111 << ADPS0);

  // use AVcc as voltage reference
  ADMUX = (0b01 << REFS0);
}

uint16_t analog_read(uint8_t channel) {
  // extract 6th bit for MUX5 and insert into ADCSRB
  ADCSRB = (ADCSRB & 0xDF) | (channel & 0x20);

  // set MUX4..MUX0 in ADMUX
  ADMUX = (ADMUX & 0xE0) | (channel & 0x1F);

  // start conversion
  ADCSRA |= (1 << ADSC);

  // wait until conversion is complete, signalled by ADSC becoming 0
  while (ADCSRA & (1 << ADSC));

  // return value combines HIGH and LOW registers
  return ADC;
}
