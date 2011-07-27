#pragma once

#include <boost/filesystem/path.hpp>

namespace mstd {

boost::filesystem::wpath executable_path();
void execute_file(const boost::filesystem::wpath & path);

}
