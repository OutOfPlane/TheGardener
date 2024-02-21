#include "safePointer.hpp"

using namespace gardener;

gardener::safePointer::safePointer(const char *name, void *targetAddress)
    : gardenObject(name), _targetAddress(targetAddress)
{
}

void *gardener::safePointer::buf()
{
    return _targetAddress;
}
