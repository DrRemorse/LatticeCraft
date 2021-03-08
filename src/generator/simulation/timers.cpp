#include "timers.hpp"

#include <common.hpp>
#include <map>
#include <vector>

using namespace std::chrono;
typedef Timers::duration_t duration_t;
typedef Timers::handler_t  handler_t;

static inline auto now() noexcept
{
  using namespace std::chrono;
  return duration_cast<microseconds> (high_resolution_clock::now().time_since_epoch());
}

/// internal timer ///

struct SystemTimer
{
  SystemTimer(duration_t t, duration_t p, handler_t cb)
    : time(t), period(p), callback(std::move(cb)) {}

  SystemTimer(SystemTimer&& other)
    : time(other.time), period(other.period),
      callback(std::move(other.callback)),
      already_dead(other.already_dead) {}

  bool is_alive() const noexcept {
    return already_dead == false;
  }
  bool is_oneshot() const noexcept {
    return period.count() == 0;
  }
  void reset() {
    callback.reset();
    already_dead = false;
  }

  duration_t time;
  duration_t period;
  handler_t  callback;
  bool already_dead = false;
};

struct timer_system
{
  void free_timer(Timers::id_t);
  void sched_timer(duration_t when, Timers::id_t);

  std::vector<SystemTimer>  timers;
  std::vector<Timers::id_t> free_timers;
  // timers sorted by timestamp
  std::multimap<duration_t, Timers::id_t> scheduled;
};
static inline timer_system& get() {
  static timer_system system_one;
  return system_one;
}

void timer_system::free_timer(Timers::id_t id)
{
  this->timers[id].reset();
  this->free_timers.push_back(id);
}

Timers::id_t Timers::periodic(duration_t when, duration_t period, handler_t handler)
{
  CC_ASSERT(handler != nullptr, "Callback function cannot be null");
  auto& system = get();
  Timers::id_t id;
  auto real_time = now() + when;

  if (UNLIKELY(system.free_timers.empty()))
  {
    id = system.timers.size();
    // occupy new slot
    system.timers.emplace_back(real_time, period, handler);
  }
  else {
    // get free timer slot
    id = system.free_timers.back();
    system.free_timers.pop_back();

    // occupy free slot
    new (&system.timers[id]) SystemTimer(real_time, period, handler);
  }

  // immediately schedule timer
  system.sched_timer(real_time, id);
  return id;
}

void Timers::stop(Timers::id_t id)
{
  auto& system = get();
  if (UNLIKELY(system.timers.at(id).already_dead)) return;

  auto& timer = system.timers[id];
  // mark as dead already
  timer.already_dead = true;
  // free resources immediately
  timer.callback.reset();
  // search for timer in scheduled
  auto it = system.scheduled.find(timer.time);
  for (; it != system.scheduled.end(); ++it) {
    // found dead timer
    if (id == it->second) {
      // erase from schedule
      system.scheduled.erase(it);
      // free from system
      system.free_timer(id);
      break;
    }
  }
}

size_t Timers::active() {
  return get().scheduled.size();
}
size_t Timers::existing() {
  return get().timers.size();
}
size_t Timers::free() {
  return get().free_timers.size();
}

/// scheduling ///

void Timers::handle()
{
  auto& system = get();

  while (LIKELY(!system.scheduled.empty()))
  {
    auto it         = system.scheduled.begin();
    auto when       = it->first;
    Timers::id_t id = it->second;

    auto ts_now = now();
    if (ts_now >= when) {
      // erase immediately
      system.scheduled.erase(it);

      // call the users callback function
      system.timers[id].callback(id);
      // if the timers struct was modified in callback, eg. due to
      // creating a timer, then the timer reference below would have
      // been invalidated, hence why its BELOW, AND MUST STAY THERE
      auto& timer = system.timers[id];

      // oneshot timers are automatically freed
      if (timer.already_dead || timer.is_oneshot())
      {
        system.free_timer(id);
      }
      else
      {
        // if the timer is recurring, we will simply reschedule it
        // NOTE: we are carefully using (when + period) to avoid drift
        auto new_time = when + timer.period;
        // update timers self-time
        timer.time = new_time;
        // reschedule
        system.scheduled.
          emplace(std::piecewise_construct,
                  std::forward_as_tuple(new_time),
                  std::forward_as_tuple(id));
      }

    } else {
      // exit early, because we have nothing more to do,
      // and there is a deferred handler
      return;
    }
  }
}
void timer_system::sched_timer(duration_t when, Timers::id_t id)
{
  this->scheduled.
    emplace(std::piecewise_construct,
            std::forward_as_tuple(when),
            std::forward_as_tuple(id));
}
