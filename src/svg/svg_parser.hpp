#ifndef __XG_SVG_PARSER_HPP__
#define __XG_SVG_PARSER_HPP__

#include <string>
#include <xg/util/dictionary.hpp>
#include <iostream>

#include "svg_dom.hpp"

namespace xg {

class SVGLoadException ;
class SVGDocument ;

class SVGParser {
public:
    SVGParser(SVGDocument &doc): document_(doc) {}

    void parseString(const std::string &xml) ;
    void parseStream(std::istream &strm, size_t buffer_sz = 1024) ;
protected:

    template <typename T>
    std::shared_ptr<T> createNode(const Dictionary &a) {
        auto node = std::make_shared<T>() ;
        node->parseAttributes(a) ;
        auto ele = std::dynamic_pointer_cast<svg::Element>(node) ;

        if ( !nodes_.empty() ) {
            auto stack_node = nodes_.back() ;
            stack_node->addChild(ele) ;
        }
        nodes_.push_back(ele) ;
        return node ;
    }
    void beginElement(const std::string &name, const Dictionary &attributes) ;
    void endElement() ;

private:

    static void start_element_handler(void *data, const char *element_name, const char **attributes) ;
    static void end_element_handler(void *data, const char *elelemnt_name);
    static void character_data_handler(void *data, const char *character_data, int length);

private:

    std::string text_ ;

    SVGDocument &document_ ;
    std::deque<std::shared_ptr<svg::Element>> nodes_ ;
    std::deque<std::string> elements_ ;
};


}

#endif
