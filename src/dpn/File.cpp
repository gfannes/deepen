#include <dpn/File.hpp>
#include <dpn/log.hpp>
#include <dpn/read/markdown.hpp>
#include <dpn/read/naft.hpp>

#include <gubg/Strange.hpp>
#include <gubg/mss.hpp>

namespace dpn { 

	bool File::parse(const std::string &str, Format format)
	{
		MSS_BEGIN(bool);

		switch (format)
		{
			case Format::Markdown: MSS(read::markdown(root.childs, str)); break;
			case Format::Naft: MSS(read::naft(root.childs, str)); break;
			case Format::Freemind: MSS(read::freemind(root.childs, str)); break;

			default: MSS(false, log::error() << "Format " << format << " not supported" << std::endl); break;
		}

		MSS_END();
	}

	bool File::interpret()
	{
		MSS_BEGIN(bool);

		bool ok = true;
		auto interpret = [&](auto &node, auto &path){
			// Parse as many meta items as possible and insert into node.metas and some node.my_ fields
			std::string text;
			for (gubg::Strange strange{node.text}; (strange.strip(' '), !strange.empty()); )
			{
				std::optional<meta::State> state;
				AGG(ok, meta::parse(state, strange), return);
				if (state)
				{
					node.metas.push_back(*state);
					node.my_state = *state;
					continue;
				}

				std::optional<meta::Effort> effort;
				AGG(ok, meta::parse(effort, strange), return);
				if (effort)
				{
					node.metas.push_back(*effort);
					node.my_effort = *effort;
					continue;
				}

				std::optional<meta::Duedate> duedate;
				AGG(ok, meta::parse(duedate, strange), return);
				if (duedate)
				{
					node.metas.push_back(*duedate);
					continue;
				}

				std::optional<meta::Urgency> urgency;
				AGG(ok, meta::parse(urgency, strange), return);
				if (urgency)
				{
					node.metas.push_back(*urgency);
					node.my_urgency = *urgency;
					continue;
				}

				std::optional<meta::Command> command;
				AGG(ok, meta::parse(command, strange), return);
				if (command)
				{
					node.metas.push_back(*command);
					continue;
				}

				std::optional<meta::Tag> tag;
				AGG(ok, meta::parse(tag, strange), return);
				if (tag)
				{
					node.metas.push_back(*tag);
					node.my_tags[tag->key] = tag->value;
					node.total_tags[tag->key].insert(tag->value);
					continue;
				}

				strange.pop_all(text);
			}
			node.text = text;
		};
		each_node(interpret, Direction::Push);
		MSS(ok);

		MSS_END();
	}

	// Free functions
	std::ostream &operator<<(std::ostream &os, File::Format format)
	{
		switch (format)
		{
			case File::Format::Markdown: os << "Markdown"; break;
			case File::Format::Naft: os << "Naft"; break;
			case File::Format::Freemind: os << "Freemind"; break;
		}
		return os;
	}

	// Privates

} 