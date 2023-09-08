#include "global.hpp"

using namespace gardener;

// Variables
sysInfo_t global::systemInfo;

// PINs
ioPin *global::OUT1;
ioPin *global::OUT2;
ioPin *global::IN1;
ioPin *global::IN2;
ioPin *global::AIN0;
ioPin *global::AIN1;
ioPin *global::AIN2;
ioPin *global::AIN3;
ioPin *global::AOUT;
ioPin *global::AUX1;
ioPin *global::AUX2;
ioPin *global::LED_RD;
ioPin *global::LED_GN;
ioPin *global::LED_BL;
ioPin *global::LED_WH;
ioPin *global::LED_STAT_RDY;
ioPin *global::LED_STAT_STA;
ioPin *global::LED_STAT_ERR;
// I2C-Ports
i2cPort *global::systemBus;

// internal PINS
ioPin *global::Sense_12V0;
ioPin *global::Sense_3V3;
ioPin *global::Sense_VIN;

// Protocols/Drivers
networkAdapter *global::connectivity;