#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <cstdlib>

#include <cuda_runtime.h>

#define RAY_HOST_DEVICE __host__ __device__

using std::make_shared;
using std::shared_ptr;

// Constants
constexpr double infinity = std::numeric_limits<double>::infinity();
constexpr double pi = 3.1415926535897932385;

// Utility Functions
RAY_HOST_DEVICE inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

// [0, 1)
RAY_HOST_DEVICE inline double random_double() {
    // Returns a random real in [0,1).
    return std::rand() / (RAND_MAX + 1.0);
}
// [min, max)
RAY_HOST_DEVICE inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max - min) * random_double();
}

// Common headers
#include "vec3.h"
#include "color.h"
#include "interval.h"
#include "ray.h"

#endif