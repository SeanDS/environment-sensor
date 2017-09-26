#include "bme280.h"
#include "usb.h"

uint16_t read8(uint8_t reg)
{
  uint16_t val;

  if (i2c_start(BME280_ADDRESS<<1 | I2C_WRITE) == 0)
  {
    i2c_write((uint8_t)reg);

    i2c_stop();

    if (i2c_start(BME280_ADDRESS<<1 | I2C_READ) == 0)
    {
      val |= ((uint16_t)i2c_read_nack());

      i2c_stop();

      return val;
    }
  }
}


uint16_t read16(uint8_t reg)
{
  uint16_t val;

  if (i2c_start(BME280_ADDRESS<<1 | I2C_WRITE) == 0)
  {
    i2c_write((uint8_t)reg);

    i2c_stop();

    if (i2c_start(BME280_ADDRESS<<1 | I2C_READ) == 0)
    {
      val = ((uint16_t)i2c_read_ack());
      val <<= 8;
      val |= ((uint16_t)i2c_read_nack());

      i2c_stop();

      return val;
    }
  }
}

uint16_t read16_LE(uint8_t reg)
{
  uint16_t temp = read16(reg);
  return (temp >> 8) | (temp << 8);

}

int16_t readS16(uint8_t reg)
{
  return (int16_t)read16(reg);

}

int16_t readS16_LE(uint8_t reg)
{
  return (int16_t)read16_LE(reg);

}

uint32_t read24(uint8_t reg)
{
  uint32_t val;

  if (i2c_start(BME280_ADDRESS<<1 | I2C_WRITE) == 0)
  {
    i2c_write(reg);

    i2c_stop();

    if (i2c_start(BME280_ADDRESS<<1 | I2C_READ) == 0)
    {
      val = ((uint16_t) i2c_read_ack());
      val <<= 8;
      val |= ((uint16_t) i2c_read_ack());
      val <<= 8;
      val |= ((uint16_t) i2c_read_nack());

      i2c_stop();

      return val;
    } else
    {
      usb_send_message("Could not connect to sensor");
    }
  }
}

void bme280_read_coefficients(void)
{
  _bme280_calib.dig_T1 = read16_LE(BME280_REGISTER_DIG_T1);
  _bme280_calib.dig_T2 = readS16_LE(BME280_REGISTER_DIG_T2);
  _bme280_calib.dig_T3 = readS16_LE(BME280_REGISTER_DIG_T3);

  _bme280_calib.dig_P1 = read16_LE(BME280_REGISTER_DIG_P1);
  _bme280_calib.dig_P2 = readS16_LE(BME280_REGISTER_DIG_P2);
  _bme280_calib.dig_P3 = readS16_LE(BME280_REGISTER_DIG_P3);
  _bme280_calib.dig_P4 = readS16_LE(BME280_REGISTER_DIG_P4);
  _bme280_calib.dig_P5 = readS16_LE(BME280_REGISTER_DIG_P5);
  _bme280_calib.dig_P6 = readS16_LE(BME280_REGISTER_DIG_P6);
  _bme280_calib.dig_P7 = readS16_LE(BME280_REGISTER_DIG_P7);
  _bme280_calib.dig_P8 = readS16_LE(BME280_REGISTER_DIG_P8);
  _bme280_calib.dig_P9 = readS16_LE(BME280_REGISTER_DIG_P9);

  _bme280_calib.dig_H1 = read8(BME280_REGISTER_DIG_H1);
  _bme280_calib.dig_H2 = readS16_LE(BME280_REGISTER_DIG_H2);
  _bme280_calib.dig_H3 = read8(BME280_REGISTER_DIG_H3);
  _bme280_calib.dig_H4 = (read8(BME280_REGISTER_DIG_H4) << 4) | (read8(BME280_REGISTER_DIG_H4+1) & 0xF);
  _bme280_calib.dig_H5 = (read8(BME280_REGISTER_DIG_H5+1) << 4) | (read8(BME280_REGISTER_DIG_H5) >> 4);
  _bme280_calib.dig_H6 = (int8_t)read8(BME280_REGISTER_DIG_H6);
}

void bme280_init()
{

  if (i2c_start(BME280_ADDRESS<<1 | I2C_WRITE) == 0)
  {
    i2c_write(BME280_REGISTER_CONTROLHUMID);
    i2c_write(0x05);

    i2c_write(BME280_REGISTER_CONTROL);
    i2c_write(0xB7);

    i2c_write(BME280_REGISTER_CHIPID);

    bme280_read_coefficients();

    i2c_stop();

    if (i2c_start(BME280_ADDRESS<<1 | I2C_READ) == 0)
    {
      uint8_t val = i2c_read_nack();

      usb_send_byte((char) val);

      i2c_stop();
    }
  }
}

float bme280_read_temperature(void)
{
  int32_t adc_T = read24(BME280_REGISTER_TEMPDATA);

  int32_t var1, var2;

  adc_T >>= 4;

  var1  = ((((adc_T>>3) - ((int32_t)_bme280_calib.dig_T1 <<1))) *
           ((int32_t)_bme280_calib.dig_T2)) >> 11;

  var2  = (((((adc_T>>4) - ((int32_t)_bme280_calib.dig_T1)) *
             ((adc_T>>4) - ((int32_t)_bme280_calib.dig_T1))) >> 12) *
           ((int32_t)_bme280_calib.dig_T3)) >> 14;

  t_fine = var1 + var2;

  float T  = (t_fine * 5 + 128) >> 8;

  return T / 100;
}

double bme280_read_pressure(void)
{
  int64_t var1, var2, p;

  bme280_read_temperature(); // must be done first to get t_fine

  int32_t adc_P = read24(BME280_REGISTER_PRESSUREDATA);

  // extract only the first 20 bits, where pressure is encoded
  adc_P >>= 4;

  var1 = ((double) t_fine) - 64000.0;
  var2 = var1 * var1 * ((double) _bme280_calib.dig_P6) / 32768.0;
  var2 = var2 + var1 * ((double) _bme280_calib.dig_P5) * 2.0;
  var2 = (var2 / 4.0) + (((double) _bme280_calib.dig_P4) * 65536.0);
  var1 = (((double) _bme280_calib.dig_P3) * var1 * var1 / 524288.0
    + ((double) _bme280_calib.dig_P2) * var1) / 524288.0;
  var1 = (1.0 + var1 / 32768.0) * ((double) _bme280_calib.dig_P1);

  if (var1 == 0) {
    return 0;  // avoid exception caused by division by zero
  }

  p = 1048576.0 - (double) adc_P;
  p = (p - (var2 / 4096.0)) * 6250.0 / var1;
  var1 = ((double) _bme280_calib.dig_P9) * p * p / 2147483648.0;
  var2 = p * ((double) _bme280_calib.dig_P8) / 32768.0;

  p = p + (var1 + var2 + ((double) _bme280_calib.dig_P7)) / 16.0;

  return p;
}

float bme280_read_humidity(void)
{
  bme280_read_temperature(); // must be done first to get t_fine

  int32_t adc_H = read16(BME280_REGISTER_HUMIDDATA);

  int32_t v_x1_u32r;

  v_x1_u32r = (t_fine - ((int32_t)76800));

  v_x1_u32r = (((((adc_H << 14) - (((int32_t)_bme280_calib.dig_H4) << 20) -
                  (((int32_t)_bme280_calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
               (((((((v_x1_u32r * ((int32_t)_bme280_calib.dig_H6)) >> 10) *
                    (((v_x1_u32r * ((int32_t)_bme280_calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
                  ((int32_t)2097152)) * ((int32_t)_bme280_calib.dig_H2) + 8192) >> 14));

  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                             ((int32_t)_bme280_calib.dig_H1)) >> 4));

  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  float h = (v_x1_u32r>>12);
  return  h / 1024.0;
}
