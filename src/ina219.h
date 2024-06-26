#include "drivers.h"
#include "driver/i2c.h"
#include <esp_err.h>


#define INA219_ADDR_GND_GND 0x40 //!< I2C address, A1 pin - GND, A0 pin - GND
#define INA219_ADDR_GND_VS  0x41 //!< I2C address, A1 pin - GND, A0 pin - VS+
#define INA219_ADDR_GND_SDA 0x42 //!< I2C address, A1 pin - GND, A0 pin - SDA
#define INA219_ADDR_GND_SCL 0x43 //!< I2C address, A1 pin - GND, A0 pin - SCL
#define INA219_ADDR_VS_GND  0x44 //!< I2C address, A1 pin - VS+, A0 pin - GND
#define INA219_ADDR_VS_VS   0x45 //!< I2C address, A1 pin - VS+, A0 pin - VS+
#define INA219_ADDR_VS_SDA  0x46 //!< I2C address, A1 pin - VS+, A0 pin - SDA
#define INA219_ADDR_VS_SCL  0x47 //!< I2C address, A1 pin - VS+, A0 pin - SCL
#define INA219_ADDR_SDA_GND 0x48 //!< I2C address, A1 pin - SDA, A0 pin - GND
#define INA219_ADDR_SDA_VS  0x49 //!< I2C address, A1 pin - SDA, A0 pin - VS+
#define INA219_ADDR_SDA_SDA 0x4a //!< I2C address, A1 pin - SDA, A0 pin - SDA
#define INA219_ADDR_SDA_SCL 0x4b //!< I2C address, A1 pin - SDA, A0 pin - SCL
#define INA219_ADDR_SCL_GND 0x4c //!< I2C address, A1 pin - SCL, A0 pin - GND
#define INA219_ADDR_SCL_VS  0x4d //!< I2C address, A1 pin - SCL, A0 pin - VS+
#define INA219_ADDR_SCL_SDA 0x4e //!< I2C address, A1 pin - SCL, A0 pin - SDA
#define INA219_ADDR_SCL_SCL 0x4f //!< I2C address, A1 pin - SCL, A0 pin - SCL

#define REG_CONFIG      0
#define REG_SHUNT_U     1
#define REG_BUS_U       2
#define REG_POWER       3
#define REG_CURRENT     4
#define REG_CALIBRATION 5

#define BIT_RST   15
#define BIT_BRNG  13
#define BIT_PG0   11
#define BIT_BADC0 7
#define BIT_SADC0 3
#define BIT_MODE  0

#define MASK_PG   (3 << BIT_PG0)
#define MASK_BADC (0xf << BIT_BADC0)
#define MASK_SADC (0xf << BIT_SADC0)
#define MASK_MODE (7 << BIT_MODE)
#define MASK_BRNG (1 << BIT_BRNG)

#define DEF_CONFIG 0x399f

/**
 * Bus voltage range
 */
typedef enum {
    INA219_BUS_RANGE_16V = 0, //!< 16V FSR
    INA219_BUS_RANGE_32V      //!< 32V FSR (default)
} ina219_bus_voltage_range_t;

/**
 * PGA gain for shunt voltage
 */
typedef enum {
    INA219_GAIN_1 = 0, //!< Gain: 1, Range: +-40 mV
    INA219_GAIN_0_5,   //!< Gain: 1/2, Range: +-80 mV
    INA219_GAIN_0_25,  //!< Gain: 1/4, Range: +-160 mV
    INA219_GAIN_0_125  //!< Gain: 1/8, Range: +-320 mV (default)
} ina219_gain_t;

/**
 * ADC resolution/averaging
 */
typedef enum {
    INA219_RES_9BIT_1S    = 0,  //!< 9 bit, 1 sample, conversion time 84 us
    INA219_RES_10BIT_1S   = 1,  //!< 10 bit, 1 sample, conversion time 148 us
    INA219_RES_11BIT_1S   = 2,  //!< 11 bit, 1 sample, conversion time 276 us
    INA219_RES_12BIT_1S   = 3,  //!< 12 bit, 1 sample, conversion time 532 us (default)
    INA219_RES_12BIT_2S   = 9,  //!< 12 bit, 2 samples, conversion time 1.06 ms
    INA219_RES_12BIT_4S   = 10, //!< 12 bit, 4 samples, conversion time 2.13 ms
    INA219_RES_12BIT_8S   = 11, //!< 12 bit, 8 samples, conversion time 4.26 ms
    INA219_RES_12BIT_16S  = 12, //!< 12 bit, 16 samples, conversion time 8.51 ms
    INA219_RES_12BIT_32S  = 13, //!< 12 bit, 32 samples, conversion time 17.02 ms
    INA219_RES_12BIT_64S  = 14, //!< 12 bit, 64 samples, conversion time 34.05 ms
    INA219_RES_12BIT_128S = 15, //!< 12 bit, 128 samples, conversion time 68.1 ms
} ina219_resolution_t;

/**
 * Operating mode
 */
typedef enum {
    INA219_MODE_POWER_DOWN = 0, //!< Power-done
    INA219_MODE_TRIG_SHUNT,     //!< Shunt voltage, triggered
    INA219_MODE_TRIG_BUS,       //!< Bus voltage, triggered
    INA219_MODE_TRIG_SHUNT_BUS, //!< Shunt and bus, triggered
    INA219_MODE_DISABLED,       //!< ADC off (disabled)
    INA219_MODE_CONT_SHUNT,     //!< Shunt voltage, continuous
    INA219_MODE_CONT_BUS,       //!< Bus voltage, continuous
    INA219_MODE_CONT_SHUNT_BUS  //!< Shunt and bus, continuous (default)
} ina219_mode_t;

esp_err_t ina219_reset(void);
esp_err_t ina219_init(void);
esp_err_t ina219_calibrate(float r_shunt);
esp_err_t ina219_get_current(float *current);
