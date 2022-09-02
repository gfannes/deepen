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
		auto interpret = [&](auto &n, auto &path){
			std::string text;
			for (gubg::Strange strange{n.text}; (strange.strip(' '), !strange.empty()); )
			{
				std::optional<meta::State> state;
				AGG(ok, meta::parse(state, strange), return);
				if (state)
				{
					n.metas.push_back(*state);
					continue;
				}

				std::optional<meta::Effort> effort;
				AGG(ok, meta::parse(effort, strange), return);
				if (effort)
				{
					n.metas.push_back(*effort);
					n.my_effort = *effort;
					continue;
				}

				std::optional<meta::Duedate> duedate;
				AGG(ok, meta::parse(duedate, strange), return);
				if (duedate)
				{
					n.metas.push_back(*duedate);
					continue;
				}

				std::optional<meta::Urgency> urgency;
				AGG(ok, meta::parse(urgency, strange), return);
				if (urgency)
				{
					n.metas.push_back(*urgency);
					n.my_urgency = *urgency;
					continue;
				}

				std::optional<meta::Command> command;
				AGG(ok, meta::parse(command, strange), return);
				if (command)
				{
					n.metas.push_back(*command);
					continue;
				}

				std::optional<meta::Tag> tag;
				AGG(ok, meta::parse(tag, strange), return);
				if (tag)
				{
					n.metas.push_back(*tag);
					n.tags[tag->key] = tag->value;
					continue;
				}

				strange.pop_all(text);
			}
			n.text = text;

			if (auto state = n.template get<meta::State>())
			{
				if (state->status == meta::Status::Done)
					n.my_effort.done = n.my_effort.total;
			}

			n.local_effort = n.my_effort;
			for (const auto &child: n.childs)
				n.local_effort += child.local_effort;
		};
		each_node(interpret, Direction::Pull);
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