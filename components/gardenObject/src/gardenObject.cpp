#include "gardenObject.hpp"

// extenal codes
// NVS
#define ESP_ERR_NVS_BASE 0x1100                                /*!< Starting number of error codes */
#define ESP_ERR_NVS_NOT_INITIALIZED (ESP_ERR_NVS_BASE + 0x01)  /*!< The storage driver is not initialized */
#define ESP_ERR_NVS_NOT_FOUND (ESP_ERR_NVS_BASE + 0x02)        /*!< Id namespace doesn’t exist yet and mode is NVS_READONLY */
#define ESP_ERR_NVS_TYPE_MISMATCH (ESP_ERR_NVS_BASE + 0x03)    /*!< The type of set or get operation doesn't match the type of value stored in NVS */
#define ESP_ERR_NVS_READ_ONLY (ESP_ERR_NVS_BASE + 0x04)        /*!< Storage handle was opened as read only */
#define ESP_ERR_NVS_NOT_ENOUGH_SPACE (ESP_ERR_NVS_BASE + 0x05) /*!< There is not enough space in the underlying storage to save the value */
#define ESP_ERR_NVS_INVALID_NAME (ESP_ERR_NVS_BASE + 0x06)     /*!< Namespace name doesn’t satisfy constraints */
#define ESP_ERR_NVS_INVALID_HANDLE (ESP_ERR_NVS_BASE + 0x07)   /*!< Handle has been closed or is NULL */
#define ESP_ERR_NVS_REMOVE_FAILED (ESP_ERR_NVS_BASE + 0x08)    /*!< The value wasn’t updated because flash write operation has failed. The value was written however, and update will be finished after re-initialization of nvs, provided that flash operation doesn’t fail again. */
#define ESP_ERR_NVS_KEY_TOO_LONG (ESP_ERR_NVS_BASE + 0x09)     /*!< Key name is too long */
#define ESP_ERR_NVS_PAGE_FULL (ESP_ERR_NVS_BASE + 0x0a)        /*!< Internal error; never returned by nvs API functions */
#define ESP_ERR_NVS_INVALID_STATE (ESP_ERR_NVS_BASE + 0x0b)    /*!< NVS is in an inconsistent state due to a previous error. Call nvs_flash_init and nvs_open again, then retry. */
#define ESP_ERR_NVS_INVALID_LENGTH (ESP_ERR_NVS_BASE + 0x0c)   /*!< String or blob length is not sufficient to store data */
#define ESP_ERR_NVS_NO_FREE_PAGES (ESP_ERR_NVS_BASE + 0x0d)    /*!< NVS partition doesn't contain any empty pages. This may happen if NVS partition was truncated. Erase the whole partition and call nvs_flash_init again. */
#define ESP_ERR_NVS_VALUE_TOO_LONG (ESP_ERR_NVS_BASE + 0x0e)   /*!< Value doesn't fit into the entry or string or blob length is longer than supported by the implementation */
#define ESP_ERR_NVS_PART_NOT_FOUND (ESP_ERR_NVS_BASE + 0x0f)   /*!< Partition with specified name is not found in the partition table */

#define ESP_ERR_NVS_NEW_VERSION_FOUND (ESP_ERR_NVS_BASE + 0x10)    /*!< NVS partition contains data in new format and cannot be recognized by this version of code */
#define ESP_ERR_NVS_XTS_ENCR_FAILED (ESP_ERR_NVS_BASE + 0x11)      /*!< XTS encryption failed while writing NVS entry */
#define ESP_ERR_NVS_XTS_DECR_FAILED (ESP_ERR_NVS_BASE + 0x12)      /*!< XTS decryption failed while reading NVS entry */
#define ESP_ERR_NVS_XTS_CFG_FAILED (ESP_ERR_NVS_BASE + 0x13)       /*!< XTS configuration setting failed */
#define ESP_ERR_NVS_XTS_CFG_NOT_FOUND (ESP_ERR_NVS_BASE + 0x14)    /*!< XTS configuration not found */
#define ESP_ERR_NVS_ENCR_NOT_SUPPORTED (ESP_ERR_NVS_BASE + 0x15)   /*!< NVS encryption is not supported in this version */
#define ESP_ERR_NVS_KEYS_NOT_INITIALIZED (ESP_ERR_NVS_BASE + 0x16) /*!< NVS key partition is uninitialized */
#define ESP_ERR_NVS_CORRUPT_KEY_PART (ESP_ERR_NVS_BASE + 0x17)     /*!< NVS key partition is corrupt */
#define ESP_ERR_NVS_WRONG_ENCRYPTION (ESP_ERR_NVS_BASE + 0x19)     /*!< NVS partition is marked as encrypted with generic flash encryption. This is forbidden since the NVS encryption works differently. */

#define ESP_ERR_NVS_CONTENT_DIFFERS (ESP_ERR_NVS_BASE + 0x18) /*!< Internal error; never returned by nvs API functions.  NVS key is different in comparison */

using namespace gardener;

#define X(typeID, name) name,
const char *gardener::g_errToString[]{
    G_ERROR_CODES};
#undef X

g_err gardener::g_err_translate(esp_err_t e)
{
    if (e == ESP_OK)
        return G_OK;
    if (e == ESP_FAIL)
        return G_ERR_HARDWARE;
    if (e == ESP_ERR_TIMEOUT)
        return G_ERR_TIMEOUT;
    if (e == ESP_ERR_INVALID_ARG)
        return G_ERR_INVALID_ARGS;
    if (e == ESP_ERR_INVALID_STATE)
        return G_ERR_INVALID_STATE;
    if (e == ESP_ERR_NVS_NOT_FOUND)
        return G_ERR_KEY_NOT_FOUND;
    if (e == ESP_ERR_NVS_INVALID_HANDLE)
        return G_ERR_INVALID_STATE;
    if (e == ESP_ERR_NVS_INVALID_NAME)
        return G_ERR_INVALID_ARGS;
    if (e == ESP_ERR_NVS_INVALID_LENGTH)
        return G_ERR_NOMEM;

    return G_ERR_UNK;
}

gardenObject::gardenObject(const char *name) : _name(name)
{
    _mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(_mutex);
    _locked = false;
}

bool gardenObject::lock(gardenObject &lockedBy, uint32_t timeout)
{
    if (xSemaphoreTake(_mutex, timeout / portTICK_PERIOD_MS) == pdTRUE)
    {
        _lockedBy = lockedBy._name;
        _locked = true;
        return true;
    }
    return false;
}

void gardener::gardenObject::unlock()
{
    _lockedBy = nullptr;
    _locked = false;
    xSemaphoreGive(_mutex);
}

const char *gardener::gardenObject::lockedBy()
{
    if (_lockedBy)
        return _lockedBy;
    return "NONE";
}

const char *gardener::gardenObject::getName()
{
    return _name;
}
