#include "libcron/TaskQueue.h"

#include <algorithm>
#include <stdexcept>

namespace libcron
{
TaskQueue::TaskQueue(std::shared_ptr<ICronLock> lock) : lockSptr(lock)
{
  if (!lockSptr) { throw std::invalid_argument("TaskQueue(): lock is null"); }
}

const std::vector<Task>& TaskQueue::get_tasks() const
{
  return c;
}

std::vector<Task>& TaskQueue::get_tasks()
{
  return c;
}

size_t TaskQueue::size() const noexcept
{
  return c.size();
}

bool TaskQueue::empty() const noexcept
{
  return c.empty();
}

void TaskQueue::push(Task& t)
{
  c.push_back(t);
}

void TaskQueue::push(Task&& t)
{
  c.push_back(std::move(t));
}

void TaskQueue::push(std::vector<Task>& tasks_to_insert)
{
  c.reserve(c.size() + tasks_to_insert.size());
  c.insert(c.end(),
           std::make_move_iterator(tasks_to_insert.begin()),
           std::make_move_iterator(tasks_to_insert.end()));
}

const Task& TaskQueue::top() const
{
  return c[0];
}

Task& TaskQueue::at(const size_t i)
{
  return c[i];
}

void TaskQueue::sort()
{
  std::sort(c.begin(), c.end(), std::less<>());
}

void TaskQueue::clear()
{
  lockSptr->lock();
  c.clear();
  lockSptr->unlock();
}

void TaskQueue::remove(Task& to_remove)
{
  const std::string& nameToRemove(to_remove.get_name());
  auto               it = std::find_if(c.begin(),
                         c.end(),
                         [&nameToRemove](const Task& to_compare)
                         { return nameToRemove == to_compare; });

  if (it != c.end()) { c.erase(it); }
}

void TaskQueue::remove(const std::string& to_remove)
{
  lockSptr->lock();

  auto it = std::find_if(c.begin(),
                         c.end(),
                         [&to_remove](const Task& to_compare)
                         { return to_remove == to_compare; });

  if (it != c.end()) { c.erase(it); }

  lockSptr->unlock();
}

void TaskQueue::lock_queue() const
{
  /* Do not allow to manipulate the Queue */
  lockSptr->lock();
}

void TaskQueue::release_queue() const
{
  /* Allow Access to the Queue Manipulating-Functions */
  lockSptr->unlock();
}
}  // namespace libcron
