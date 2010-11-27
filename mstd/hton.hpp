
#pragma once

#if defined(_MSC_VER)
#include <stdlib.h>
#else
#include <endian.h>
#endif

#include "cstdint.hpp"

namespace mstd {

    namespace detail {

        template<size_t> struct hton_helper;

        template<>
        struct hton_helper<1> {
            template<class T>
            static T apply(T t)
            {
                return t;
            }
        };

        template<>
        struct hton_helper<2> {
            template<class T>
            static T apply(T t)
            {
#if defined(_MSC_VER)
                return _byteswap_ushort(t);
#else
                return htobe16(t);
#endif
            }
        };

        template<>
        struct hton_helper<4> {
            template<class T>
            static T apply(T t)
            {
#if defined(_MSC_VER)
                return _byteswap_ulong(t);
#else
                return htobe32(t);
#endif
            }
        };

        template<>
        struct hton_helper<8> {
            template<class T>
            static T apply(volatile T t)
            {
#if defined(_MSC_VER)
                return _byteswap_uint64(t);
#else
                return htobe64(t);
#endif
            }
        };

    }

    template<class T>
    T hton(T t)
    {
        return detail::hton_helper<sizeof(T)>::apply(t);
    }

    template<class T>
    T ntoh(T t)
    {
        return detail::hton_helper<sizeof(T)>::apply(t);
    }

}
