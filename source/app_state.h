#pragma once

struct rotation_state
{
    float x, y, z;
};

class app_state
{
public:
    rotation_state rot_state;
};
