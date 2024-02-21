#ifndef G_SAFE_POINTER_H
#define G_SAFE_POINTER_H

#include "gardenObject.hpp"

namespace gardener
{

    class safePointer : public gardenObject
    {
    public:
        safePointer(const char *name, void *targetAddress);
        virtual ~safePointer() {}
        void *buf();

    protected:
        void *_targetAddress;

    private:
        
    };

} // namespace gardener

#endif