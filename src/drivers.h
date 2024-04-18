#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/dac_oneshot.h"
#include "driver/i2c.h"
#include "esp_adc/adc_continuous.h"

#define SDA_PIN GPIO_NUM_21
#define SCL_PIN GPIO_NUM_22

esp_err_t i2c_init(void);