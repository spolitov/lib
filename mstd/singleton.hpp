#pragma once

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>

#include "config.hpp"
#include "mutex.hpp"

namespace mstd {

namespace detail {
    MSTD_DECL void MSTD_STDCALL register_cleaner(void (MSTD_STDCALL *cleaner)());
}

void MSTD_STDCALL call_once(boost::uint32_t & flag, void (MSTD_STDCALL *f)());

template<class T>
class singleton : private boost::noncopyable {
public:
    static T & instance()
    {
        mstd::call_once(once_, &create_instance);
        return *instance_;
    }
private:
    static void MSTD_STDCALL create_instance()
    {
        instance_ = new T;
        detail::register_cleaner(&singleton<T>::clean);
    }

    static void MSTD_STDCALL clean()
    {
        delete instance_;
    }

    static T* instance_;
    static mstd::mutex mutex_;
    static boost::uint32_t once_;
};

template<class T>
T* singleton<T>::instance_;

template<class T>
boost::uint32_t singleton<T>::once_ = 0;

#define MSTD_SINGLETON_DECLARATION(T) friend class mstd::singleton<T>;

namespace detail {
    template<class T>
    class default_instance : public singleton<default_instance<T> > {
    public:
        T & value() { return value_; }
    private:
        T value_;
        friend class mstd::singleton<default_instance>;
    };
}

template<class T>
T & default_instance()
{
    return detail::default_instance<T>::instance().value();
}

}
