#include "ina219.h"
#include <math.h>

#define ina219_address INA219_ADDR_GND_GND


float i_lsb;

static const float u_shunt_max[] = {
    [INA219_GAIN_1]     = 0.04,
    [INA219_GAIN_0_5]   = 0.08,
    [INA219_GAIN_0_25]  = 0.16,
    [INA219_GAIN_0_125] = 0.32,
};

static esp_err_t read_reg_16(uint8_t reg, uint16_t *val)
{
    uint8_t read_buffer[2];
    uint8_t write_buffer[1] = {reg};

    esp_err_t err = i2c_master_write_read_device(I2C_NUM_0, ina219_address,write_buffer, 1, read_buffer, 2, 1000);
    *val = (read_buffer[0] << 8) | (read_buffer[1]);

    return err;
}

static esp_err_t write_reg_16(uint8_t reg, uint16_t val)
{
    uint8_t write_buffer[3] = {reg,(val >> 8), val & 0xFF};

    esp_err_t err = i2c_master_write_to_device(I2C_NUM_0, ina219_address, write_buffer, 3, 1000);

    return err;
}

esp_err_t ina219_init(void){
    // uint16_t config = (INA219_BUS_RANGE_16V << BIT_BRNG) | 
    //                 (INA219_GAIN_0_125 << BIT_PG0) | 
    //                 (INA219_RES_12BIT_16S << BIT_BADC0) |
    //                 (INA219_RES_12BIT_16S << BIT_SADC0) |
    //                 (INA219_MODE_CONT_SHUNT_BUS << BIT_MODE);
    uint16_t config = 0b0011111001100101;
    return write_reg_16(REG_CONFIG, config);
}

esp_err_t ina219_reset(void){
    return write_reg_16(REG_CONFIG, 1 << BIT_RST);
}

esp_err_t ina219_calibrate(float r_shunt){
    i_lsb = (uint16_t) ( u_shunt_max[INA219_GAIN_0_125] / r_shunt / 32767 * 100000000);
    i_lsb /= 100000000;
    i_lsb /= 0.0001;
    i_lsb = ceil(i_lsb);
    i_lsb *= 0.0001;

    uint16_t cal = (uint16_t)((0.04096) / (i_lsb * r_shunt));

    return write_reg_16(REG_CALIBRATION, cal);
}
esp_err_t ina219_get_current(float *current){
    int16_t raw;
    esp_err_t err = read_reg_16(REG_CURRENT, (uint16_t *)&raw);
    //printf("Raw ADC Value: %i\n", raw);

    *current = raw * i_lsb * .75;

    return err;
}
