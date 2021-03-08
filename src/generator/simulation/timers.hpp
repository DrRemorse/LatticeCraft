#pragma once

#include <chrono>
#include <delegate.hpp>

struct Timers
{
  using id_t      = int;
  using duration_t = std::chrono::microseconds;
  using tickval_t = int;
  using handler_t = delegate<void(int)>;

  /// create a one-shot timer that triggers @when from now
  /// returns a timer id
  static id_t oneshot(duration_t when, handler_t);
  /// create a periodic timer that begins @when and repeats every @period
  static id_t periodic(duration_t period, handler_t);
  static id_t periodic(duration_t when, duration_t period, handler_t);
  // un-schedule timer, and free it
  static void stop(id_t);

  static size_t active();
  static size_t existing();
  static size_t free();

  static void handle();

};

inline Timers::id_t Timers::oneshot(duration_t when, handler_t handler)
{
  return periodic(when, std::chrono::milliseconds(0), handler);
}

inline Timers::id_t Timers::periodic(duration_t period, handler_t handler)
{
  return periodic(period, period, handler);
}
