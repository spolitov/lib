#pragma once

namespace boost {
    namespace posix_time {
        class ptime;
    }
}

namespace nexus {

typedef int32_t Seconds;
typedef int64_t Milliseconds;

class Clock {
public:
    static Milliseconds milliseconds();
    static boost::posix_time::ptime posix(Milliseconds time) { return timeStart() + boost::posix_time::milliseconds(time); }

    static inline Seconds seconds() { return static_cast<Seconds>(milliseconds() / 1000); }
    static inline boost::posix_time::ptime posixNow() { return posix(milliseconds()); }

    static const boost::posix_time::ptime & timeStart();
};

}
