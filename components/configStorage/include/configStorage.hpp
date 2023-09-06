#ifndef G_CONFIG_STORAGE_H
#define G_CONFIG_STORAGE_H

#include "gardenObject.hpp"

namespace gardener
{

    class configStorage : public gardenObject
    {
    public:
        configStorage(const char *name);
        virtual ~configStorage() {}

    protected:
    private:
    };
} // namespace gardener

#endif