#pragma once
#include <timer.h>

template <typename precision, typename period>
void timer<precision, period>::start()
{
    _start = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
}

template <typename precision, typename period>
void timer<precision, period>::end()
{
    _end = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
}

template <typename precision, typename period>
void timer<precision, period>::begin_counting_for(std::chrono::seconds time)
{
    _counting_for = time;
    start();
}
template <typename precision, typename period>
bool timer<precision, period>::is_expired()
{
    end();
    _duration = _end - _start;
    if (_duration.count() > _counting_for.count())
        return true;
    else
        return false;
}