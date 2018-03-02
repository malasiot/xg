#include <xg/svg_document.hpp>

#include "svg_parser.hpp"
#include "svg_dom.hpp"

#include <expat.h>
#include <memory>

using namespace std ;

namespace xg {

struct SVGParserContext {

   XML_Parser expat_parser_ ;
   SVGParser &parser_ ;
} ;


void SVGParser::parseString(const std::string &xml) {

    std::unique_ptr<XML_ParserStruct, decltype(&XML_ParserFree)> parser( XML_ParserCreate(nullptr), &XML_ParserFree ) ;

    SVGParserContext ctx{parser.get(), *this} ;
    XML_SetUserData(parser.get(), &ctx);

    XML_SetElementHandler(parser.get(), &start_element_handler, &end_element_handler);
    XML_SetCharacterDataHandler(parser.get(), &character_data_handler);

    if ( XML_Parse(parser.get(), xml.c_str(), xml.length(), true ) != XML_STATUS_OK )  {
        throw SVGLoadException(XML_ErrorString(XML_GetErrorCode(parser.get())),
                               XML_GetCurrentLineNumber(parser.get()),
                               XML_GetCurrentColumnNumber(parser.get())) ;
    }
}

void SVGParser::parseStream(std::istream &strm, size_t buffer_size) {

    std::unique_ptr<XML_ParserStruct, decltype(&XML_ParserFree)> parser( XML_ParserCreate(nullptr), &XML_ParserFree ) ;

    std::unique_ptr<char[]> buf(new char [buffer_size]);

    SVGParserContext ctx{parser.get(), *this} ;
    XML_SetUserData(parser.get(), &ctx);

    XML_SetElementHandler(parser.get(), &start_element_handler, &end_element_handler);
    XML_SetCharacterDataHandler(parser.get(), &character_data_handler);

    bool done ;
    do {
        strm.read(buf.get(), buffer_size);
        size_t len = strm.gcount() ;
        done = len < buffer_size ;
        if ( XML_Parse(parser.get(), buf.get(), (int)len, done) != XML_STATUS_OK )
            throw SVGLoadException(XML_ErrorString(XML_GetErrorCode(parser.get())),
                                   XML_GetCurrentLineNumber(parser.get()),
                                   XML_GetCurrentColumnNumber(parser.get())) ;
    } while (!done);
}

void SVGParser::beginElement(const string &name, const Dictionary &attributes) {

    elements_.push_back(name) ;

   for( const auto &s: elements_ )
       cout << "<" << s << "> " ;
   cout << endl ;

    if ( name == "svg" ) {
        bool is_root = nodes_.empty() ;
        auto node = createNode<svg::SVGElement>(attributes) ;
        if ( is_root ) document_.root_ = node ;
    }
    else if ( name == "g" )
        createNode<svg::GroupElement>(attributes) ;
    else if ( name == "rect" )
        createNode<svg::RectElement>(attributes) ;
    else if ( name == "path" )
        createNode<svg::PathElement>(attributes) ;
    else if ( name == "line" )
        createNode<svg::LineElement>(attributes) ;
    else if ( name == "ellipse" )
        createNode<svg::EllipseElement>(attributes) ;
    else if ( name == "polyline" )
        createNode<svg::PolylineElement>(attributes) ;
    else if ( name == "polygon" )
        createNode<svg::PolygonElelemnt>(attributes) ;
    else if ( name == "circle" )
        createNode<svg::CircleElement>(attributes) ;
    else if ( name == "text" )
        createNode<svg::TextElement>(attributes) ;
    else if ( name == "defs" )
        createNode<svg::DefsElement>(attributes) ;
    else if ( name == "symbol" )
        createNode<svg::SymbolElement>(attributes) ;
    else if ( name == "linearGradient" )
        createNode<svg::LinearGradientElement>(attributes) ;
    else if ( name == "radialGradient" )
        createNode<svg::RadialGradientElement>(attributes) ;
    else if ( name == "use" )
        createNode<svg::UseElelement>(attributes) ;
    else if ( name == "image" )
        createNode<svg::ImageElement>(attributes) ;
    else if ( name == "pattern" )
        createNode<svg::PatternElement>(attributes) ;
    else if ( name == "clipPath" )
        createNode<svg::ClipPathElement>(attributes) ;
    else if ( name == "style" )
        createNode<svg::StyleElement>(attributes) ;
    else if ( name == "stop" )
        createNode<svg::Stop>(attributes) ;
    else
        createNode<svg::UnsupportedElement>(attributes) ;
}

void SVGParser::endElement() {
    nodes_.pop_back() ;
    elements_.pop_back() ;
}

void SVGParser::start_element_handler(void *data, const char *element_name, const char **attributes)
{
    SVGParserContext *ctx = (SVGParserContext *)data ;

    Dictionary attr ;
    const char **p = attributes ;
    while ( *p ) {
        string name(*p++) ;
        string value(*p++) ;
        attr.add(name, value) ;
    }

    try {
        ctx->parser_.beginElement(element_name, attr) ;
    } catch ( svg::SVGDOMException &e ) {
        stringstream strm ;
        strm << "error while parsing SVG element <" << element_name << ">: " << e.what() ;

        throw SVGLoadException(strm.str(),
                               XML_GetCurrentLineNumber(ctx->expat_parser_),
                               -1) ;
    }
}

void SVGParser::end_element_handler(void *data, const char *) {
    SVGParserContext *ctx = (SVGParserContext *)data ;

    ctx->parser_.endElement() ;
}

void SVGParser::character_data_handler(void *data, const char *character_data, int length) {
    SVGParserContext *ctx = (SVGParserContext *)data ;
    ctx->parser_.text_.assign(character_data, length) ;
}
}
