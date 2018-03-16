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

    if ( name == "svg" ) {
        bool is_root = nodes_.empty() ;
        auto node = createNode<svg::SVGElement>(attributes, is_root) ;
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
        createNode<svg::PolygonElement>(attributes) ;
    else if ( name == "circle" )
        createNode<svg::CircleElement>(attributes) ;
    else if ( name == "text" )
        createNode<svg::TextElement>(attributes) ;
    else if ( name == "tspan" )
        createNode<svg::TSpanElement>(attributes) ;
    else if ( name == "tref" )
        createNode<svg::TRefElement>(attributes) ;
    else if ( name == "defs" )
        createNode<svg::DefsElement>(attributes) ;
    else if ( name == "symbol" )
        createNode<svg::SymbolElement>(attributes) ;
    else if ( name == "linearGradient" )
        createNode<svg::LinearGradientElement>(attributes) ;
    else if ( name == "radialGradient" )
        createNode<svg::RadialGradientElement>(attributes) ;
    else if ( name == "use" )
        createNode<svg::UseElement>(attributes) ;
    else if ( name == "image" )
        createNode<svg::ImageElement>(attributes) ;
    else if ( name == "pattern" )
        createNode<svg::PatternElement>(attributes) ;
    else if ( name == "clipPath" )
        createNode<svg::ClipPathElement>(attributes) ;
    else if ( name == "style" )
        createNode<svg::StyleElement>(attributes) ;
    else if ( name == "stop" )
        createNode<svg::StopElement>(attributes) ;
    else
        createNode<svg::UnsupportedElement>(attributes) ;
}

void SVGParser::endElement() {
    nodes_.pop_back() ;
    elements_.pop_back() ;
}

void SVGParser::characters(const string &text) {
    if ( auto p = std::dynamic_pointer_cast<svg::TextElement>(nodes_.back()) ) {
        p->addChild(make_shared<svg::TSpanElement>(text)) ;
    }
    else if ( auto p = std::dynamic_pointer_cast<svg::TSpanElement>(nodes_.back()) ) {
        p->text_ = text ;
    }
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

string SVGParser::processWhiteSpace(const char *character_data, int length) {

    string text ;

    if ( nodes_.back()->space() == svg::WhiteSpaceProcessing::Default ) {

        const char *p = character_data ;
        const char *q = p + length ;

        bool is_space = false, is_start = true ;
        while ( p != q ) {
            if ( *p == '\r' ) ;
            else if ( *p == '\n' )  ;
            else if ( *p == '\t' || *p == ' ') {
                if ( !is_space ) is_space = true ;
            }
            else {
                if ( is_space ) {
                    if ( !is_start ) text.push_back(' ') ;
                    is_space = false ;
                }
                text.push_back(*p) ;
                is_start = false ;
            }
            ++p ;
        }
    } else {
        const char *p = character_data ;
        const char *q = p + length ;

        while ( p != q ) {
            if ( isspace(*p) ) text.push_back(' ') ;
            ++p ;
        }
    }

    return text ;
}

void SVGParser::character_data_handler(void *data, const char *character_data, int length) {
    SVGParserContext *ctx = (SVGParserContext *)data ;

    string text = ctx->parser_.processWhiteSpace(character_data, length) ;

    if ( !text.empty() ) ctx->parser_.characters(text) ;
}
}
