#include "libcron/CronClock.h"

#ifdef WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#endif

using namespace std::chrono;

namespace libcron
{
// UTCClock
std::chrono::system_clock::time_point UTCClock::now() const
{
  return std::chrono::system_clock::now();
}

std::chrono::seconds UTCClock::utc_offset(
  std::chrono::system_clock::time_point) const
{
  return 0s;
}

// LocalClock
std::chrono::system_clock::time_point LocalClock::now() const
{
  const auto& now{std::chrono::system_clock::now()};
  return now + utc_offset(now);
}

std::chrono::seconds LocalClock::utc_offset(
  std::chrono::system_clock::time_point now) const
{
#ifdef WIN32
  (void)now;

  TIME_ZONE_INFORMATION tz_info{};
  seconds               offset{0};

  auto res = GetTimeZoneInformation(&tz_info);
  if (res != TIME_ZONE_ID_INVALID)
  {
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms725481(v=vs.85).aspx
    // UTC = local time + bias => local_time = utc - bias, so UTC offset is
    // -bias
    offset = minutes{-tz_info.Bias};
  }
#else
  auto t = system_clock::to_time_t(now);
  tm   tm{};
  localtime_r(&t, &tm);
  seconds offset{tm.tm_gmtoff};
#endif
  return offset;
}
}  // namespace libcron
