#ifndef __XG_SVG_DOM_EXCEPTIONS_HPP__
#define __XG_SVG_DOM_EXCEPTIONS_HPP__

#include <string>
#include <stdexcept>

namespace xg {
namespace svg {

class SVGDOMAttributeValueException: public std::runtime_error {
public:
    SVGDOMAttributeValueException(const std::string &msg = std::string()): std::runtime_error(msg) {}
} ;

class SVGDOMException: public std::runtime_error {
public:
    SVGDOMException(const std::string &msg): std::runtime_error(msg) {}
} ;


} // namespace svg
} //namespace xg




#endif

