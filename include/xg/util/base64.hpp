#ifndef __XG_BASE64_HPP__
#define __XG_BASE64_HPP__

#include <string>

namespace xg {
std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);
}

#endif
