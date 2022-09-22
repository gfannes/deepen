#include <dpn/plan/Resources.hpp>
#include <dpn/log.hpp>

#include <gubg/naft/Reader.hpp>
#include <gubg/naft/Document.hpp>
#include <gubg/file/system.hpp>
#include <gubg/mss.hpp>

namespace dpn { namespace plan { 

	bool Resources::load_from_file(const std::string &filepath)
	{
		MSS_BEGIN(bool);

		std::string content;
		MSS(gubg::file::read(content, filepath), log::error() << "Could not read resources from " << filepath << std::endl);

		clear();

		using Reader = gubg::naft::Reader;
		Reader reader{content};

		using Item = Reader::Item;
		for (Item item; reader(item); )
		{
			switch (item.what)
			{
				case Item::NodeOpen:
				if (item.text == "resource")
				{
					std::optional<std::string> name_opt;
					std::optional<double> capacity_opt;
					for (; reader(item) && item.what == Item::Attribute; )
					{
						if (false) ;
						else if (item.key("name")) name_opt = item.value();
						else if (item.key("capacity")) capacity_opt = std::stod(item.value());
						else
							MSS(false, log::error() << "Unknown attribute " << item << std::endl);
					}
					MSS(!!name_opt, log::error() << "Expected name attribute" << std::endl);
					MSS(!!capacity_opt, log::error() << "Expected capacity attribute" << std::endl);

					emplace_back(Resource{*name_opt, *capacity_opt});
				}
				else
					MSS(false, log::error() << "Unknown node " << item.text << std::endl);
				break;
			}
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Resources &resources)
	{
		gubg::naft::Document doc{os};
		auto n = doc.node("Resources");
		n.attr("size", resources.size());
		for (const auto &resource: resources)
			n.node("Resource").attr("name", resource.name).attr("capacity", resource.capacity);
		return os;
	}

} } 