#include <xg/svg_document.hpp>
#include <xg/canvas.hpp>

#include "svg_parser.hpp"
#include "svg_render_context.hpp"
#include "svg_href_resolver.hpp"

#include <sstream>

using namespace std ;

namespace xg {

SVGLoadException::SVGLoadException(const std::__cxx11::string &error_msg, size_t line, size_t col):
    msg_(error_msg), line_(line), col_(col) {}

std::string SVGLoadException::what() {
    ostringstream strm ;
    strm << "XML parse error: " << msg_ << " line: " << line_ << ", column: " << col_ ;
    return strm.str() ;
}

void SVGDocument::readStream(std::istream &strm) {
    svg::HRefResolver resolver ;
    SVGParser parser(*this, resolver) ;
    parser.parseStream(strm) ;
    resolver.resolve() ;
}

void Canvas::drawSVG(const SVGDocument &doc)
{
    svg::RenderingContext ctx(*this) ;
    ctx.populateRefs(doc.root_);
    auto root = std::dynamic_pointer_cast<svg::SVGElement>(doc.root_) ;
    ctx.render(*root) ;
}

}
