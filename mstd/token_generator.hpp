#pragma once

#include <boost/scoped_ptr.hpp>

#include "config.hpp"

#include "singleton.hpp"

namespace mstd {

class MSTD_DECL token_generator : public mstd::singleton<token_generator> {
public:
    std::string next(size_t len);

    ~token_generator();
private:
    token_generator();
    
    class Impl;
    boost::scoped_ptr<Impl> impl_;
    
    MSTD_SINGLETON_DECLARATION(token_generator);
};

}
