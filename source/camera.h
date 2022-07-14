#pragma once
#include <glm/vec3.hpp>

class camera
{
    glm::vec3 delta{};

public:
    void rotate(float dx, float dy);
};