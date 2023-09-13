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

ioPin *global::MOT_FWD;
ioPin *global::MOT_REV;
// I2C-Ports
i2cPort *global::systemBus;

// internal PINS
ioPin *global::sense12VA;
ioPin *global::sense3V3;
ioPin *global::senseVIN;

ioPin *global::enable12VA;

ioPin *global::HC4051S0;
ioPin *global::HC4051S1;
ioPin *global::HC4051S2;

ioPin *global::iSense_LED_RD;
ioPin *global::iSense_LED_GN;
ioPin *global::iSense_LED_BL;
ioPin *global::iSense_LED_WH;
ioPin *global::iSense_MOT;
ioPin *global::iSense_AUX1;
ioPin *global::iSense_AUX2;

humidityTemperatureSensor *global::environmental;

// Protocols/Drivers
networkAdapter *global::connectivity;