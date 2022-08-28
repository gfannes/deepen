#include <dpn/read/markdown.hpp>
#include <dpn/log.hpp>

#include <gubg/markdown/Reader.hpp>
#include <gubg/xml/Reader.hpp>
#include <gubg/mss.hpp>

#include <vector>

namespace dpn { namespace read { 

	bool markdown(Nodes &nodes, const std::string &text)
	{
		MSS_BEGIN(bool);

		using Reader = gubg::markdown::Reader;

		std::vector<Nodes *> nodes_stack;
		nodes_stack.push_back(&nodes);

		Reader reader{text};

		for (Reader::Item item; reader(item); )
		{
			L(item);
			switch (item.what)
			{
				case Reader::Item::HeadingOpen:
				case Reader::Item::BulletOpen:
				{
					auto &node = nodes_stack.back()->emplace_back();
					node.text = item.text;
					node.depth = item.level;
					nodes_stack.push_back(&node.childs);
				}
				break;

				case Reader::Item::HeadingClose:
				case Reader::Item::BulletClose:
				nodes_stack.pop_back();
				break;

				case Reader::Item::Line:
				{
					auto &node = nodes_stack.back()->emplace_back();
					node.text = item.text;
				}
				break;
			}
		}
		MSS(!reader.error, log::error() << "Failed to read markdown: " << *reader.error);

		MSS_END();
	}

	bool freemind(Nodes &nodes, const std::string &text)
	{
		MSS_BEGIN(bool);


		MSS_END();
	}

} } 