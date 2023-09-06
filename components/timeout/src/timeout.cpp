#include "timeout.hpp"
#include "esp_timer.h"

using namespace gardener;

timeout::timeout(int64_t time_in, timeUnit unit)
{
    setPeriod(time_in, unit);
    reset();
}

void gardener::timeout::setPeriod(int64_t time_in, timeUnit unit)
{
    switch (unit)
    {
    case MINUTES:
        _time_in = time_in * 1000 * 1000 * 60;
        break;
    
    case SECONDS:
        _time_in = time_in * 1000 * 1000;
        break;

    case MILLISECONDS:
        _time_in = time_in * 1000;
        break;

    case MICROSECONDS:
        _time_in = time_in;
        break;
    
    default:
        break;
    }
}

void timeout::reset()
{
    _time_out = esp_timer_get_time() + _time_in;
}

bool timeout::isEllapsed()
{
    return (esp_timer_get_time() > _time_out);
}

int64_t gardener::timeout::getTimeout_s()
{
    return _time_in/(1000 * 1000);
}

int64_t gardener::timeout::getTimeout_ms()
{
    return _time_in/(1000);
}

int64_t gardener::timeout::getTimeout_us()
{
    return _time_in;
}
