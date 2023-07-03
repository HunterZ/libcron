#pragma once

#include <map>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "CronLock.h"
#include "Task.h"

namespace libcron
{
    class TaskQueue
    {
        public:

            explicit TaskQueue(
                std::shared_ptr<ICronLock> lock = std::make_shared<NullLock>()
            )
                : lockSptr(lock)
            {
                if (!lockSptr)
                {
                    throw std::invalid_argument("TaskQueue(): lock is null");
                }
            }

            const std::vector<Task>& get_tasks() const
            {
                return c;
            }

            std::vector<Task>& get_tasks()
            {
                return c;
            }

            size_t size() const noexcept
            {
                return c.size();
            }

            bool empty() const noexcept
            {
                return c.empty();
            }

            void push(Task& t)
            {
                c.push_back(std::move(t));
            }

            void push(Task&& t)
            {
                c.push_back(std::move(t));
            }

            void push(std::vector<Task>& tasks_to_insert)
            {
                c.reserve(c.size() + tasks_to_insert.size());
                c.insert(c.end(), std::make_move_iterator(tasks_to_insert.begin()), std::make_move_iterator(tasks_to_insert.end()));
            }

            const Task& top() const
            {
                return c[0];
            }

            Task& at(const size_t i)
            {
                return c[i];
            }

            void sort()
            {
                std::sort(c.begin(), c.end(), std::less<>());
            }

            void clear()
            {
                lockSptr->lock();
                c.clear();
                lockSptr->unlock();
            }

            void remove(Task& to_remove)
            {
                auto it = std::find_if(c.begin(), c.end(), [&to_remove] (const Task& to_compare) {
                                    return to_remove.get_name() == to_compare;
                                    });

                if (it != c.end())
                {
                    c.erase(it);
                }
            }

            void remove(std::string to_remove)
            {
                lockSptr->lock();
                auto it = std::find_if(c.begin(), c.end(), [&to_remove] (const Task& to_compare) {
                                    return to_remove == to_compare;
                                    });
                if (it != c.end())
                {
                    c.erase(it);
                }

                lockSptr->unlock();
            }

            void lock_queue()
            {
                /* Do not allow to manipulate the Queue */
                lockSptr->lock();
            }

            void release_queue()
            {
                /* Allow Access to the Queue Manipulating-Functions */
                lockSptr->unlock();
            }

        private:

            std::shared_ptr<ICronLock> lockSptr;
            std::vector<Task> c;
    };
}
