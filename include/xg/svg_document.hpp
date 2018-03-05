#ifndef __XG_SVG_DOCUMENT_HPP__
#define __XG_SVG_DOCUMENT_HPP__

#include <string>
#include <istream>
#include <memory>


namespace xg {

namespace svg {
class SVGElement ;
}

// Encapsulates the SVG dom
class SVGDocument {
public:

    void readStream(std::istream &strm) ;

private:

    friend class SVGParser;
    friend class Canvas ;

    std::shared_ptr<svg::SVGElement> root_ ;
};


class SVGLoadException {
public:
    SVGLoadException(const std::string &error_msg, size_t line, size_t col) ;

    std::string what() ;
private:
    std::string msg_ ;
    size_t line_, col_ ;
};

}


#endif
