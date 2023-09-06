#ifndef GARDEN_OBJECT_H
#define GARDEN_OBJECT_H

#include <stdint.h>
#include <stdio.h>
#include <cstring>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "gardenError.h"

#define G_LOGV(format, ...) esp_log_write(ESP_LOG_VERBOSE, _name, LOG_FORMAT(V, format), esp_log_timestamp(), _name, ##__VA_ARGS__)
#define G_LOGD(format, ...) esp_log_write(ESP_LOG_DEBUG, _name, LOG_FORMAT(D, format), esp_log_timestamp(), _name, ##__VA_ARGS__)
#define G_LOGI(format, ...) esp_log_write(ESP_LOG_INFO, _name, LOG_FORMAT(I, format), esp_log_timestamp(), _name, ##__VA_ARGS__)
#define G_LOGE(format, ...) esp_log_write(ESP_LOG_ERROR, _name, LOG_FORMAT(E, format), esp_log_timestamp(), _name, ##__VA_ARGS__)

#define G_ERROR_DECODE(x)                                                      \
    do                                                                          \
    {                                                                           \
        G_LOGE("[%s] on %s:%d", g_errToString[x], __FILE__, __LINE__); \
    } while (0)

namespace gardener
{
#define X(typeID, name) typeID,
    enum g_err : size_t
    {
        G_ERROR_CODES
    };
#undef X

    extern const char *g_errToString[];

    g_err g_err_translate(esp_err_t e);
    class gardenObject
    {
    public:
        gardenObject(const char *name);
        bool lock(gardenObject &lockedBy, uint32_t timeout_ms = 0);
        void unlock();
        const char *lockedBy();
        const char *getName();

    protected:
        const char *_name;
        const char *_lockedBy = nullptr;
        bool _locked = false;

    private:
        SemaphoreHandle_t _mutex = NULL;
    };

} // namespace gardener

#endif