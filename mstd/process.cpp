#include <boost/config.hpp>

#include "filesystem.hpp"

#if BOOST_WINDOWS
#include <Windows.h>
#else

#include "strings.hpp"

#if __APPLE__
#include <sys/syslimits.h>
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

#endif

#include "process.hpp"

namespace mstd {

boost::filesystem::wpath executable_path()
{
#if BOOST_WINDOWS
    HMODULE mh = GetModuleHandle(NULL);		
    wchar_t buffer[MAX_PATH + 2];
    GetModuleFileNameW(mh, buffer, MAX_PATH + 1);
    return boost::filesystem::wpath(buffer);
#elif __APPLE__
    char fn[PATH_MAX + 1];
    uint32_t pathlen = PATH_MAX;
    _NSGetExecutablePath(fn, &pathlen);
    return boost::filesystem::wpath(mstd::deutf8(fn));
#else
    char buf[0x100 + 1];
    ssize_t size = readlink("/proc/self/exe", buf, sizeof(buf));
    buf[size] = 0;
    return boost::filesystem::wpath(mstd::deutf8(buf));
#endif
}

void execute_file(const boost::filesystem::wpath & path)
{
#if BOOST_WINDOWS
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    CreateProcessW(NULL, const_cast<wchar_t*>(wfname(path).c_str()), NULL, NULL, true, 0, NULL, NULL, &si, &pi);
#else
    std::string fname = apifname(path).c_str();
    if(!vfork())
    {
        execl(fname.c_str(), NULL, NULL);
        _exit(0);
    }
#endif
}

}
