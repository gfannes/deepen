#include <dpn/read/naft.hpp>
#include <dpn/log.hpp>

#include <gubg/naft/Reader.hpp>
#include <gubg/mss.hpp>
#include <gubg/OnlyOnce.hpp>
#include <gubg/string_algo/algo.hpp>

#include <vector>

namespace dpn { namespace read { 

	bool naft(Node &node, const std::string &text)
	{
		MSS_BEGIN(bool);

		using Reader = gubg::naft::Reader;

		std::vector<Node *> node_stack;
		node_stack.push_back(&node);
		auto create_child = [&]() -> Node& {
			return node_stack.back()->childs.emplace_back();
		};

		Reader reader{text};

		gubg::OnlyOnce check_for_root_metadata;
		for (Reader::Item item; reader(item); )
		{
			L(item);
			switch (item.what)
			{
				case Reader::Item::NodeOpen:
				{
					auto &node = create_child();
					node.text = item.text;
					node.level = node_stack.size();
					node_stack.push_back(&node);
				}
				break;

				case Reader::Item::NodeClose:
				{
					node_stack.pop_back();
				}
				break;

				case Reader::Item::Attribute:
				{
					auto &node = create_child();
					node.attributes.emplace_back(item.key(), item.value());
				}
				break;

				case Reader::Item::Text:
				{
					auto &node = (check_for_root_metadata()
						? *node_stack.back()
						: create_child());
					gubg::string_algo::strip_right(item.text, " \t\n\r");
					node.text = item.text;
				}
				break;
			}
		}
		MSS(!reader.error, log::error() << "Failed to read naft: " << *reader.error);

		MSS_END();
	}

} } 