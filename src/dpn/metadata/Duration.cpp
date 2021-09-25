#include <dpn/metadata/Duration.hpp>
#include <cmath>

namespace dpn { namespace metadata { 

    bool Duration::parse(gubg::Strange &strange)
    {
        if (strange.empty())
            return false;

        unsigned int weeks = 0, days = 0, hours = 0, minutes = 0;
        {
            gubg::Strange tmp;
            if (strange.pop_until(tmp, 'w'))
                tmp.pop_decimal(weeks);
            if (strange.pop_until(tmp, 'd'))
                tmp.pop_decimal(days);
            if (strange.pop_until(tmp, 'h'))
                tmp.pop_decimal(hours);
            if (strange.pop_until(tmp, 'm'))
                tmp.pop_decimal(minutes);
        }
        if (!strange.empty())
            return false;

        days += weeks*5;
        hours += days*8;
        minutes += hours*60;

        this->minutes = minutes;

        return true;
    }

    void Duration::stream(std::ostream &os) const
    {
        unsigned int minutes = std::lround(this->minutes);

        auto divide = [](auto &value, unsigned int divider){
            const unsigned int res = value/divider;
            value = value%divider;
            return res;
        };

        unsigned int hours = divide(minutes, 60);
        unsigned int days = divide(hours, 8);
        unsigned int weeks = divide(days, 5);

        if (weeks > 0)
        {
            os << weeks << 'w';
            hours = 0;
            minutes = 0;
        }
        if (days > 0)
        {
            os << days << 'd';
            minutes = 0;
        }
        if (hours > 0)
            os << hours << 'h';
        if (minutes > 0 || (weeks == 0 && days == 0 && hours == 0))
            os << minutes << 'm';
    }

    Duration &Duration::operator+=(const Duration &rhs)
    {
        minutes += rhs.minutes;
        return *this;
    }

} }
