#ifndef G_GLOBALS_H
#define G_GLOBALS_H

#include <i2cPort.hpp>
#include <portExpander.hpp>
#include <systemInfo.hpp>
#include <networkAdapter.hpp>

namespace gardener
{
    namespace global
    {
        //Variables
        extern sysInfo_t systemInfo;

        //PINs        
        extern ioPin *pinNetworkStatus;
        extern ioPin *pinServerStatus;
        extern ioPin *pinTimeStatus;
        extern ioPin *pinMDBStatus;
        extern ioPin *pinBusyStatus;
        extern ioPin *pinErrorStatus;
        extern ioPin *pinBtnConfig;
        extern ioPin *pinUSBOvercurrent;

        //I2C-Ports
        extern i2cPort *systemBus;
        extern i2cPort *expansionBus;

        //Hardware
        extern portExpander *extraPins;

        //Protocols/Drivers
        extern networkAdapter *connectivity;



        
    } // namespace global

} // namespace gardener

#endif