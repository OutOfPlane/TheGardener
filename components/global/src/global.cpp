#include "global.hpp"

using namespace gardener;


i2cPort *global::systemBus = nullptr;
i2cPort *global::expansionBus = nullptr;

portExpander *global::extraPins = nullptr;

ioPin *global::pinNetworkStatus = nullptr;
ioPin *global::pinServerStatus = nullptr;
ioPin *global::pinTimeStatus = nullptr;
ioPin *global::pinMDBStatus = nullptr;
ioPin *global::pinBusyStatus = nullptr;
ioPin *global::pinErrorStatus = nullptr;
ioPin *global::pinBtnConfig = nullptr;
ioPin *global::pinUSBOvercurrent = nullptr;

sysInfo_t global::systemInfo;

networkAdapter *global::connectivity = nullptr;