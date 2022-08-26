#include <dpn/read/markdown.hpp>

#include <gubg/markdown/Reader.hpp>
#include <gubg/naft/Reader.hpp>
#include <gubg/xml/Reader.hpp>
#include <gubg/mss.hpp>

#include <vector>

namespace dpn { namespace read { 

	bool markdown(onto::Nodes &nodes, const std::string &str)
	{
		MSS_BEGIN(bool, "");

		using Reader = gubg::markdown::Reader;

		std::vector<onto::Nodes *> nodes_stack;
		nodes_stack.push_back(&nodes);

		Reader reader{str};

		for (Reader::Item item; reader(item); )
		{
			switch (item.what)
			{
				case Reader::Item::Heading:
				break;
			}
		}
		MSS(!reader.error, log::error() << "Failed to read markdown: " << *reader.error);

		MSS_END();
	}

	bool naft(onto::Nodes &nodes, const std::string &str)
	{
		MSS_BEGIN(bool);


		MSS_END();
	}

	bool freemind(onto::Nodes &nodes, const std::string &str)
	{
		MSS_BEGIN(bool);


		MSS_END();
	}

} } 