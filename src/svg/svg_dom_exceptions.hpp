#ifndef __XG_SVG_DOM_EXCEPTIONS_HPP__
#define __XG_SVG_DOM_EXCEPTIONS_HPP__

#include <string>
#include <stdexcept>

namespace xg {
namespace svg {

class SVGDOMAttributeValueException {
public:
    SVGDOMAttributeValueException(const std::string &name, const std::string &val,
                                  const std::string &msg = std::string()): name_(name), val_(val), msg_(msg) {}
    std::string name_, val_, msg_ ;
} ;

class SVGDOMException: public std::runtime_error {
public:
    SVGDOMException(const std::string &msg): std::runtime_error(msg) {}
} ;


} // namespace svg
} //namespace xg




#endif

