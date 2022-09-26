#include <dpn/Link.hpp>

namespace dpn { 

	Link &goc(Links &links, Link::Type type, const std::filesystem::path &fp)
	{
		for (auto &link: links)
		{
			if (link.type == type && link.fp == fp)
				return link;
		}

		auto &link = links.emplace_back();
		link.type = type;
		link.fp = fp;
		return link;
	}

} 