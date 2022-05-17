#pragma once
#include <chrono>

template <typename precision = std::chrono::seconds::rep, typename period = std::chrono::seconds::period>
class timer
{
public:
    using time_duration_t = std::chrono::duration<precision, period>;
    using time_point_t = std::chrono::time_point<std::chrono::system_clock, time_duration_t>;
    using seconds_t = std::chrono::seconds;

    void begin_counting_for(std::chrono::seconds time);
    bool is_expired();

private:
    void inline start();
    void inline end();

    time_point_t _start;
    time_point_t _end;
    time_duration_t _duration;
    seconds_t _counting_for;
};

using timer_t = timer<>;
#include <timer_impl.h>