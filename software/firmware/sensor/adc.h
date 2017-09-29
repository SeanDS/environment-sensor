#ifndef _ADC_H_
#define _ADC_H_

#define ADC_0 0b00000000
#define ADC_1 0b00000001
// No ADC_2
// No ADC_3
#define ADC_4 0b00000100
#define ADC_5 0b00000101
#define ADC_6 0b00000110
#define ADC_7 0b00000111
#define ADC_8 0b00100000
#define ADC_9 0b00100001
#define ADC_10 0b00100010
#define ADC_11 0b00100011
#define ADC_12 0b00100100
#define ADC_13 0b00100101
#define ADC_TEMPERATURE_SENSOR 0b00100111

void adc_init(void);
uint16_t analog_read(uint8_t);

#endif /* _ADC_H_ */
