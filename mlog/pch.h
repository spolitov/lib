#if defined(_MSC_VER)
#pragma once
#endif

#if !MLOG_NO_LOGGING

#if defined(_MSC_VER)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#endif

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS)
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#include <string.h>

#include <exception>
#include <iosfwd>
#include <iomanip>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/stream.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

#include <boost/thread.hpp>

#include <mstd/atomic.hpp>
#include <mstd/buffers.hpp>
#include <mstd/cstdint.hpp>
#include <mstd/exception.hpp>
#include <mstd/itoa.hpp>
#include <mstd/reference_counter.hpp>
#include <mstd/reverse_lock.hpp>
#include <mstd/singleton.hpp>
#include <mstd/strings.hpp>
#include <mstd/threads.hpp>
#include <mstd/utf8.hpp>

#endif

#define MLOG_BUILDING
