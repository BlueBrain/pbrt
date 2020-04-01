#ifndef PBRT_VSD_VSDEVENT_H
#define PBRT_VSD_VSDEVENT_H

#include "geometry.h"

namespace pbrt
{

// VSDEvent Declarations
class VSDEvent {
public:
    VSDEvent() { p = Point(0, 0, 0); power = 0.f; }
    VSDEvent(const Point &pos, const float &pow):
        p(pos), power(pow) { }
public:
    Point p;
    float power;
};

}

#endif // PBRT_VSD_VSDEVENT_H
