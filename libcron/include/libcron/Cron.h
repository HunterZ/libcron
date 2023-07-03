#pragma once

#include <chrono>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>

#include "libcron/CronClock.h"
#include "libcron/CronLock.h"
#include "libcron/Task.h"
#include "libcron/TaskQueue.h"

namespace libcron
{
    class Cron
    {
        public:

            // allow specifying nothing, a lock, or a lock + clock
            explicit Cron(
                std::shared_ptr<ICronLock> lock = std::make_shared<NullLock>(),
                std::shared_ptr<ICronClock> clock = std::make_shared<LocalClock>()
            );

            // allow specifying only a clock
            explicit Cron(std::shared_ptr<ICronClock> clock);

            // schedule a callback task under the given name
            bool add_schedule(std::string name, const std::string& schedule, Task::TaskFunction work);

            template<typename Schedules = std::map<std::string, std::string>>
            std::tuple<bool, std::string, std::string>
            add_schedule(const Schedules& name_schedule_map, Task::TaskFunction work);

            // clear scheduled task list
            void clear_schedules();

            // remove task that was scheduled under the given name
            void remove_schedule(const std::string& name);

            // return task count
            size_t count() const;

            // Tick is expected to be called at least once a second to prevent missing schedules.
            size_t tick();

            size_t tick(std::chrono::system_clock::time_point now);

            // returns time until next scheduled task execution, or
            //  std::numeric_limits<std::chrono::minutes>::max() if no tasks
            //  are currently scheduled
            std::chrono::system_clock::duration time_until_next() const;

            // returns a reference to the held clock instance
            // this should not be assumed valid beyond the lifetime of the Cron
            //  instance that returned it
            ICronClock& get_clock() const;

            // commands all tasks to recalculate expiration time
            void recalculate_schedule();

            // return an ordered sequence of scheduled task names and their
            //  respective expiration times
            void get_time_until_expiry_for_tasks(
                std::vector<std::tuple<
                    std::string, std::chrono::system_clock::duration
                > >& status) const;

            friend std::ostream& operator<< (std::ostream& stream, const Cron& c);

        private:

            std::shared_ptr<ICronLock> lockSptr;
            std::shared_ptr<ICronClock> clockSptr;
            TaskQueue tasks{lockSptr};
            bool first_tick = true;
            std::chrono::system_clock::time_point last_tick{};
    };

    template<typename Schedules>
    std::tuple<bool, std::string, std::string> Cron::add_schedule(
        const Schedules& name_schedule_map, Task::TaskFunction work)
    {
        bool is_valid = true;
        std::tuple<bool, std::string, std::string> res{false, "", ""};

        std::vector<Task> tasks_to_add;
        tasks_to_add.reserve(name_schedule_map.size());

        for (auto it = name_schedule_map.begin(); is_valid && it != name_schedule_map.end(); ++it)
        {
            const auto& [name, schedule] = *it;
            auto cron = CronData::create(schedule);
            is_valid = cron.is_valid();
            if (is_valid)
            {
                Task t{std::move(name), CronSchedule{cron}, work };
                if (t.calculate_next(clockSptr->now()))
                {
                    tasks_to_add.push_back(std::move(t));
                }
            }
            else
            {
                std::get<1>(res) = name;
                std::get<2>(res) = schedule;
            }
        }

        // Only add tasks and sort once if all elements in the map where valid
        if (is_valid && tasks_to_add.size() > 0)
        {
            tasks.lock_queue();
            tasks.push(tasks_to_add);
            tasks.sort();
            tasks.release_queue();
        }

        std::get<0>(res) = is_valid;
        return res;
    }

    std::ostream& operator<<(std::ostream& stream, const Cron& c);
}
