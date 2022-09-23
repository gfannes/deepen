#include <dpn/List.hpp>

namespace dpn { 

	std::ostream &operator<<(std::ostream &os, const List::Item &item)
	{
		os << item.node();
		return os;
	}

} 