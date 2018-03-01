#ifndef __XG_SVG_PARSER_HPP__
#define __XG_SVG_PARSER_HPP__

#include <string>
#include <xg/util/dictionary.hpp>
#include <iostream>

namespace xg {

class SVGLoadException ;
class SVGDocument ;

class SVGParser {
public:
    SVGParser(SVGDocument &doc): document_(doc) {}

    void parseString(const std::string &xml) ;
    void parseStream(std::istream &strm, size_t buffer_sz = 1024) ;
protected:

    void beginElement(const std::string &name, const Dictionary &attributes) ;
    void endElement() ;

private:

    static void start_element_handler(void *data, const char *element_name, const char **attributes) ;
    static void end_element_handler(void *data, const char *elelemnt_name);
    static void character_data_handler(void *data, const char *character_data, int length);


private:

    std::string element_, text_ ;
    Dictionary attributes_ ;

    SVGDocument &document_ ;
};


}

#endif
