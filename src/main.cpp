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
#include <timeout.hpp>
#include <sht40.hpp>
#include <esp32nvs.hpp>

#include "webpage.h"

#define __ST(A) #A
#define __STR(A) __ST(A)

using namespace gardener;

void main_task(void *pvParameter);
void updateSensorData();

esp_err_t statusPageHandler(httpd_req_t *req);
esp_err_t configPageHandler(httpd_req_t *req);
esp_err_t dataRequestHandler(httpd_req_t *req);

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
        if (global::LED_STAT_ERR->lock(*global::LED_STAT_ERR))
        {
            global::LED_STAT_ERR->set(1);
            global::LED_STAT_ERR->unlock();
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

wifiAdapter adaptWiFi("WiFi", 10);
char sensorDataBuff[1024];
esp32nvs configStore("nvs");

void main_task(void *pvParameter)
{
    const char *_name = "main";
    vTaskDelay(4000 / portTICK_RATE_MS);
    g_err erg = G_OK;

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
    global::AOUT = new esp32ioPin("AOUT", 12);

    global::AUX1 = myPortExpander.getIOPin(3);
    global::AUX2 = myPortExpander.getIOPin(4);

    global::LED_RD = new esp32ioPin("LED_RD", 13);
    global::LED_GN = new esp32ioPin("LED_GN", 14);
    global::LED_BL = new esp32ioPin("LED_BL", 16);
    global::LED_WH = new esp32ioPin("LED_WH", 17);

    global::MOT_FWD = new esp32ioPin("M_FWD", 25);
    global::MOT_REV = new esp32ioPin("M_REV", 26);

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

    global::environmental = new SHT40("SHT40", global::systemBus);
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
    global::IN1->mode(PIN_INPUT);
    global::IN2->mode(PIN_INPUT);
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
    global::AOUT->mode(PIN_OUTPUT_PWM);
    global::MOT_FWD->mode(PIN_OUTPUT_PWM);
    global::MOT_REV->mode(PIN_OUTPUT_PWM);

    global::AUX1->set(1);
    global::AUX2->set(0);
    global::LED_STAT_RDY->set(0);
    global::LED_STAT_STA->set(0);
    global::LED_STAT_ERR->set(0);

    global::enable12VA->mode(PIN_OUTPUT);
    global::enable12VA->set(1);

    httpClient myClient("http");
    uint16_t bufSz = 256;
    char outBuf[bufSz];

    adaptWiFi.start();

    char mySSID[65] = "VisioVerdis";
    char myPWD[65] = "VisioVerdis";
    size_t len;

    configStore.openNamespace("netw");
    
    len = 65;
    if ((erg = configStore.readStr("ssid", mySSID, len)) == G_OK)
    {
        G_LOGI("Retrieved SSID from nvs");
    }
    else
    {
        G_ERROR_DECODE(erg);
    }

    len = 65;
    if ((erg = configStore.readStr("pwd", myPWD, len)) == G_OK)
    {
        G_LOGI("Retrieved PWD from nvs");
    }
    else
    {
        G_ERROR_DECODE(erg);
    }
    configStore.close();

    G_LOGI("SSID: %s PWD: %s", mySSID, myPWD);

    adaptWiFi.setSSID(mySSID);
    adaptWiFi.setPWD(myPWD);

    char tmpSSID[33];
    sprintf(tmpSSID, "GraviPlant%s", global::systemInfo.hardware.hardwareID);

    adaptWiFi.startSTA(tmpSSID, "");

    global::LED_STAT_STA->set(1);

    adaptWiFi.startConnect();

    global::connectivity = &adaptWiFi;

    captivePortal prtl("portal", adaptWiFi.getNetIFAP());

    prtl.start();

    prtl.onGet("/config", configPageHandler);
    prtl.onGet("/status", statusPageHandler);
    prtl.onPost("/status", statusPageHandler);
    prtl.onGet("/data", dataRequestHandler);

    esp_log_set_vprintf(webLogHandler);

    timeout waterTimeout(0, SECONDS);
    timeout requestTimeout(10, SECONDS);

    while (1)
    {
        if (requestTimeout.isEllapsed())
        {
            requestTimeout.reset();

            int32_t gn, rd, bl, wh;
            if (myClient.getPrintf(outBuf, bufSz, "https://api.graviplant-online.de/v1/light/?sn=%s&AUTOCOMPLETED=true", global::systemInfo.hardware.hardwareID) != G_OK)
            {
                global::LED_STAT_RDY->set(0);
            }
            else
            {
                if (sscanf(outBuf, "%d %d %d %d", &rd, &gn, &bl, &wh) != 4)
                {
                    if (sscanf(outBuf, "%d", &rd) == 1)
                    {
                        G_LOGI("No LIGHT pending");
                        global::LED_STAT_RDY->set(1);
                    }
                    else
                    {
                        G_LOGE("Invalid light format: %s", outBuf);
                    }
                }
                else
                {
                    if (global::LED_RD->lock(*global::LED_RD, 100))
                    {
                        global::LED_RD->setVoltage(rd * 33);
                        global::LED_RD->unlock();
                    }
                    if (global::LED_GN->lock(*global::LED_GN, 100))
                    {
                        global::LED_GN->setVoltage(gn * 33);
                        global::LED_GN->unlock();
                    }
                    if (global::LED_BL->lock(*global::LED_BL, 100))
                    {
                        global::LED_BL->setVoltage(bl * 33);
                        global::LED_BL->unlock();
                    }
                    if (global::LED_WH->lock(*global::LED_WH, 100))
                    {
                        global::LED_WH->setVoltage(wh * 33);
                        global::LED_WH->unlock();
                    }
                }
            }

            char waterCMD;
            int32_t val;
            myClient.getPrintf(outBuf, bufSz, "https://api.graviplant-online.de/v1/water/?sn=%s&AUTOCOMPLETED=true", global::systemInfo.hardware.hardwareID);
            if (sscanf(outBuf, "%c %d", &waterCMD, &val) != 2)
            {
                if (sscanf(outBuf, "%d", &val) == 1)
                {
                    G_LOGI("No WATER pending");
                }
                else
                {
                    G_LOGE("Invalid water format: %s", outBuf);
                }
            }
            else
            {
                if (waterCMD == 'a')
                {
                    if (global::AOUT->lock(*global::AOUT, 100))
                    {
                        global::AOUT->setVoltage((val * 33) / 100);
                        global::AOUT->unlock();
                    }
                }
                else if (waterCMD == 'm')
                {
                    if (val > 0)
                    {
                        if (global::MOT_REV->lock(*global::MOT_REV, 100))
                        {
                            global::MOT_REV->setVoltage(3300);
                            global::MOT_REV->unlock();
                        }
                        if (global::MOT_FWD->lock(*global::MOT_FWD, 100))
                        {
                            global::MOT_FWD->setVoltage(3300 - (val * 33));
                            global::MOT_FWD->unlock();
                        }
                    }
                    else
                    {
                        if (global::MOT_FWD->lock(*global::MOT_FWD, 100))
                        {
                            global::MOT_FWD->setVoltage(3300);
                            global::MOT_FWD->unlock();
                        }
                        if (global::MOT_REV->lock(*global::MOT_REV, 100))
                        {
                            global::MOT_REV->setVoltage(3300 - (-val * 33));
                            global::MOT_REV->unlock();
                        }
                    }
                }
                else if (waterCMD == 'w')
                {
                    if (global::AUX2->lock(*global::AUX2, 100))
                    {
                        global::AUX2->set(1);
                        global::AUX2->unlock();
                    }
                    waterTimeout.setPeriod(val, SECONDS);
                    waterTimeout.reset();
                }
                else
                {
                    G_LOGE("Invalid water command: %s", outBuf);
                }
            }
        }

        if (waterTimeout.isEllapsed())
        {
            if (global::AUX2->lock(*global::AUX2, 100))
            {
                global::AUX2->set(0);
                global::AUX2->unlock();
            }
        }

        vTaskDelay(100 / portTICK_RATE_MS);
        if (global::LED_STAT_ERR->lock(*global::LED_STAT_ERR, 100))
        {
            global::LED_STAT_ERR->set(0);
            global::LED_STAT_ERR->unlock();
        }
    }
}

void updateSensorData()
{
    char tmpSSID[32];
    adaptWiFi.getSSID(tmpSSID);

    int32_t vin, v3v3, v12va, ain0, ain1, ain2, ain3,
        iaux1, iaux2, imot, ird, ign, ibl, iwh,
        vrd, vgn, vbl, vwh, aout, mfwd, mrev, temper, humi;

    uint8_t sin1, sin2, sout1, sout2, saux1, saux2;

    if (global::senseVIN->lock(*global::senseVIN, 100))
    {
        global::senseVIN->getVoltage(vin);
        global::senseVIN->unlock();
    }

    if (global::sense3V3->lock(*global::sense3V3, 100))
    {
        global::sense3V3->getVoltage(v3v3);
        global::sense3V3->unlock();
    }

    if (global::sense12VA->lock(*global::sense12VA, 100))
    {
        global::sense12VA->getVoltage(v12va);
        global::sense12VA->unlock();
    }

    if (global::iSense_AUX1->lock(*global::iSense_AUX1, 100))
    {
        global::iSense_AUX1->getVoltage(iaux1);
        global::iSense_AUX1->unlock();
        iaux1 = iaux1 * 606 / 1000;
    }

    if (global::iSense_AUX2->lock(*global::iSense_AUX2, 100))
    {
        global::iSense_AUX2->getVoltage(iaux2);
        global::iSense_AUX2->unlock();
        iaux2 = iaux2 * 606 / 1000;
    }

    if (global::iSense_MOT->lock(*global::iSense_MOT, 100))
    {
        global::iSense_MOT->getVoltage(imot);
        global::iSense_MOT->unlock();
        imot = imot * 606 / 1000;
    }

    if (global::iSense_LED_RD->lock(*global::iSense_LED_RD, 100))
    {
        global::iSense_LED_RD->getVoltage(ird);
        global::iSense_LED_RD->unlock();
        ird = ird * 606 / 1000;
    }

    if (global::iSense_LED_GN->lock(*global::iSense_LED_GN, 100))
    {
        global::iSense_LED_GN->getVoltage(ign);
        global::iSense_LED_GN->unlock();
        ign = ign * 606 / 1000;
    }

    if (global::iSense_LED_BL->lock(*global::iSense_LED_BL, 100))
    {
        global::iSense_LED_BL->getVoltage(ibl);
        global::iSense_LED_BL->unlock();
        ibl = ibl * 606 / 1000;
    }

    if (global::iSense_LED_WH->lock(*global::iSense_LED_WH, 100))
    {
        global::iSense_LED_WH->getVoltage(iwh);
        global::iSense_LED_WH->unlock();
        iwh = iwh * 606 / 1000;
    }

    if (global::LED_RD->lock(*global::LED_RD, 100))
    {
        global::LED_RD->getVoltage(vrd);
        global::LED_RD->unlock();
    }
    if (global::LED_GN->lock(*global::LED_GN, 100))
    {
        global::LED_GN->getVoltage(vgn);
        global::LED_GN->unlock();
    }
    if (global::LED_BL->lock(*global::LED_BL, 100))
    {
        global::LED_BL->getVoltage(vbl);
        global::LED_BL->unlock();
    }
    if (global::LED_WH->lock(*global::LED_WH, 100))
    {
        global::LED_WH->getVoltage(vwh);
        global::LED_WH->unlock();
    }

    if (global::AIN0->lock(*global::AIN0, 100))
    {
        global::AIN0->getVoltage(ain0);
        global::AIN0->unlock();
    }
    if (global::AIN1->lock(*global::AIN1, 100))
    {
        global::AIN1->getVoltage(ain1);
        global::AIN1->unlock();
    }
    if (global::AIN2->lock(*global::AIN2, 100))
    {
        global::AIN2->getVoltage(ain2);
        global::AIN2->unlock();
    }
    if (global::AIN3->lock(*global::AIN3, 100))
    {
        global::AIN3->getVoltage(ain3);
        global::AIN3->unlock();
    }

    if (global::OUT1->lock(*global::OUT1, 100))
    {
        global::OUT1->get(sout1);
        global::OUT1->unlock();
    }

    if (global::OUT2->lock(*global::OUT2, 100))
    {
        global::OUT2->get(sout2);
        global::OUT2->unlock();
    }

    if (global::IN1->lock(*global::IN1, 100))
    {
        global::IN1->get(sin1);
        global::IN1->unlock();
    }

    if (global::IN2->lock(*global::IN2, 100))
    {
        global::IN2->get(sin2);
        global::IN2->unlock();
    }

    if (global::AUX1->lock(*global::AUX1, 100))
    {
        global::AUX1->get(saux1);
        global::AUX1->unlock();
    }

    if (global::AUX2->lock(*global::AUX2, 100))
    {
        global::AUX2->get(saux2);
        global::AUX2->unlock();
    }

    if (global::AOUT->lock(*global::AOUT, 100))
    {
        global::AOUT->getVoltage(aout);
        global::AOUT->unlock();
    }

    if (global::MOT_FWD->lock(*global::MOT_FWD, 100))
    {
        global::MOT_FWD->getVoltage(mfwd);
        global::MOT_FWD->unlock();
    }

    if (global::MOT_REV->lock(*global::MOT_REV, 100))
    {
        global::MOT_REV->getVoltage(mrev);
        global::MOT_REV->unlock();
    }

    if (global::environmental->lock(*global::environmental, 100))
    {
        global::environmental->getHumidity(humi);
        global::environmental->getTemperature(temper);
        global::environmental->unlock();
    }

    sprintf(sensorDataBuff, R"EOF({
"hwID":"%s",
"wifissid":"%s",
"wifistat":"%s",
"AIN0_mV":%d,
"AIN1_mV":%d,
"AIN2_mV":%d,
"AIN3_mV":%d,
"vin_mV":%d,
"v3v3_mV":%d,
"v12va_mV":%d,
"iaux1_mA":%d,
"iaux2_mA":%d,
"imot_mA":%d,
"ird_mA":%d,
"ign_mA":%d,
"ibl_mA":%d,
"iwh_mA":%d,
"rd_p":%d,
"gn_p":%d,
"bl_p":%d,
"wh_p":%d,
"OUT1_v":%d,
"OUT2_v":%d,
"IN1_v":%d,
"IN2_v":%d,
"AUX1_s":%d,
"AUX2_s":%d,
"AOUT_mV":%d,
"mot_p":%d,
"temp_d":%d,
"hum_r":%d,
"fwvers":"%s",
"fwfeat":"%s"
})EOF",
            global::systemInfo.hardware.hardwareID,
            tmpSSID,
            adaptWiFi.getStateName(),
            ain0,
            ain1,
            ain2,
            ain3,
            vin,
            v3v3,
            v12va,
            iaux1,
            iaux2,
            imot,
            ird,
            ign,
            ibl,
            iwh,
            vrd / 33,
            vgn / 33,
            vbl / 33,
            vwh / 33,
            sout1,
            sout2,
            sin1,
            sin2,
            saux1,
            saux2,
            (aout * 100 / 33),
            (mrev - mfwd) / 33,
            temper,
            humi,
            getFirmwareStringLong(),
            __STR(G_CODE_FEATURE));
}

char pageBuff[1024];

char decodeHex(const char *data)
{
    if ((*data >= '0') && (*data <= '9'))
        return (*data) - '0';
    if ((*data >= 'a') && (*data <= 'f'))
        return ((*data) - 'a') + 10;
    if ((*data >= 'A') && (*data <= 'F'))
        return ((*data) - 'A') + 10;
    return 0;
}

void parseURLencoded(char *data)
{
    char *dataPtr = data;
    char *resPtr = data;

    while (*dataPtr)
    {
        if (*dataPtr == '+')
        {
            *resPtr = ' ';
            resPtr++;
            dataPtr++;
        }
        else if (*dataPtr == '%')
        {
            *resPtr = (decodeHex(dataPtr + 1) << 4) | decodeHex(dataPtr + 2);
            resPtr++;
            dataPtr += 3;
        }
        else
        {
            *resPtr = *dataPtr;
            resPtr++;
            dataPtr++;
        }
    }
    *resPtr = '\0';
}

esp_err_t statusPageHandler(httpd_req_t *req)
{
    captivePortal *prtl = (captivePortal *)req->user_ctx;
    const char *_name = prtl->getName();

    if (req->method == HTTP_POST)
    {
        char postBuff[500];
        /* Truncate if content length larger than the buffer */
        size_t recv_size = req->content_len;
        if (recv_size > sizeof(postBuff) - 1)
        {
            recv_size = sizeof(postBuff) - 1;
        }

        int ret = httpd_req_recv(req, postBuff, recv_size);
        if (ret <= 0)
        { /* 0 return value indicates connection closed */
            /* Check if timeout occurred */
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* In case of timeout one can choose to retry calling
                 * httpd_req_recv(), but to keep it simple, here we
                 * respond with an HTTP 408 (Request Timeout) error */
                httpd_resp_send_408(req);
            }
            /* In case of error, returning ESP_FAIL will
             * ensure that the underlying socket is closed */
            return ESP_FAIL;
        }
        // terminate buffer
        postBuff[recv_size] = '\0';
        printf("%s\r\n", postBuff);

        // change = and & to whitespace
        char *content = postBuff;
        while (*content)
        {
            if (*content == '=')
                *content = ' ';
            if (*content == '&')
                *content = ' ';
            content++;
        }
        printf("%s\r\n", postBuff);

        char id[2];
        char val[256 * 2];

        if (sscanf(postBuff, "%c %255s %c %255s", &id[0], &val[0], &id[1], &val[256]) == 4)
        {
            parseURLencoded(&val[0]);
            parseURLencoded(&val[256]);
            configStore.openNamespace("netw");
            g_err erg = G_OK;
            for (size_t i = 0; i < 2; i++)
            {
                if (id[i] == 's')
                {

                    adaptWiFi.setSSID(&val[256 * i]);
                    if ((erg = configStore.writeStr("ssid", &val[256 * i])) == G_OK)
                    {
                        G_LOGI("Store SSID to nvs");
                    }
                    else
                    {
                        G_ERROR_DECODE(erg);
                    }
                }

                if (id[i] == 'm')
                {
                    adaptWiFi.setPWD(&val[256 * i]);
                    if ((erg = configStore.writeStr("pwd", &val[256 * i])) == G_OK)
                    {
                        G_LOGI("Store PWD to nvs");
                    }
                    else
                    {
                        G_ERROR_DECODE(erg);
                    }
                }
            }
            configStore.close();
            G_LOGI("Updated SSID and PWD");
            adaptWiFi.startConnect();
        }
    }

    httpd_resp_set_hdr(req, "Connection", "close");

    httpd_resp_send_chunk(req, webpageHeader1, HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, "GraviPlant Status", HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, webpageHeader2, HTTPD_RESP_USE_STRLEN);

    sprintf(pageBuff, R"EOF(
<nav>
<b>GraviPlant</b>
%s
</nav>
<div>
<h1>Status</h1>
<button onclick="window.location.href='/config';">
Setup Network
</button>)EOF",
            global::systemInfo.hardware.hardwareID);
    httpd_resp_send_chunk(req, pageBuff, HTTPD_RESP_USE_STRLEN);

    httpd_resp_send_chunk(req, webpageDataTable, HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, webpageTableUpdateScript, HTTPD_RESP_USE_STRLEN);

    httpd_resp_send_chunk(req, "</div>", HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, webpageFooter, HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, nullptr, 0);

    return ESP_OK;
}

esp_err_t configPageHandler(httpd_req_t *req)
{
    captivePortal *prtl = (captivePortal *)req->user_ctx;
    const char *_name = prtl->getName();

    httpd_resp_set_hdr(req, "Connection", "close");

    httpd_resp_send_chunk(req, webpageHeader1, HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, "GraviPlant Config", HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, webpageHeader2, HTTPD_RESP_USE_STRLEN);

    sprintf(pageBuff, R"EOF(
<nav>
<b>GraviPlant</b>
%s
</nav>
<div>
<h1>Config</h1>
<button onclick="window.location.href='/status';">
Back
</button><br>
<form action=/status method=post>
<label>WiFi SSID:</label><input type=text name=s></input>
<label>WiFi PWD:</label><input type=password name=m></input>
<input type=submit value=Start>
</form>)EOF",
            global::systemInfo.hardware.hardwareID);
    httpd_resp_send_chunk(req, pageBuff, HTTPD_RESP_USE_STRLEN);

    httpd_resp_send_chunk(req, "</div>", HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, webpageFooter, HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, nullptr, 0);

    return ESP_OK;
}

esp_err_t dataRequestHandler(httpd_req_t *req)
{
    captivePortal *prtl = (captivePortal *)req->user_ctx;
    const char *_name = prtl->getName();

    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_hdr(req, "Content-Type", "application/json");
    updateSensorData();

    httpd_resp_send_chunk(req, sensorDataBuff, HTTPD_RESP_USE_STRLEN);

    httpd_resp_send_chunk(req, nullptr, 0);

    return ESP_OK;
}