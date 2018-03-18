#ifndef __XG_SVG_DOM_ATTRIBUTES_HPP__
#define __XG_SVG_DOM_ATTRIBUTES_HPP__

#include "svg_length.hpp"
#include "svg_dom_exceptions.hpp"

#include <xg/path.hpp>

namespace xg {
namespace svg {

class ViewBox {
public:

    ViewBox() = default ;
    ViewBox(float x, float y, float w, float h): x_(x), y_(y), width_(w), height_(h) {}

    bool parse(const std::string &s) ;

    float x_ = 0, y_ = 0, width_ = 0 , height_ = 0 ;
} ;

class PathData {
public:

    PathData() = default ;

    bool parse(const std::string &str) ;

    const Path &path() const { return path_ ; }

protected:

    Path path_ ;
} ;


class PointList {
public:

    bool parse(const std::string &str) ;

    const std::vector<Point2d> &points() const { return points_ ; }

protected:

    std::vector<Point2d> points_ ;
} ;

class PreserveAspectRatio
{
public:

    enum ViewBoxAlign { NoViewBoxAlign, XMinYMin, XMidYMin, XMaxYMin, XMinYMid, XMidYMid, XMaxYMid,
                        XMinYMax, XMidYMax, XMaxYMax } ;
    enum ViewBoxPolicy { MeetViewBoxPolicy, SliceViewBoxPolicy } ;

    PreserveAspectRatio(): view_box_align_(XMidYMid), view_box_policy_(MeetViewBoxPolicy), defer_aspect_ratio_(false) {}

    bool parse(const std::string &str) ;
    void constrainViewBox(double width, double height, ViewBox &orig) const;
    Matrix2d getViewBoxTransform(double sw, double sh, double vwidth, double vheight, double vx, double vy) const;

    bool defer_aspect_ratio_ ;
    ViewBoxAlign view_box_align_ ;
    ViewBoxPolicy view_box_policy_ ;
} ;

class URIReference {
public:
    URIReference() = default ;
    URIReference(const std::string &val): uri_(val) {}

    bool parse(const std::string &v) {
        uri_ = v ;
        return true ;
    }

    const std::string &uri() const { return uri_ ; }

    std::string uri_ ;
};

template<typename S>
class OptionalAttribute {
public:

    typedef S type ;

    // set default value
    OptionalAttribute() = default ;

    OptionalAttribute(const OptionalAttribute<S> &) = delete ;
    OptionalAttribute &operator = ( const OptionalAttribute<S> &) = delete ;

    const S &value() const { return value_ ; }
    void assign(const S &val) { value_ = val ; has_value_ = true ; }
    bool hasValue() const { return has_value_ ;}

    S value_ ;
    bool has_value_ = false ;
} ;

#define SVG_ELEMENT_ATTRIBUTE(avar, aget, atype, adef)\
protected:\
    OptionalAttribute<atype> avar ;\
public:\
\
    const atype &aget() const { \
        static const atype avar_default_{adef} ;\
        return ( avar.hasValue() ) ? avar.value() : avar_default_ ; }\
    bool aget##IsSet() const { return avar.hasValue() ; }



template<typename T>
inline void parseOptionalAttribute(const char *name, const Dictionary &attr, OptionalAttribute<T> &t) {

    std::string value = attr.get(name) ;
    try {
        if ( !value.empty() ) {
            T v ;
            v.parse(value) ;
            t.assign(v) ;
        }
    }
    catch ( SVGDOMAttributeValueException &e ) {
        std::string reason = e.msg_ ;
        std::stringstream strm ;
        strm << "invalid value \"" << value << "\" of attribute \"" << name << "\"" ;
        if ( !reason.empty() ) strm << ": " << reason ;
        throw SVGDOMException(strm.str()) ;
    }
}


} // namespace svg
} //namespace xg




#endif

