#include "libcron/Cron.h"

namespace libcron
{
Cron::Cron(std::shared_ptr<ICronLock> lock, std::shared_ptr<ICronClock> clock)
  : lockSptr(lock), clockSptr(clock)
{
  if (!lockSptr) { throw std::invalid_argument("Cron(): lock is null"); }
  if (!clockSptr) { throw std::invalid_argument("Cron(): clock is null"); }
}

Cron::Cron(std::shared_ptr<ICronClock> clock)
  : lockSptr(std::make_shared<NullLock>()), clockSptr(clock)
{
  if (!lockSptr) { throw std::invalid_argument("Cron(): lock is null"); }
  if (!clockSptr) { throw std::invalid_argument("Cron(): clock is null"); }
}

bool Cron::add_schedule(std::string        name,
                        const std::string& schedule,
                        Task::TaskFunction work)
{
  auto cron{CronData::create(schedule)};
  if (!cron) { return false; }

  tasks.lock_queue();
  Task t{std::move(name), CronSchedule{*cron}, work};
  if (t.calculate_next(clockSptr->now()))
  {
    tasks.push(std::move(t));
    tasks.sort();
  }
  tasks.release_queue();

  return true;
}

void Cron::clear_schedules()
{
  tasks.clear();
}

void Cron::remove_schedule(const std::string& name)
{
  tasks.remove(name);
}

size_t Cron::count() const
{
  return tasks.size();
}

// Tick is expected to be called at least once a second to prevent missing
// schedules.
size_t Cron::tick()
{
  return tick(clockSptr->now());
}

size_t Cron::tick(std::chrono::system_clock::time_point now)
{
  tasks.lock_queue();
  size_t res = 0;

  if (first_tick) { first_tick = false; }
  else
  {
    constexpr auto one_second    = std::chrono::seconds{1};
    constexpr auto three_hours   = std::chrono::hours{3};
    auto           diff          = now - last_tick;
    auto           absolute_diff = diff >= diff.zero() ? diff : -diff;
    if (absolute_diff < one_second)
    {
      // Only allow time to flow if at least one second has passed since the
      // last tick, either forward or backward.
      now = last_tick;
    }
    else if (absolute_diff >= three_hours)
    {
      // https://linux.die.net/man/8/cron
      // Time changes of more than 3 hours are considered to be corrections to
      // the clock or timezone, and the new time is used immediately.
      for (auto& t : tasks.get_tasks()) { t.calculate_next(now); }
    }
    else
    {
      // Change of less than three hours

      // If time has moved backwards: Since tasks are not rescheduled, they
      // won't run before we're back at least the original point in time which
      // prevents running tasks twice.

      // If time has moved forward, tasks that would have run since last tick
      // will be run.
    }
  }

  last_tick = now;

  for (auto& t : tasks.get_tasks())
  {
    if (t.is_expired(now))
    {
      t.execute(now);
      using namespace std::chrono_literals;
      if (!t.calculate_next(now + 1s)) { tasks.remove(t); }
      res++;
    }
  }

  // Only sort if at least one task was executed
  if (res > 0) { tasks.sort(); }

  tasks.release_queue();
  return res;
}

std::chrono::system_clock::duration Cron::time_until_next() const
{
  return (tasks.empty() ? std::chrono::system_clock::duration::max()
                        : tasks.top().time_until_expiry(clockSptr->now()));
}

ICronClock& Cron::get_clock() const
{
  return *clockSptr;
}

void Cron::recalculate_schedule()
{
  tasks.lock_queue();
  for (auto& t : tasks.get_tasks())
  {
    using namespace std::chrono_literals;
    // Ensure that next schedule is in the future
    t.calculate_next(clockSptr->now() + 1s);
  }
  tasks.release_queue();
}

void Cron::get_time_until_expiry_for_tasks(
  std::vector<std::tuple<std::string, std::chrono::system_clock::duration> >&
    status) const
{
  const auto& now{clockSptr->now()};
  const auto& taskList{tasks.get_tasks()};
  status.clear();
  status.reserve(taskList.size());

  tasks.lock_queue();
  for (auto& t : tasks.get_tasks())
  {
    status.emplace_back(t.get_name(), t.time_until_expiry(now));
  }
  tasks.release_queue();
}

std::ostream& operator<<(std::ostream& stream, const Cron& c)
{
  auto now = c.clockSptr->now();
  std::for_each(c.tasks.get_tasks().cbegin(),
                c.tasks.get_tasks().cend(),
                [&stream, &now](const Task& t)
                { stream << t.get_status(now) << '\n'; });

  return stream;
}
}  // namespace libcron
