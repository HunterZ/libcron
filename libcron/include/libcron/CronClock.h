#pragma once

#include <chrono>

namespace libcron
{
class ICronClock
{
public:
  virtual std::chrono::system_clock::time_point now() const = 0;
  virtual std::chrono::seconds                  utc_offset(
                     std::chrono::system_clock::time_point now) const = 0;
};

class UTCClock : public ICronClock
{
public:
  std::chrono::system_clock::time_point now() const override;

  std::chrono::seconds utc_offset(
    std::chrono::system_clock::time_point) const override;
};

class LocalClock : public ICronClock
{
public:
  std::chrono::system_clock::time_point now() const override;

  std::chrono::seconds utc_offset(
    std::chrono::system_clock::time_point now) const override;
};
}  // namespace libcron
