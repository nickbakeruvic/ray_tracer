#ifndef INTERVAL_H
#define INTERVAL_H

class interval {
  public:
    double min, max;

    RAY_HOST_DEVICE interval() : min(+infinity), max(-infinity) {} // Default interval is empty

    RAY_HOST_DEVICE interval(double min, double max) : min(min), max(max) {}

    RAY_HOST_DEVICE double size() const {
        return max - min;
    }

    RAY_HOST_DEVICE bool contains(double x) const {
        return min <= x && x <= max;
    }

    RAY_HOST_DEVICE bool surrounds(double x) const {
        return min < x && x < max;
    }

    RAY_HOST_DEVICE double clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    static const interval empty, universe;
};

const interval interval::empty    = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

#endif