#include <dpn/meta/Tag.hpp>

#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	bool parse(std::optional<Tag> &tag, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		if (strange.pop_if('^'))
		{
			gubg::Strange substr;
			MSS(strange.pop_until(substr, ' ') || strange.pop_all(substr));
			substr.strip_right(" \t\n\r");
			tag.emplace();
			if (substr.pop_until(tag->key, ':'))
				substr.pop_all(tag->value);
			else
				substr.pop_all(tag->key);
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Tag &tag)
	{
		os << "[Tag](" << tag.key << ")(" << tag.value << ")";
		return os;
	}
} } 