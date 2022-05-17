#pragma once

enum class problems_list : uint8_t;

struct rotation_state
{
    float x, y, z;
};

class app_state
{
public:
    rotation_state rot_state;
    problems_list current_problem;
    int input_answer;
};
