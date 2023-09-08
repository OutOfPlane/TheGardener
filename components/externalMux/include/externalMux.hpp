#ifndef G_EXTERNAL_MUX_H
#define G_EXTERNAL_MUX_H

#include "gardenObject.hpp"
#include "ioPin.hpp"

namespace gardener
{
    class externalMux : public gardenObject
    {
    public:
        externalMux(const char *name, uint16_t numChannels, ioPin *bit0 = nullptr, ioPin *bit1 = nullptr, ioPin *bit2 = nullptr, ioPin *bit3 = nullptr);
        virtual ~externalMux();
        virtual g_err selectChannel(uint16_t channel);

    protected:
        uint16_t _numChannels = 0;
        uint16_t _numBits = 0;

    private:
        ioPin **_pinList;
    };

} // namespace gardener

#endif