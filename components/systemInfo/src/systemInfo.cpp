#include <systemInfo.hpp>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "buildnumber.hpp"
#include "global.hpp"

using namespace gardener;

char firmwareShort[32] = {0};
char firmwareLong[64] = {0};

bool test_PCBrevision_G()
{
    // Revision G has LED on GPIO27 all others have nothing
    // if ADCVal < 3500 -> We have an LED and therefore RevG
    adc2_config_channel_atten(ADC2_CHANNEL_7, ADC_ATTEN_DB_11);
    gpio_set_direction(GPIO_NUM_27, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_27, GPIO_PULLUP_ONLY);
    vTaskDelay(10 / portTICK_RATE_MS);
    int erg = 0;
    if (adc2_get_raw(ADC2_CHANNEL_7, ADC_WIDTH_BIT_12, &erg) != ESP_OK)
    {
        return false;
    }
    return (erg < 3500);
}

bool test_PCBrevision_2_1()
{
    // Revision 2.1 has LED on GPIO14 but none on GPIO27
    // if ADCVal < 3500 -> We have an LED and therefore Rev2.0
    adc2_config_channel_atten(ADC2_CHANNEL_6, ADC_ATTEN_DB_11);
    gpio_set_direction(GPIO_NUM_14, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_14, GPIO_PULLUP_ONLY);
    vTaskDelay(10 / portTICK_RATE_MS);
    int erg = 0;
    if (adc2_get_raw(ADC2_CHANNEL_6, ADC_WIDTH_BIT_12, &erg) != ESP_OK)
    {
        return false;
    }
    return (erg < 3500);
}

bool test_PCBrevision_2_2()
{
    // Revision 2.2 has RTC and Portexpander on I2C0
    bool tca9534_present = false;
    global::systemBus->present(0x23, tca9534_present);
    if(tca9534_present)
    {
        return true;
    }
    return false;
}

sysInfo_t gardener::identifySystem()
{
    sysInfo_t info;

    // Identify the Software
    info.software.IDFVer = IDF_VER;
    info.software.firmware.buildDate = __DATE__;
    info.software.firmware.buildTime = __TIME__;
    info.software.firmware.major = G_CODE_MAJOR;
    info.software.firmware.minor = G_CODE_MINOR;
    info.software.firmware.patch = G_CODE_PATCH;
    info.software.firmware.build = G_CODE_BUILD;

    info.hardware.hardwareID = nullptr;

    getFirmwareStringShort();
    getFirmwareStringLong();

    uint8_t mac[8];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    // Identify the Hardware
    info.hardware.PCBRev = PCB_Revision_Unknown;
    if (test_PCBrevision_G())
    {
        info.hardware.PCBRev = PCB_Revision_G;
    }
    else if (test_PCBrevision_2_1())
    {
        info.hardware.PCBRev = PCB_Revision_2_1;
    }
    else if (test_PCBrevision_2_2())
    {
        info.hardware.PCBRev = PCB_Revision_2_2;
    }

    return info;
}

const char *gardener::getFirmwareStringShort()
{
    if (!firmwareShort[0])
        sprintf(firmwareShort, "%d.%d.%d", G_CODE_MAJOR, G_CODE_MINOR, G_CODE_PATCH);
    return firmwareShort;
}

const char *gardener::getFirmwareStringLong()
{
    if (!firmwareLong[0])
        sprintf(firmwareLong, "%d.%d.%d:%05d", G_CODE_MAJOR, G_CODE_MINOR, G_CODE_PATCH, G_CODE_BUILD);
    return firmwareLong;
}

const char *gardener::getPCBRevisionString(PCBRevision rev)
{
    switch (rev)
    {
    case PCB_Revision_G:
        return "RevG";
    case PCB_Revision_2_1:
        return "Rev2.1";
    case PCB_Revision_2_2:
        return "Rev2.2";

    case PCB_Revision_Unknown:
    default:
        return "Unknown";
    }
}
