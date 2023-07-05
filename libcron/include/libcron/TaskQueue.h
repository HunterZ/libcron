#pragma once

#include <memory>
#include <string>
#include <vector>

#include "libcron/CronLock.h"
#include "libcron/Task.h"

namespace libcron
{
class TaskQueue
{
public:
  // instantiate a task queue with the given lock, or with a NullLock
  explicit TaskQueue(
    std::shared_ptr<ICronLock> lock = std::make_shared<NullLock>());

  // get read-only reference to task list
  // this method is NOT thread safe
  // return value should not be assumed valid beyond the life of the
  //  TaskQueue instance that provided it
  const std::vector<Task>& get_tasks() const;

  // get a mutable reference to task list
  // this method is NOT thread safe
  // return value should not be assumed valid beyond the life of the
  //  TaskQueue instance that provided it
  std::vector<Task>& get_tasks();

  // return number of tasks in the queue
  // this method is NOT thread safe
  size_t size() const noexcept;

  // return whether task queue is empty
  // this method is NOT thread safe
  bool empty() const noexcept;

  // push a task onto the queue
  // this will likely copy-construct a new instance from the given one
  // does not sort the queue
  // this method is NOT thread safe
  void push(Task& t);

  // move a task onto the queue
  // this will likely move-construct a new instance from the given one
  // does not sort the queue
  // this method is NOT thread safe
  void push(Task&& t);

  // move an ordered sequence of tasks onto the queue
  // does not sort the queue
  // this method is NOT thread safe
  void push(std::vector<Task>& tasks_to_insert);

  // returns a read-only reference to the first queued task
  // does not check for the existence of said task
  // this method is NOT thread safe
  const Task& top() const;

  // returns a mutable reference to the task at the given index
  // does not check for the existence of said task
  // this method is NOT thread safe
  Task& at(const size_t i);

  // sort queued tasks
  // sort order is determined by Task's operator<() method
  // this method is NOT thread safe
  void sort();

  // clear the queue, destroying all contained tasks
  // this method IS thread safe
  void clear();

  // remove first copy of the given task from the queue
  // equivalency is determined by Task's operator==(string, Task)
  //  method
  // this method is NOT thread safe
  void remove(Task& to_remove);

  // remove first task with the given name from the queue
  // equivalency is determined by Task's operator==(string, Task)
  //  method
  // this method IS thread safe
  void remove(const std::string& to_remove);

  // block other lock_queue() or thread safe method calls until the
  //  caller subsequently calls release_queue()
  void lock_queue() const;

  // release lock so that other lock_queue() or thread safe method
  //  calls can occur without being (further) blocked
  void release_queue() const;

private:
  mutable std::shared_ptr<ICronLock> lockSptr;
  std::vector<Task>          c;
};
}  // namespace libcron
