#include <xg/svg_document.hpp>
#include "svg_parser.hpp"

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
    SVGParser parser(*this) ;
    parser.parseStream(strm) ;
}


}
