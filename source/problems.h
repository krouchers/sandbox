#pragma once
#include <vector>
#include <problem.h>

class application;

enum class problems_list : uint8_t
{
    CUBE = 0,
    PARAL,
    PYRAMID,
    ICO,
    OCT,
};

extern int answers[5];

class problems
{
    std::vector<const char *> _problems_names = {
        "cube", "3", "pyramid", "ico", "paleh"};
    application *_app;
    size_t _problems_count;
    std::vector<problem> _problems;

public:
    problems(application *app);
    problem &operator[](problems_list problem);
};