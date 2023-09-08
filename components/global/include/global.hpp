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

        //External PINs        
        extern ioPin *OUT1;
        extern ioPin *OUT2;
        extern ioPin *IN1;
        extern ioPin *IN2;
        extern ioPin *AIN0;
        extern ioPin *AIN1;
        extern ioPin *AIN2;
        extern ioPin *AIN3;
        extern ioPin *AOUT;
        extern ioPin *AUX1;
        extern ioPin *AUX2;
        extern ioPin *LED_RD;
        extern ioPin *LED_GN;
        extern ioPin *LED_BL;
        extern ioPin *LED_WH;
        extern ioPin *LED_STAT_RDY;
        extern ioPin *LED_STAT_STA;
        extern ioPin *LED_STAT_ERR;

        //internal PINS
        extern ioPin *sense12VA;
        extern ioPin *sense3V3;
        extern ioPin *senseVIN;

        extern ioPin *enable12VA;

        extern ioPin *HC4051S0;
        extern ioPin *HC4051S1;
        extern ioPin *HC4051S2;

        extern ioPin *iSense_LED_RD;
        extern ioPin *iSense_LED_GN;
        extern ioPin *iSense_LED_BL;
        extern ioPin *iSense_LED_WH;
        extern ioPin *iSense_MOT;
        extern ioPin *iSense_AUX1;
        extern ioPin *iSense_AUX2;


        //I2C-Ports
        extern i2cPort *systemBus;

        //Protocols/Drivers
        extern networkAdapter *connectivity;



        
    } // namespace global

} // namespace gardener

#endif