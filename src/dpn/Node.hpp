#ifndef HEADER_dpn_Node_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Node_hpp_ALREADY_INCLUDED

#include <dpn/Attribute.hpp>
#include <dpn/meta/State.hpp>
#include <dpn/meta/Effort.hpp>
#include <dpn/meta/Duedate.hpp>
#include <dpn/meta/Prio.hpp>
#include <dpn/meta/Command.hpp>
#include <dpn/meta/Data.hpp>
#include <dpn/log.hpp>

#include <string>
#include <vector>
#include <variant>

namespace dpn { 

	class Node
	{
	public:
		std::string text;
		unsigned int depth = 0;
		Attributes attributes;

		using Meta = std::variant<meta::State, meta::Effort, meta::Duedate, meta::Prio, meta::Command, meta::Data>;
		std::vector<Meta> metas;

		std::vector<Node> childs;

		template <typename Meta>
		const Meta *get() const
		{
			const Meta *ptr = nullptr;
			for (const auto &meta: metas)
			{
				if (auto *p = std::get_if<Meta>(&meta))
				{
					if (ptr)
						log::warning() << "Found multiple occurences for metadata " << meta::Traits<Meta>::type_name() << ", taking last item." << std::endl;
					ptr = p;
				}
			}
			return ptr;
		}
	};

	using Nodes = std::vector<Node>;

} 

#endif