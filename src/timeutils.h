#ifndef TIMEUTILS_H
#define TIMEUTILS_H
#include <stdint.h>
#include <chrono>

// This returns the milliseconds since the epoch.
uint64_t millis();

// This returns the microseconds since the epoch.
uint64_t micros();

// This returns the nanoseconds since the epoch.
uint64_t nanos();
#endif