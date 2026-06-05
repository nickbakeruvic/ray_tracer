#ifndef RAY_H
#define RAY_H

#include "vec3.h"

// p(t) = orig + t * direction
class ray {
    public:
        RAY_HOST_DEVICE ray() {}

        RAY_HOST_DEVICE ray(const point3& origin, const vec3& direction) : orig(origin), dir(direction) {}

        RAY_HOST_DEVICE const point3& origin() const { return orig; }
        RAY_HOST_DEVICE const vec3& direction() const { return dir; }

        RAY_HOST_DEVICE point3 at(double t) const {
            return orig + t*dir;
        }

    private:
        point3 orig;
        vec3 dir;
};

#endif