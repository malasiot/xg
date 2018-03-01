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
    if ( name == "svg" ) {
        auto node = std::make_shared<svg::DocumentNode>() ;
        node->parseAttributes(attributes) ;
        document_.root_ = node ;
    }

}

void SVGParser::endElement() {

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
