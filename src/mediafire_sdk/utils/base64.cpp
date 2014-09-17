/**
 * @file base64.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "base64.hpp"

#include <string>
#include <vector>

namespace {
    static const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static const char pad_char = '=';
}  // namespace

std::string mf::utils::Base64Encode( void const * data, std::size_t size )
{
    std::string encodedString;
    encodedString.reserve(((size/3) + (size % 3 > 0)) * 4);
    int32_t temp;
    uint8_t const * cursor = static_cast<uint8_t const*>(data);
    for (size_t idx = 0; idx < size/3; idx++)
    {
        temp  = (*cursor++) << 16;  // Convert to big endian
        temp += (*cursor++) << 8;
        temp += (*cursor++);
        encodedString.append(1, base64_chars[(temp & 0x00FC0000) >> 18]);
        encodedString.append(1, base64_chars[(temp & 0x0003F000) >> 12]);
        encodedString.append(1, base64_chars[(temp & 0x00000FC0) >> 6 ]);
        encodedString.append(1, base64_chars[(temp & 0x0000003F)      ]);
    }
    switch (size % 3)
    {
        case 1:
            temp  = (*cursor++) << 16;  // Convert to big endian
            encodedString.append(1, base64_chars[(temp & 0x00FC0000) >> 18]);
            encodedString.append(1, base64_chars[(temp & 0x0003F000) >> 12]);
            encodedString.append(2, pad_char);
            break;
        case 2:
            temp  = (*cursor++) << 16;  // Convert to big endian
            temp += (*cursor++) << 8;
            encodedString.append(1, base64_chars[(temp & 0x00FC0000) >> 18]);
            encodedString.append(1, base64_chars[(temp & 0x0003F000) >> 12]);
            encodedString.append(1, base64_chars[(temp & 0x00000FC0) >> 6 ]);
            encodedString.append(1, pad_char);
            break;
    }
    return encodedString;
}

boost::optional<std::vector<uint8_t>> mf::utils::Base64Decode(
        const std::string & input)
{
    if (input.length() % 4)  // Sanity check
        return boost::optional<std::vector<uint8_t>>();
    size_t padding = 0;
    if (input.length())
    {
        if (input[input.length()-1] == pad_char)
            padding++;
        if (input[input.length()-2] == pad_char)
            padding++;
    }
    // Setup a vector to hold the result
    std::vector<uint8_t> decodedBytes;
    decodedBytes.reserve(((input.length()/4)*3) - padding);
    int32_t temp = 0;  // Holds decoded quanta
    std::basic_string<char>::const_iterator cursor = input.begin();
    while (cursor < input.end())
    {
        for (size_t quantumPosition = 0; quantumPosition < 4; quantumPosition++)
        {
            temp <<= 6;
            if       (*cursor >= 0x41 && *cursor <= 0x5A)  // This area will
                temp |= *cursor - 0x41;                    // need tweaking if
            else if  (*cursor >= 0x61 && *cursor <= 0x7A)  // you are using an
                temp |= *cursor - 0x47;                    // alternate alphabet
            else if  (*cursor >= 0x30 && *cursor <= 0x39)
                temp |= *cursor + 0x04;
            else if  (*cursor == 0x2B)
                temp |= 0x3E;  // change to 0x2D for URL alphabet
            else if  (*cursor == 0x2F)
                temp |= 0x3F;  // change to 0x5F for URL alphabet
            else if  (*cursor == pad_char)  // pad
            {
                switch ( input.end() - cursor )
                {
                    case 1:  // One pad character
                        decodedBytes.push_back((temp >> 16) & 0x000000FF);
                        decodedBytes.push_back((temp >> 8 ) & 0x000000FF);
                        return decodedBytes;
                    case 2:  // Two pad characters
                        decodedBytes.push_back((temp >> 10) & 0x000000FF);
                        return decodedBytes;
                    default:
                        {
                            // Invalid Padding
                            return boost::optional<std::vector<uint8_t>>();
                        }
                }
            }
            else
            {
                // Non-Valid Character
                return boost::optional<std::vector<uint8_t>>();
            }
            cursor++;
        }
        decodedBytes.push_back((temp >> 16) & 0x000000FF);
        decodedBytes.push_back((temp >> 8 ) & 0x000000FF);
        decodedBytes.push_back((temp      ) & 0x000000FF);
    }
    return decodedBytes;
}
