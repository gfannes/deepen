#ifndef HEADER_dpn_List_hpp_ALREADY_INCLUDED
#define HEADER_dpn_List_hpp_ALREADY_INCLUDED

#include <dpn/Node.hpp>

#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace dpn { 

	class List
	{
	public:
		class Item
		{
		public:
			Item(const Node &node): node_(&node) {}

			Path path;
			std::filesystem::path fp;

			const Node &node() const {return *node_;}

			double urgency_value() const { return node_->my_urgency ? node_->my_urgency->value() : 0.0; }
			// Rice is computed on the total effort and is thus stable against changing filter settings
			double rice() const
			{
				double res = 0.0;
				if (const auto todo = node_->total_effort.todo(); todo > 0)
					res = urgency_value()/todo;
				return res;
			}
			unsigned long yyyymmdd() const
			{
				unsigned long res = 0u;
				if (auto *duedate = node_->template get<meta::Duedate>())
					res = duedate->yyyymmdd();
				return res;
			}
			const std::string &text() const { return node_->text; }
			auto state() const
			{
				std::optional<meta::State> res;
				if (auto *state = node_->template get<meta::State>())
					res = *state;
				return res;
			}
			auto my_effort() const { return node_->my_effort; }
			auto filtered_effort() const { return node_->filtered_effort; }

		private:
			const Node *node_ = nullptr;
		};

		std::vector<Item> items;
		meta::Effort effort;

		void clear(){*this = List{};}

		void sort_on_urgency()
		{
			auto compare = [&](const auto &lhs, const auto &rhs){return lhs.urgency_value() > rhs.urgency_value();};
			std::stable_sort(items.begin(), items.end(), compare);
		}
		void sort_on_rice()
		{
			auto compare = [&](const auto &lhs, const auto &rhs){return lhs.rice() > rhs.rice();};
			std::stable_sort(items.begin(), items.end(), compare);
		}
		void sort_on_duedate()
		{
			auto compare = [&](const auto &lhs, const auto &rhs){return lhs.yyyymmdd() > rhs.yyyymmdd();};
			std::stable_sort(items.begin(), items.end(), compare);
		}
		void sort_on_effort()
		{
			auto compare = [&](const auto &lhs, const auto &rhs){return lhs.filtered_effort().todo() > rhs.filtered_effort().todo();};
			std::stable_sort(items.begin(), items.end(), compare);
		}
		void sort(Sort sort)
		{
			switch (sort)
			{
				case Sort::No: break;
				case Sort::Urgency: sort_on_urgency(); break;
				case Sort::Rice: sort_on_rice(); break;
				case Sort::DueDate: sort_on_duedate(); break;
				case Sort::Effort: sort_on_effort(); break;
			}
		}
	};

} 

#endif