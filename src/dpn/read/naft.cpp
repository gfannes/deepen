#include <dpn/read/naft.hpp>
#include <dpn/log.hpp>

#include <gubg/naft/Reader.hpp>
#include <gubg/mss.hpp>

#include <vector>

namespace dpn { namespace read { 

	bool naft(Nodes &nodes, const std::string &text)
	{
		MSS_BEGIN(bool);

		using Reader = gubg::naft::Reader;

		std::vector<Nodes *> nodes_stack;
		nodes_stack.push_back(&nodes);

		Reader reader{text};

		for (Reader::Item item; reader(item); )
		{
			L(item);
			switch (item.what)
			{
				case Reader::Item::NodeOpen:
				{
					auto &node = nodes_stack.back()->emplace_back();
					node.text = item.text;
					node.depth = nodes_stack.size();
					nodes_stack.push_back(&node.childs);
				}
				break;

				case Reader::Item::NodeClose:
				{
					nodes_stack.pop_back();
				}
				break;

				case Reader::Item::Attribute:
				{
					auto &node = nodes_stack.back()->emplace_back();
					node.attributes.emplace_back(item.key(), item.value());
				}
				break;

				case Reader::Item::Text:
				{
					auto &node = nodes_stack.back()->emplace_back();
					node.text = item.text;
				}
				break;
			}
		}
		MSS(!reader.error, log::error() << "Failed to read naft: " << *reader.error);

		MSS_END();
	}

} } 