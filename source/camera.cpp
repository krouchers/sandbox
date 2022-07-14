#include "camera.h"

void camera::rotate(float dx, float dy)
{
    delta += glm::vec3(dy, -dx, 0.f);
}