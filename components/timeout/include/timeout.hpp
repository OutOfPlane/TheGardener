#ifndef G_TIMEOUT_H
#define G_TIMEOUT_H

#include <stdint.h>
#include <stdio.h>

namespace gardener
{
    enum timeUnit
    {
        MINUTES,
        SECONDS,
        MILLISECONDS,
        MICROSECONDS
    };

    class timeout
    {
    public:
        timeout(int64_t time_in, timeUnit unit);
        void setPeriod(int64_t time_in, timeUnit unit);
        void reset();
        bool isEllapsed();
        int64_t getTimeout_s();
        int64_t getTimeout_ms();
        int64_t getTimeout_us();
        

    protected:

    private:
        int64_t _time_in;
        int64_t _time_out;
    };

} // namespace gardener

#endif