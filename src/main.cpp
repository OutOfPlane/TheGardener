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
#include <externalMux.hpp>
#include <muxedPin.hpp>
#include <httpClient.hpp>
#include <wifiAdapter.hpp>
#include <captivePortal.hpp>

using namespace gardener;

void main_task(void *pvParameter);

httpClient errClient("EHTTP");
int webLogHandler(const char *pattern, va_list lst)
{
    if (pattern[0] == '\033' && pattern[7] == 'E')
    {
        if (errClient.lock(errClient))
        {
            char msg[512];
            vsprintf(msg, pattern + 9, lst);
            if (errClient.getPrintf(msg, 511, "https://api.graviplant-online.de/v1/reportError/?sn=%s&msg=%s",
                                    global::systemInfo.hardware.hardwareID,
                                    msg) == G_OK)
                printf("*");
            errClient.unlock();
        }
    }
    return vprintf(pattern, lst);
}

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
        if (prsnt)
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

    global::AUX1 = myPortExpander.getIOPin(3);
    global::AUX2 = myPortExpander.getIOPin(4);

    global::LED_RD = new esp32ioPin("LED_RD", 14);
    global::LED_GN = new esp32ioPin("LED_GN", 13);
    global::LED_BL = new esp32ioPin("LED_BL", 16);
    global::LED_WH = new esp32ioPin("LED_WH", 17);

    global::senseVIN = new adcPin("VIN_sense", 2, &myADC, 10);
    global::sense12VA = new adcPin("12VA_sense", 6, &myADC, 10);
    global::sense3V3 = new adcPin("3V3_sense", 3, &myADC, 2);

    global::enable12VA = new esp32ioPin("12VA_EN", 4);

    global::HC4051S0 = myPortExpander.getIOPin(0);
    global::HC4051S1 = myPortExpander.getIOPin(1);
    global::HC4051S2 = myPortExpander.getIOPin(2);
    externalMux myMux("HC4051", 8, global::HC4051S0, global::HC4051S1, global::HC4051S2);

    adcPin muxADC("muxADC", 7, &myADC, 1);
    global::iSense_LED_RD = new muxedPin("iADC_RD", &muxADC, &myMux, 0);
    global::iSense_LED_GN = new muxedPin("iADC_GN", &muxADC, &myMux, 1);
    global::iSense_LED_BL = new muxedPin("iADC_BL", &muxADC, &myMux, 2);
    global::iSense_LED_WH = new muxedPin("iADC_WH", &muxADC, &myMux, 3);
    global::iSense_MOT = new muxedPin("iADC_MOT", &muxADC, &myMux, 4);
    global::iSense_AUX1 = new muxedPin("iADC_AUX1", &muxADC, &myMux, 5);
    global::iSense_AUX2 = new muxedPin("iADC_AUX2", &muxADC, &myMux, 6);

    global::LED_STAT_RDY = myPortExpander.getIOPin(7);
    global::LED_STAT_STA = myPortExpander.getIOPin(6);
    global::LED_STAT_ERR = myPortExpander.getIOPin(5);
    //------END-IO-Hardware----------------

    //------BEGIN-Automatic System Configuration--------------
    global::systemInfo = identifySystem();
    printf("Starting TheGardener v. %s\r\n", getFirmwareStringLong());
    printf("Dectedted Hardware: %s\r\n", getPCBRevisionString(global::systemInfo.hardware.PCBRev));
    uint8_t buf[8];
    esp_read_mac(buf, ESP_MAC_WIFI_STA);

    asprintf(&global::systemInfo.hardware.hardwareID, "%02X%02X%02X%02X%02X%02X",
             buf[0], buf[1], buf[2],
             buf[3], buf[4], buf[5]);

    G_LOGI("My ID is: %s", global::systemInfo.hardware.hardwareID);

    //------END-Automatic System Configuration----------------

    // esp_log_set_vprintf(webLogHandler);

    global::OUT1->mode(PIN_OUTPUT);
    global::OUT2->mode(PIN_OUTPUT);
    global::AUX1->mode(PIN_OUTPUT);
    global::AUX2->mode(PIN_OUTPUT);
    global::LED_STAT_RDY->mode(PIN_OUTPUT);
    global::LED_STAT_STA->mode(PIN_OUTPUT);
    global::LED_STAT_ERR->mode(PIN_OUTPUT);
    global::HC4051S0->mode(PIN_OUTPUT);
    global::HC4051S1->mode(PIN_OUTPUT);
    global::HC4051S2->mode(PIN_OUTPUT);

    global::LED_RD->mode(PIN_OUTPUT_PWM);
    global::LED_GN->mode(PIN_OUTPUT_PWM);
    global::LED_BL->mode(PIN_OUTPUT_PWM);
    global::LED_WH->mode(PIN_OUTPUT_PWM);

    global::AUX1->set(0);
    global::AUX2->set(0);
    global::LED_STAT_RDY->set(0);
    global::LED_STAT_STA->set(0);
    global::LED_STAT_ERR->set(0);

    global::enable12VA->mode(PIN_OUTPUT);
    global::enable12VA->set(1);

    httpClient myClient("http");
    uint16_t bufSz = 256;
    char outBuf[bufSz];

    wifiAdapter adapt("WiFi", 10);

    // adapt.setSSID("PeanutPay");
    // adapt.setPWD("PeanutPay");
    // adapt.start();
    // adapt.startConnect();
    // adapt.waitConnected();

    adapt.startSTA(global::systemInfo.hardware.hardwareID, "");

    captivePortal prtl("portal", adapt.getNetIFAP());

    prtl.start();

    esp_log_set_vprintf(webLogHandler);

    while (1)
    {
        // int32_t val_mV = 0;
        // global::AIN2->getVoltage(val_mV);
        // global::LED_GN->setVoltage(3300);
        // vTaskDelay(1000 / portTICK_RATE_MS);

        // global::iSense_LED_RD->getVoltage(val_mV);
        // G_LOGI("i_RD: %d mV", val_mV);
        // vTaskDelay(100 / portTICK_RATE_MS);
        // global::iSense_LED_RD->getVoltage(val_mV);
        // G_LOGI("i_RD: %d mV", val_mV);

        // global::iSense_LED_GN->getVoltage(val_mV);
        // G_LOGI("i_GN: %d mV", val_mV);
        // vTaskDelay(100 / portTICK_RATE_MS);
        // global::iSense_LED_GN->getVoltage(val_mV);
        // G_LOGI("i_GN: %d mV", val_mV);

        // global::iSense_LED_BL->getVoltage(val_mV);
        // G_LOGI("i_BL: %d mV", val_mV);
        // vTaskDelay(100 / portTICK_RATE_MS);
        // global::iSense_LED_BL->getVoltage(val_mV);
        // G_LOGI("i_BL: %d mV", val_mV);

        // global::iSense_LED_WH->getVoltage(val_mV);
        // G_LOGI("i_WH: %d mV", val_mV);
        // vTaskDelay(100 / portTICK_RATE_MS);
        // global::iSense_LED_WH->getVoltage(val_mV);
        // G_LOGI("i_WH: %d mV", val_mV);

        // myClient.getPrintf(outBuf, bufSz, "https://api.graviplant-online.de/v1/water/?sn=%s", global::systemInfo.hardware.hardwareID);
        // printf("%s\r\n", outBuf);

        // myClient.getPrintf(outBuf, bufSz, "https://api.graviplant-online.de/v1/light/?sn=%s", global::systemInfo.hardware.hardwareID);
        // printf("%s\r\n", outBuf);

        vTaskDelay(1000 / portTICK_RATE_MS);

        // G_ERROR_DECODE(G_ERR_NO_IMPLEMENTATION);

        //data url: https://api.graviplant-online.de/v1/telemetry/?sn=test&temp=23.5&humid=80.5

        // global::OUT2->set(0);
        // global::OUT1->set(1);
        // vTaskDelay(1000/portTICK_RATE_MS);
        // global::LED_GN->setVoltage(1000);
        // global::OUT2->set(1);
        // global::OUT1->set(0);
        // vTaskDelay(1000/portTICK_RATE_MS);
        // global::LED_GN->setVoltage(500);
        // int32_t val_mV = 0;
        // global::AIN1->getVoltage(val_mV);
        // G_LOGI("Voltage1 %d mV", val_mV);
        // global::AIN2->getVoltage(val_mV);
        // G_LOGI("Voltage2 %d mV", val_mV);
        // global::sense3V3->getVoltage(val_mV);
        // G_LOGI("3v3: %d mV", val_mV);
        // global::sense12VA->getVoltage(val_mV);
        // G_LOGI("12VA: %d mV", val_mV);
        // global::senseVIN->getVoltage(val_mV);
        // G_LOGI("VIN: %d mV", val_mV);
    }
}
