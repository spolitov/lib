#pragma once

#ifndef MCRYPT_BUILDING
#include <boost/array.hpp>
#include <boost/intrusive_ptr.hpp>
#endif

#include "Config.h"

namespace boost { namespace filesystem {
    template<class String, class Traits>
    class basic_path;

    struct path_traits;
    typedef basic_path<std::string, path_traits> path;

    struct wpath_traits;
    typedef basic_path<std::wstring, wpath_traits> wpath;
} }

namespace mcrypt {

typedef boost::array<unsigned char, 20> SHADigest;
typedef boost::array<unsigned char, 16> MD5Digest;

class RSA;

typedef boost::intrusive_ptr<RSA> RSAPtr;

}
