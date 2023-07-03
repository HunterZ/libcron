#pragma once

#include <mutex>

namespace libcron
{
  class ICronLock
  {
    public:
      virtual void lock() = 0;
      virtual void unlock() = 0;
  };

  class NullLock : public ICronLock
  {
    public:
      void lock() override {}
      void unlock() override {}
  };

  class Locker : public ICronLock
  {
    public:
      void lock() override { m.lock(); }
      void unlock() override { m.unlock(); }
    private:
      std::recursive_mutex m{};
  };
}
