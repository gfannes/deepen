#ifndef HEADER_dpn_Node_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Node_hpp_ALREADY_INCLUDED

#include <dpn/enums.hpp>
#include <dpn/types.hpp>
#include <dpn/Attribute.hpp>
#include <dpn/meta/State.hpp>
#include <dpn/meta/Moscow.hpp>
#include <dpn/meta/Sequence.hpp>
#include <dpn/meta/Effort.hpp>
#include <dpn/meta/Duedate.hpp>
#include <dpn/meta/Urgency.hpp>
#include <dpn/meta/Command.hpp>
#include <dpn/meta/Tag.hpp>
#include <dpn/Link.hpp>
#include <dpn/log.hpp>

#include <gubg/std/filesystem.hpp>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <variant>
#include <ostream>

namespace dpn { 

	class Node;

	using Path = std::vector<const Node *>;
	
	class Node
	{
	public:
		enum class Type {Normal, Root, Link};

		Node() {}
		Node(const std::string &text): text(text) {}

		const Node *parent = nullptr;
		Type type = Type::Normal;

		std::string text;
		unsigned int level = 0;
		bool is_heading = false;
		Attributes attributes;

		using Meta = std::variant<meta::State, meta::Moscow, meta::Sequence, meta::Effort, meta::Duedate, meta::Urgency, meta::Command, meta::Tag>;
		std::vector<Meta> metas;

		std::vector<Node> childs;

		std::optional<meta::State> my_state;
		std::optional<meta::State> agg_state;

		std::optional<meta::Moscow> my_moscow;
		meta::Moscow agg_moscow;

		std::optional<meta::Sequence> my_sequence;
		meta::Sequence agg_sequence;

		meta::Effort my_effort;

		meta::Effort filtered_effort;
		meta::Effort total_effort;
		meta::Effort abs_effort;
		std::optional<meta::Effort> diff_effort;
		meta::Effort tmp_effort_;

		std::optional<meta::Urgency> my_urgency;
		meta::Urgency agg_urgency;

		std::set<std::filesystem::path> my_includes;
		std::set<std::filesystem::path> my_requires;
		Links links;
		
		std::set<std::filesystem::path> all_dependencies;

		template <typename Ftor>
		void each_link(Ftor &&ftor, Dependency dep) const
		{
			for (const auto &link: links)
			{
				switch (link.type)
				{
					case Link::Include:
					if (dep == Dependency::Include || dep == Dependency::Mine)
						ftor(link);
					break;

					case Link::Require:
					if (dep == Dependency::Require || dep == Dependency::Mine)
						ftor(link);
					break;
				}
			}
		}

		Tags my_tags;
		TagSets all_tags;

		unsigned int depth(bool include_roots) const;

		std::string path(const Path &, char sep = '/') const;

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

	std::string to_string(const Path &, char sep = '/');

	std::ostream &operator<<(std::ostream &, const Node &);

} 

#endif