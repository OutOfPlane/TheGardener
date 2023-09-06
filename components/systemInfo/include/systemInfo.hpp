#ifndef G_SYSTEM_INFO_H
#define G_SYSTEM_INFO_H

#include <stdint.h>
#include <stdio.h>

namespace gardener
{
    enum PCBRevision{
        PCB_Revision_Unknown,
        PCB_Revision_G,
        PCB_Revision_2_1,
        PCB_Revision_2_2
    };

    typedef struct{
        PCBRevision PCBRev;
        char * hardwareID;
        uint8_t hardwareMAC[8];
    } hardwareInfo_t;

    typedef struct{
        struct{
            uint8_t major;
            uint8_t minor;
            uint8_t patch;
            uint32_t build;
            const char * buildDate;
            const char * buildTime;
        } firmware;
        const char * IDFVer;        
    } softwareInfo_t;


    typedef struct{
        hardwareInfo_t hardware;
        softwareInfo_t software;
    } sysInfo_t;

    sysInfo_t identifySystem();
    const char * getFirmwareStringShort();
    const char * getFirmwareStringLong();
    const char * getPCBRevisionString(PCBRevision rev);

} // namespace versionID

#endif