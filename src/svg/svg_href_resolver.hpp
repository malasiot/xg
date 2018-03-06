#ifndef __XG_SVG_HREF_RESOLVER_HPP__
#define __XG_SVG_HREF_RESOLVER_HPP__

#include <string>
#include <map>

#include "svg_dom.hpp"

namespace xg {
namespace svg {

class HRefResolver{
public:

    void addElement(const std::string &id, svg::Element *e) {
         if ( !id.empty() ) elements_['#' + id] = e ;
    }

    void addReference(svg::Element *e, const std::string &href) {
        if ( e ) refs_[e] = href ;
    }

    void addPaintReference(svg::Style *e, const std::string &href, bool is_fill) {
        if ( e ) paint_refs_[e] = make_pair(href, is_fill) ;
    }

    void resolve() {
        for( const auto &p: refs_ ) {
            auto it = elements_.find(p.second) ;
            if ( it != elements_.end() )
                p.first->setHRef(it->second) ;
        }

        for( const auto &p: paint_refs_ ) {
            const std::string &id = p.second.first ;
            bool is_fill = p.second.second ;
            auto it = elements_.find(id) ;
            if ( it != elements_.end() ) {
                if ( is_fill )
                    p.first->setFillPaintServer(it->second) ;
                else
                    p.first->setStrokePaintServer(it->second) ;
            }
        }
    }

    std::map<std::string, svg::Element *> elements_ ;
    std::map<svg::Element *, std::string> refs_ ;
    std::map<svg::Style *, std::pair<std::string, bool>> paint_refs_ ;
};

} // namespace svg
} //namespace xg




#endif

