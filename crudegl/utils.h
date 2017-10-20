#pragma once

#include <string>


namespace crudegl
{


namespace utils
{


namespace fs
{


const char PATH_SEPARATOR =
#ifdef _WIN32
'\\';
#else
'/';
#endif


inline std::string join(const std::string& base, const std::string& path)
{
    return base + PATH_SEPARATOR + path;
}


inline std::string dirname(const std::string& path, const std::string& delims = "/\\")
{
    return path.substr(0, path.find_last_of(delims));
}


inline std::string basename(const std::string& path, const std::string& delims = "/\\")
{
    return path.substr(path.find_last_of(delims) + 1);
}


inline std::string noextension(const std::string& filename)
{
    typename std::string::size_type const p(filename.find_last_of('.'));
    return p > 0 && p != std::string::npos ? filename.substr(0, p) : filename;
}


}  // namespace fs


}  // namespace utils


}  // namespace crudegl
