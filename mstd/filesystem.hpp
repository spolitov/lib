#pragma once

#include <boost/algorithm/string/predicate.hpp>

#include <boost/filesystem/operations.hpp>

namespace mstd {

template<class Path>
Path relative_path(const Path & dir, const Path & path)
{
    typename Path::iterator i = path.begin(), j = dir.begin();
    while(i != path.end() && j != dir.end() && boost::iequals(*i, *j))
    {
        ++i;
        ++j;
    }
    if(i == path.begin())
        return path;
    else {
        Path result;
        for(; j != dir.end(); ++j)
            result /= L"..";
        for(; i != path.end(); ++i)
            result /= *i;
        return result;
    }
}

template<class Path>
void create_directories(const Path & path)
{
    Path dummy;
    for(typename Path::iterator i = path.begin(), end = path.end(); i != end; ++i)
    {
        dummy /= *i;
        if(!exists(dummy))
            create_directory(dummy);
    }
}

}
