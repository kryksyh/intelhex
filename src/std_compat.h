#ifndef STD_COMPAT_H
#define STD_COMPAT_H

#ifdef __cpp_lib_filesystem
    #include <filesystem>
    using fs = std::filesystem;
#elif __cpp_lib_experimental_filesystem
    #include <experimental/filesystem>
    using fs = std::experimental::filesystem;
#elif _MSC_VER > 1800   // VS2015
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
#elif __has_include(<experimental/filesystem>)
    #include <experimental/filesystem>
    using fs = std::experimental::filesystem;
#else
    #error "no filesystem support ='("
#endif

#ifdef __cpp_lib_string_view
#include <string_view>
using string_view = std::string_view
#else
#include <string>
using string_view = const std::string;
#endif

#endif // STD_COMPAT_H
