#include <dpn/meta/Data.hpp>

#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	bool parse(std::optional<Data> &data, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		if (strange.pop_if('^'))
		{
			gubg::Strange substr;
			MSS(strange.pop_until(substr, ' ') || strange.pop_all(substr));
			data.emplace();
			if (substr.pop_until(data->key, ':'))
				substr.pop_all(data->value);
			else
				substr.pop_all(data->key);
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Data &data)
	{
		os << "[Data](" << data.key << ")(" << data.value << ")";
		return os;
	}
} } 