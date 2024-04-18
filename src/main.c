#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/dac_cosine.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "i2c-lcd.h"
#include "ina219.h"
#include "drivers.h"
#include "driver/adc.h"
#include "soc.h"


void calculate_state_of_charge(void *param);
void read_battery_voltage(void *param);
void read_current(void *param);
void create_boop(void *param);
// void read_button(void *param);
void print_lcd(void *param);

struct to_lcd {
    float current;
    double soc; 
} to_lcd;

SemaphoreHandle_t i2c_mutex;
QueueHandle_t current_reading;
QueueHandle_t voltage_reading;
QueueHandle_t data_to_lcd;
TaskHandle_t make_sound;
float initial_voltage;


void app_main(void)
{
    ESP_ERROR_CHECK(i2c_init());
    ESP_ERROR_CHECK(ina219_init());
    ESP_ERROR_CHECK(ina219_calibrate(0.1));
    lcd_init();
    printf("Config Done \n");
    i2c_mutex = xSemaphoreCreateMutex();
    current_reading = xQueueCreate(64, sizeof(float));
    voltage_reading = xQueueCreate(64, sizeof(float));
    data_to_lcd = xQueueCreate(3, sizeof(struct to_lcd));
    xTaskCreate(read_battery_voltage, "read_battery_voltage", 3072, NULL, 3, NULL);
    xTaskCreate(read_current, "read_current", 6144, NULL, 3, NULL);
    xTaskCreate(create_boop, "create_boop", 2048, NULL, 3, NULL);
    xTaskCreate(print_lcd, "print_lcd", 2048, NULL, 3, NULL);
    xTaskCreate(calculate_state_of_charge, "calculate_state_of_charge", 4096, NULL, 3, NULL);
}

void calculate_state_of_charge(void *param){
    //Max Voltage 4.2V Min Voltage 2.75V
    vTaskDelay( 1000 / portTICK_PERIOD_MS);
    struct to_lcd data;
    double soc = soc_init(initial_voltage);
    float voltage;
    float current;
    while(1){
        vTaskDelay( 1000 / portTICK_PERIOD_MS);
        xQueueReceive(voltage_reading, &voltage, portMAX_DELAY);
        xQueueReceive(current_reading, &current, portMAX_DELAY);
        soc = soc_cumsum(soc, current);
        printf("soc: %.8f voltage: %.2f current: %.6f\n", soc, voltage, current);
        data.current = current;
        data.soc = soc;
        xQueueSend(data_to_lcd, &data, portMAX_DELAY);
    }

}
void read_battery_voltage(void *param){
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11);
    int value;
    float voltage;
    value = adc1_get_raw(ADC1_CHANNEL_0);
    initial_voltage = (float)value * (5) / (2950);
    while(1){
        vTaskDelay( 1000 / portTICK_PERIOD_MS);
        value = adc1_get_raw(ADC1_CHANNEL_0);
        //printf("ADC Value: %d\n", value);
        voltage = (float)value * (5) / (2950);
        if(voltage > 3.3) voltage *= .985;
        xQueueSend(voltage_reading, &voltage,  portMAX_DELAY);
        //printf("Voltage Value: %.3f\n", voltage);
    }
}
void read_current(void *param){
    float current;
    while(1){
        vTaskDelay( 1000 / portTICK_PERIOD_MS);
        xSemaphoreTake(i2c_mutex, portMAX_DELAY);
        ina219_get_current(&current);
        xSemaphoreGive(i2c_mutex);
        xQueueSend(current_reading, &current,  portMAX_DELAY);
        //printf("The current is: %f\n", current);
    }
}
void create_boop(void *param){
dac_cosine_handle_t dac_handle;

dac_cosine_config_t cosine_cfg = {
    .chan_id = DAC_CHAN_0,
    .freq_hz = 1000,
    .clk_src = DAC_COSINE_CLK_SRC_DEFAULT,
    .offset = 0,
    .phase = DAC_COSINE_PHASE_0,
    .atten = DAC_COSINE_ATTEN_DB_18,
    .flags.force_set_freq = false,
};

dac_cosine_new_channel(&cosine_cfg, &dac_handle);
//dac_cosine_start(dac_handle);
while(1){
    dac_cosine_start(dac_handle);
    vTaskDelay( 1000 / portTICK_PERIOD_MS);
    dac_cosine_stop(dac_handle);
    vTaskDelay( 1000 / portTICK_PERIOD_MS);
}
}

void print_lcd(void *param){
    struct to_lcd old_data = {
        .current = 0,
        .soc = 0,
    };
    struct to_lcd data;
    char test[60] = "";
    while(1){
        vTaskDelay( 1000 / portTICK_PERIOD_MS);
        xQueueReceive(data_to_lcd, &data, portMAX_DELAY);
        if((data.current != old_data.current) || (data.soc != old_data.soc)){
            if (data.soc < 0) data.soc = 0;
            sprintf(test, "Battery:%.2f%%                          Current:%.4f A",data.soc, data.current);
            xSemaphoreTake(i2c_mutex, portMAX_DELAY);
            lcd_clear();
            lcd_send_string(test);
            xSemaphoreGive(i2c_mutex);
        }
        old_data.current = data.current;
        old_data.soc = data.soc;
    }
}
