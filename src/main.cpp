#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include <global.hpp>
#include <systemInfo.hpp>
#include <esp32ioPin.hpp>
#include <tca9534.hpp>
#include <ads7828.hpp>

using namespace gardener;

void main_task(void *pvParameter);

extern "C" void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    xTaskCreate(&main_task, "main_task", 16384, NULL, 5, NULL);
}

void main_task(void *pvParameter)
{
    const char *_name = "main";
    vTaskDelay(4000 / portTICK_RATE_MS);

    

    //------BEGIN-Common HW Config--------------
    global::systemBus = new i2cPort("i2cSys", i2c0, 22, 21);
    //------END-Common HW Config----------------

    for (size_t i = 1; i < 128; i++)
    {
        bool prsnt = false;
        global::systemBus->present(i, prsnt);
        if(prsnt)
        {
            G_LOGI("DEV %02X is present", i);
        }
    }

    TCA9534 myPortExpander("TCA9534", global::systemBus);
    ads7828 myADC("ADS7828", global::systemBus);

    
    
    

    //------BEGIN-IO-Hardware--------------
    global::OUT1 = new esp32ioPin("OUT1", 32);
    global::OUT2 = new esp32ioPin("OUT2", 33);
    global::IN1 = new esp32ioPin("IN1", 34);
    global::IN2 = new esp32ioPin("IN2", 35);
    global::AIN0 = new adcPin("AIN0", 0, &myADC, 4.8);
    global::AIN1 = new adcPin("AIN1", 4, &myADC, 4.8);
    global::AIN2 = new adcPin("AIN2", 1, &myADC, 2);
    global::AIN3 = new adcPin("AIN3", 5, &myADC, 2);

    global::Sense_VIN = new adcPin("VIN_sense", 2, &myADC, 10);
    global::Sense_12V0 = new adcPin("12V0_sense", 6, &myADC, 10);
    global::Sense_3V3 = new adcPin("3V3_sense", 3, &myADC, 2);

    global::LED_STAT_RDY = myPortExpander.getIOPin(7);
    global::LED_STAT_STA = myPortExpander.getIOPin(6);
    global::LED_STAT_ERR = myPortExpander.getIOPin(5);
    //------END-IO-Hardware----------------

    //------BEGIN-Automatic System Configuration--------------
    global::systemInfo = identifySystem();
    printf("Starting TheGardener v. %s\r\n", getFirmwareStringLong());
    printf("Dectedted Hardware: %s\r\n", getPCBRevisionString(global::systemInfo.hardware.PCBRev));
    
    //------END-Automatic System Configuration----------------

    // esp_log_set_vprintf(webLogHandler);

    global::OUT1->mode(PIN_OUTPUT);
    global::OUT2->mode(PIN_OUTPUT);
    global::LED_STAT_RDY->mode(PIN_OUTPUT);
    global::LED_STAT_STA->mode(PIN_OUTPUT);
    global::LED_STAT_ERR->mode(PIN_OUTPUT);

    global::LED_STAT_RDY->set(0);
    global::LED_STAT_STA->set(0);
    global::LED_STAT_ERR->set(0);

    while (1)
    {
        global::OUT2->set(0);
        global::OUT1->set(1);
        vTaskDelay(1000/portTICK_RATE_MS);
        global::OUT2->set(1);
        global::OUT1->set(0);
        vTaskDelay(1000/portTICK_RATE_MS);
        int32_t val_mV = 0;
        global::AIN1->getVoltage(val_mV);
        G_LOGI("Voltage1 %d mV", val_mV);
        global::AIN2->getVoltage(val_mV);
        G_LOGI("Voltage2 %d mV", val_mV);
        global::Sense_3V3->getVoltage(val_mV);
        G_LOGI("3v3: %d mV", val_mV);
    }
    
    
}

