#ifndef __XG_SVG_DOCUMENT_HPP__
#define __XG_SVG_DOCUMENT_HPP__

#include <string>
#include <istream>
#include <memory>
#include <map>

#include <xg/image.hpp>

namespace xg {

namespace svg {
class SVGElement ;
class Element ;
}

// Encapsulates the SVG dom
class SVGDocument {
public:

    void readStream(std::istream &strm) ;

    void registerNamedElement(const std::string &id, svg::Element *e);

    svg::Element *resolve(const std::string &uri) const ;
    Image loadImageResource(const std::string &uri, svg::Element *container) ;


private:

    friend class SVGParser;
    friend class Canvas ;

    std::shared_ptr<svg::SVGElement> root_ ;

protected:

    std::map<std::string, svg::Element *> elements_ ;
    std::map<svg::Element *, Image> cached_images_ ;
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
