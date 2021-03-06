#pragma once
#include <vector>
#include <cstdint>
#include <string>


namespace torrentsync
{
namespace utils
{

typedef std::vector<uint8_t> Buffer;

Buffer makeBuffer( const std::string& );

std::string toString( const Buffer& );

const torrentsync::utils::Buffer parseIDFromHex( const std::string& str );

}; // utils
}; // torrentsync

bool operator==( const torrentsync::utils::Buffer&, const std::string& );
bool operator!=( const torrentsync::utils::Buffer&, const std::string& );

std::ostream& operator<<( std::ostream& stream, const torrentsync::utils::Buffer& buff );

//! Pretty printing wrapper for a buffer containing string data.
//! Will print all the content of the buffer escaping the binary data
//! until a c-string terminator ('\0') is found.
struct pretty_print
{
    const torrentsync::utils::Buffer& _buff;
    pretty_print(const torrentsync::utils::Buffer& buff) : _buff(buff) {}
};

std::ostream& operator<<( std::ostream& stream, const pretty_print& buff );
