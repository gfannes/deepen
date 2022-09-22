#include <dpn/read/markdown.hpp>
#include <dpn/log.hpp>

#include <gubg/markdown/Reader.hpp>
#include <gubg/xml/Reader.hpp>
#include <gubg/mss.hpp>
#include <gubg/OnlyOnce.hpp>

#include <vector>

namespace dpn { namespace read { 

	bool markdown(Node &node, const std::string &text)
	{
		MSS_BEGIN(bool);

		using Reader = gubg::markdown::Reader;

		std::vector<Node *> node_stack;
		node_stack.push_back(&node);

		Reader reader{text};

		gubg::OnlyOnce check_for_root_metadata;
		for (Reader::Item item; reader(item); )
		{
			L(item);
			switch (item.what)
			{
				case Reader::Item::HeadingOpen:
				case Reader::Item::BulletOpen:
				{
					if (check_for_root_metadata() && item.level == 0)
					{
						auto &node = *node_stack.back();
						node.text = item.text;
						node.depth = item.level;
						node_stack.push_back(&node);
					}
					else
					{
						auto &node = node_stack.back()->childs.emplace_back();
						node.text = item.text;
						node.depth = item.level;
						node.is_heading = (item.what == Reader::Item::HeadingOpen);
						node_stack.push_back(&node);
					}
				}
				break;

				case Reader::Item::HeadingClose:
				case Reader::Item::BulletClose:
				node_stack.pop_back();
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