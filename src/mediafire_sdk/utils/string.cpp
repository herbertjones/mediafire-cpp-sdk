/**
 * @file string.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "./string.hpp"

#if defined(__MINGW32__)
#       define UNICODE
#       include <windows.h>
#       include <vector>
#else
#       include <codecvt>
#       include <locale>
#       include <string>
#endif

#if defined(__MINGW32__)
uint64_t mf::utils::str_to_uint64(
        const std::string & str,
        int base
    )
{
    char const * const begin = str.c_str();
    char const * const end = str.c_str() + str.size();
    char const * const endptr = end;

    uint64_t retval = _strtoui64(
        begin,
        const_cast<char**>(&endptr),
        base );

    if (endptr == begin)
    {
        // Invalid conversion
        throw std::invalid_argument("Invalid argument");
    }
    else if (retval == _UI64_MAX)
    {
        throw std::out_of_range("Overflow");
    }

    return retval;
}

// MinGW doesn't have #include <codecvt>
std::string mf::utils::wide_to_bytes(const std::wstring & wide_string)
{
    // Get required size of buffer (in characters!)
    int chars_required_with_null_terminator = WideCharToMultiByte(
        CP_UTF8, /* code page */
        0, /* flags */
        wide_string.data(), /* wide char source */
        wide_string.size(), /* wide char size */
        NULL, /* write pointer */
        0, /* write pointer size */
        NULL, /* pointer to char for unrepresentable characters */
        NULL  /* pointer to bool if unrepresentable char used */);

    if ( chars_required_with_null_terminator == 0 )
    {
        return std::string();
    }

    std::vector<char> buffer(chars_required_with_null_terminator, '\0');

    // Convert string
    int bytes_written = WideCharToMultiByte(
        CP_UTF8,
        0,
        wide_string.data(),
        wide_string.size(),
        buffer.data(),
        chars_required_with_null_terminator,
        NULL,
        NULL);
    // bytes_written contains the number of characters written, unlike the first
    // count, which should be +1 as it includes the null terminator.

    assert( (bytes_written-1) == chars_required_with_null_terminator );

    if ( bytes_written == 0 )
    {
        return std::string();
    }

    // Return the string minus the terminator character as std::string handles
    // that for us
    return std::string(buffer.data(), bytes_written);
}

// This function receives a Utf8 std::string and returns a std::wstring
std::wstring mf::utils::bytes_to_wide(const std::string & byte_string)
{
    // Get required size of buffer
    int wchars_required_with_null_terminator = MultiByteToWideChar(
        CP_UTF8,
        0,
        byte_string.data(),
        byte_string.size(),
        NULL,
        0);

    if ( wchars_required_with_null_terminator == 0 )
    {
        return std::wstring();
    }

    std::vector<wchar_t> buffer(wchars_required_with_null_terminator, L'\0');

    // Convert string
    int bytes_written = MultiByteToWideChar(
        CP_UTF8,
        0,
        byte_string.data(),
        byte_string.size(),
        buffer.data(),
        wchars_required_with_null_terminator);
    // bytes_written contains the number of characters written, unlike the first
    // count, which should be +1 as it includes the null terminator.

    assert( (bytes_written - 1) == wchars_required_with_null_terminator );

    if ( bytes_written == 0 )
    {
        return std::wstring();
    }

    // Return the string minus the terminator character as std::wstring handles
    // that for us
    return std::wstring(buffer.data(), bytes_written);
}
#else
uint64_t mf::utils::str_to_uint64(
        const std::string & str,
        int base
    )
{
    return std::stoul(str, 0, base);
}
std::string mf::utils::wide_to_bytes(const std::wstring & str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(str);
}

std::wstring mf::utils::bytes_to_wide(const std::string & str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}
#endif

std::string mf::utils::path_to_utf8(
        const boost::filesystem::path & path
    )
{
#ifdef _WIN32
        return mf::utils::wide_to_bytes(path.wstring());
#else
        return path.string();
#endif
}
