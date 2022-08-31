#ifndef HEADER_dpn_List_hpp_ALREADY_INCLUDED
#define HEADER_dpn_List_hpp_ALREADY_INCLUDED

#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace dpn { 

	class List
	{
	public:
		struct Item
		{
			std::string text;
			std::filesystem::path fp;
			std::optional<meta::State> state;
			double prio = 1.0;
			unsigned int total_minutes = 60;
			double rice() const {return prio/(total_minutes/60.0);}
			std::optional<unsigned long> yyyymmdd;
		};

		std::vector<Item> items;

		void clear(){*this = List{};}

		void sort_on_rice()
		{
			auto compare = [&](const auto &lhs, const auto &rhs){return lhs.rice() > rhs.rice();};
			std::stable_sort(items.begin(), items.end(), compare);
		}
		void sort_on_duedate()
		{
			auto compare = [&](const auto &lhs, const auto &rhs){return lhs.yyyymmdd > rhs.yyyymmdd;};
			std::stable_sort(items.begin(), items.end(), compare);
		}
	};

} 

#endif