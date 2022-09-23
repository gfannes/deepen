#include <dpn/plan/Planner.hpp>

#include <gubg/map.hpp>
#include <gubg/mss.hpp>
#include <gubg/hr.hpp>

#include <queue>
#include <sstream>

namespace dpn { namespace plan { 

	bool Planner::load_resources(const std::string &filepath)
	{
		MSS_BEGIN(bool);

		MSS(resources_.load_from_file(filepath));

		MSS_END();
	}

	bool Planner::setup_tasks(const List &nodes, const Id__Id &part_of, const Id__Id &after, const Id__DepIds &requires)
	{
		MSS_BEGIN(bool, "");

		tasks_.resize(nodes.items.size());
		for (Id id = 0u; id < tasks_.size(); ++id)
		{
			const auto &node = nodes.items[id].node();

			auto &task = tasks_[id];
			task.id = id;
			task.desc = node.text;
			task.effort = node.my_effort.todo();
			task.is_feature = !!node.my_urgency;

			gubg::with_value(part_of, id, [&](auto parent_id){
				task.ss.insert(parent_id);
				tasks_[parent_id].ff.insert(id);
			});
			gubg::with_value(after, id, [&](auto prev_id){
				task.fs.insert(prev_id);
			});
			gubg::with_value(requires, id, [&](const auto &deps){
				for (const auto &dep: deps)
					task.ff.insert(dep);
			});
		}

		MSS_END();
	}

	bool Planner::operator()(Plan &plan)
	{
		MSS_BEGIN(bool, "");

		plan.clear();

		std::set<Id> startable, stoppable, blocked;

		auto move_unblocked_tasks = [&](){
			startable.clear();
			stoppable.clear();
			blocked.clear();
			for (const auto &task: tasks_)
			{
				if (!task.started())
				{
					bool can_start = true;
					for (auto oid: task.ss)
					{
						if (!tasks_[oid].started())
							can_start = false;
					}
					for (auto oid: task.fs)
					{
						if (!tasks_[oid].stopped())
							can_start = false;
					}

					if (can_start)
						startable.insert(task.id);
					else
						blocked.insert(task.id);
				}
				else if (!task.stopped())
				{
					bool can_stop = true;
					for (auto oid: task.ff)
					{
						if (!tasks_[oid].stopped())
							can_stop = false;
					}

					if (can_stop)
						stoppable.insert(task.id);
					else
						blocked.insert(task.id);
				}
			}
		};

		double time = 0.0;
		while (true)
		{
			move_unblocked_tasks();
			if (stoppable.empty() && startable.empty())
			{
				if (!blocked.empty())
				{
					for (const auto &task: tasks_)
					{
						if (task.started() && task.stopped())
							std::cout << "Done " << task;
					}
					for (const auto &task: tasks_)
					{
						if (task.started() && !task.stopped())
							std::cout << "Running " << task;
					}
					for (const auto &task: tasks_)
					{
						if (!task.started() && !task.stopped())
							std::cout << "Blocked " << task;
					}
					MSS(false, log::error() << "Could not plan all tasks, cyclic dependencies are present" << std::endl);
				}
				break;
			}

			for (move_unblocked_tasks(); !stoppable.empty(); move_unblocked_tasks())
			{
				for (auto id: stoppable)
				{
					auto &task = tasks_[id];
					time = *task.start + task.effort;
					for (auto oid: task.ff)
					{
						const auto &otask = tasks_[oid];
						MSS(!!otask.stop);
						time = std::max(time, *otask.stop);
					}
					task.stop = time;
					if (task.is_feature)
						std::cout << task;
				}
				stoppable.clear();
			}

			if (!startable.empty())
			{
				auto id = *startable.begin();
				// auto id = *std::prev(startable.end());
				startable.erase(id);

				auto &task = tasks_[id];

				task.start = time;
			}
		}

		MSS_END();
	}

} } 