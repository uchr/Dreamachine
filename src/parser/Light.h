#pragma once

#include "Geometry.h"

namespace parser {

struct PointLight {
    Vector3 color;
    Vector3 position;
    float intencity;
};

} // namespace parser
