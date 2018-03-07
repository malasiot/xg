#ifndef __XG_SVG_DOM_HPP__
#define __XG_SVG_DOM_HPP__

#include <string>
#include <xg/util/dictionary.hpp>
#include <xg/xform.hpp>
#include <xg/path.hpp>

#include "svg_length.hpp"
#include "svg_dom_exceptions.hpp"
#include "svg_style.hpp"

namespace xg {
namespace svg {

class ViewBox {
public:

    ViewBox() = default ;
    ViewBox(float x, float y, float w, float h): x_(x), y_(y), width_(w), height_(h) {}

    void parse(const std::string &s) ;

    float x_ = 0, y_ = 0, width_ = 0 , height_ = 0 ;
} ;

class PathData
{
public:

    enum Command { MoveToCmd, ClosePathCmd, LineToCmd, CurveToCmd, SmoothCurveToCmd,
                   QuadCurveToCmd, SmoothQuadCurveToCmd, EllipticArcToCmd } ;

    PathData() {}

    void parse(const std::string &str) ;

    struct Element {
        Element(Command cc, double arg1 = 0, double arg2 = 0, double arg3 = 0, double arg4 = 0,
                double arg5 = 0, double arg6 = 0) {
            cmd = cc ;
            args_[0] = arg1 ; args_[1] = arg2 ; args_[2] = arg3 ; args_[3] = arg4 ; args_[4] = arg5 ;
            args_[5] = arg6 ;
        }
        double args_[6] ;
        Command cmd ;
    } ;

    std::vector<Element> elements_ ;

    Path path_ ;
} ;


class PointList {
public:

    void parse(const std::string &str) ;

    std::vector<Point2d> points_ ;
} ;

class Transform {
public:

    Transform() = default ;

    void parse(const std::string &str) ;

    Matrix2d m_ ;
} ;


class PreserveAspectRatio
{
public:

    enum ViewBoxAlign { NoViewBoxAlign, XMinYMin, XMidYMin, XMaxYMin, XMinYMid, XMidYMid, XMaxYMid,
                        XMinYMax, XMidYMax, XMaxYMax } ;
    enum ViewBoxPolicy { MeetViewBoxPolicy, SliceViewBoxPolicy } ;

    PreserveAspectRatio(): view_box_align_(XMidYMid), view_box_policy_(MeetViewBoxPolicy), defer_aspect_ratio_(false) {}

    void parse(const std::string &str) ;
    void constrainViewBox(double width, double height, ViewBox &orig) ;
    Matrix2d getViewBoxTransform(double sw, double sh, double vwidth, double vheight, double vx, double vy);

    bool defer_aspect_ratio_ ;
    ViewBoxAlign view_box_align_ ;
    ViewBoxPolicy view_box_policy_ ;
} ;

class Stylable {
public:

    virtual ~Stylable() {}
    Style style_ ;
} ;

class Transformable
{
public:

    virtual ~Transformable() {}

    Matrix2d trans_ ;
} ;

class FitToViewBox {
public:

    ViewBox view_box_ ;
    PreserveAspectRatio preserve_aspect_ratio_ ;
} ;


class URI {
public:

    void parse(const std::string &val) {
        href_ = val ; // not parsed
    }

    std::string href_ ;
} ;

class CircleElement ;
class LineElement ;
class PolylineElement ;
class PolygonElement ;
class PathElement ;
class RectElement ;
class EllipseElement ;
class DefsElement ;
class GroupElement ;
class SymbolElement ;
class UseElement ;
class ClipPathElement ;
class ImageElement ;
class TextElement ;
class LinearGradientElement ;
class RadialGradientElement ;
class PatternElement ;
class TextSpanElement ;
class SVGElement ;
class StyleElement ;
class Element ;

// base class of all SVG elements

using ElementPtr = std::shared_ptr<Element> ;

class Element
{
public:

    Element() = default ;
    virtual ~Element() = default ;

    void parseElementAttributes(const Dictionary &a) ;
    void parseStyleAttributes(const Dictionary &p, Style &style) ;
    void parseTransformAttribute(const Dictionary &p, Matrix2d &t) ;
    void parseViewBoxAttributes(const Dictionary &p, ViewBox &view_box, PreserveAspectRatio &preserve_aspect_ratio );

    virtual bool canHaveChild(const ElementPtr &p) const { return false ; }

    bool addChild(const ElementPtr &p) {
        if ( !canHaveChild(p) ) return false ;
        children_.emplace_back(p) ;
        p->parent_ = this ;
        return true ;
    }

    void setRoot(SVGElement *e) { root_ = e ; }

    // return most close SVGElement
    SVGElement &root() { return *root_;}

    std::string id() const ;

    const std::vector<ElementPtr> &children() const { return children_ ; }

protected:

    std::string id_ ;
    Element *parent_ = nullptr ;
    SVGElement *root_ ;

    std::vector<ElementPtr> children_ ;
} ;



// Helper class to define container elements that can contain children belonginf only within a set of types

template<typename...>
struct Container: public Element {
    static bool accepts(const std::shared_ptr<svg::Element> &ele) {
        return false ;
    }
};

template<typename T, typename... Ts>
struct Container<T, Ts...>: public Element {
    static bool accepts(const std::shared_ptr<svg::Element> &ele) {
        if ( std::dynamic_pointer_cast<T>(ele) ) return true ;
        else return Container<Ts...>::accepts(ele) ;
    }

    bool canHaveChild(const std::shared_ptr<Element> &p) const override { return accepts(p) ; }
};

using GroupContainer =  Container<CircleElement, LineElement, PolylineElement, PolygonElement, RectElement, PathElement, EllipseElement,
DefsElement, SVGElement, GroupElement, SymbolElement, UseElement, LinearGradientElement, RadialGradientElement,
ClipPathElement, ImageElement, PatternElement, StyleElement, TextElement> ;

using ShapeContainer = Container<CircleElement, LineElement, PolylineElement, PolygonElement, RectElement, PathElement, EllipseElement, UseElement, TextElement> ;

template<typename S>
class Attribute {
public:

    typedef S type ;

    // set default value
    Attribute(const S &def): default_(def) {}

    Attribute(const Attribute<S> &) = delete ;
    Attribute &operator = ( const Attribute<S> &) = delete ;

    S value() const { return ( has_value_ ) ? value_ : default_ ; }
    void assign(const S &val) { value_ = val ; has_value_ = true ; }
    bool hasValue() const { return has_value_ ;}
    void setDefault(const S &val) { default_ = val; }

    S value_ ;
    S default_ ;
    bool has_value_ = false ;
} ;



class StyleElement: public Element {
public:

    StyleElement() = default ;

    void parseAttributes(const Dictionary &) ;

    std::string type_, media_, title_ ;
} ;


class StopElement: public Element, public Stylable {
public:

    StopElement() = default ;

    void parseAttributes(const Dictionary &p) ;

    Attribute<float> offset_{0.0} ;
} ;



enum class GradientSpreadMethod { Unknown, Pad, Reflect, Repeat } ;
enum class GradientUnits { UserSpaceOnUse, ObjectBoundingBox } ;

class GradientElement: public Container<StopElement>, public Stylable {
public:

    GradientElement(): spread_method_(GradientSpreadMethod::Pad), gradient_units_(GradientUnits::ObjectBoundingBox), trans_(Matrix2d()) { }

    void parseAttributes(const Dictionary &pNode) ;

    GradientSpreadMethod spreadMethod() ;
    GradientUnits gradientUnits() ;
    Matrix2d gradientTransform() ;

    void collectStops(std::vector<StopElement *> &stops) ;
protected:


    Attribute<GradientSpreadMethod> spread_method_ ;
    Attribute<GradientUnits> gradient_units_ ;
    Attribute<Matrix2d> trans_ ;
    std::string href_ ;

} ;

class LinearGradientElement: public GradientElement {
public:

    LinearGradientElement() = default ;

    void parseAttributes(const Dictionary &a) ;

    Length x1() ;
    Length y1() ;
    Length x2() ;
    Length y2() ;

    Attribute<Length> x1_{0.0_perc}, y1_{0.0_perc}, x2_{1.0_perc}, y2_{0.0_perc} ;
} ;

class RadialGradientElement: public GradientElement {
public:

    RadialGradientElement() = default ;

    void parseAttributes(const Dictionary &a) ;

    Length cx() ;
    Length cy() ;
    Length fx() ;
    Length fy() ;
    Length r() ;

    Attribute<Length> cx_{0.5_perc}, cy_{0.5_perc}, r_{0.5_perc}, fx_{0}, fy_{0} ;
} ;

enum class PatternUnits { UserSpaceOnUse, ObjectBoundingBox } ;

class PatternElement:
        public GroupContainer,
        public Stylable

{
public:

    PatternElement() = default ;

    void parseAttributes(const Dictionary &a) ;

    PatternUnits patternUnits() ;
    PatternUnits patternContentUnits() ;
    Matrix2d patternTransform() ;
    Length x() ;
    Length y() ;
    Length width() ;
    Length height() ;
    ViewBox viewBox() ;
    PreserveAspectRatio preserveAspectRatio() ;

protected:

    Attribute<PatternUnits> pattern_units_{PatternUnits::UserSpaceOnUse} ;
    Attribute<PatternUnits> pattern_content_units_{PatternUnits::ObjectBoundingBox} ;
    Attribute<Matrix2d> trans_{Matrix2d()} ;
    Attribute<Length> x_{0}, y_{0}, width_{0}, height_{0} ;
    Attribute<ViewBox> view_box_{ViewBox()} ;
    Attribute<PreserveAspectRatio> preserve_aspect_ratio_{PreserveAspectRatio()} ;

    std::string href_ ;
} ;


class ImageElement: public Element, public Transformable, public Stylable
{
public:

    ImageElement() = default ;

    void parseAttributes(const Dictionary &attrs) ;

    URI uri_ ;
    Length x_{0}, y_{0}, width_{0}, height_{0} ;
    PreserveAspectRatio preserve_aspect_ratio_ ;
} ;

enum ClipPathUnits { UserSpaceOnUse, ObjectBoundingBox } ;

class ClipPathElement:
        public ShapeContainer,
        public Transformable, public Stylable
{
public:

    ClipPathElement(): clip_path_units_(UserSpaceOnUse) {}

    void parseAttributes(const Dictionary &a) ;

    ClipPathUnits clip_path_units_ ;

} ;

class UseElement: public GroupContainer, public Transformable, public Stylable
{
public:
    UseElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    Length x_{0}, y_{0}, width_, height_ ;
    URI uri_ ;
} ;


class GroupElement:
        public GroupContainer,
        public Transformable, public Stylable
{
public:

    GroupElement() = default ;

    void parseAttributes(const Dictionary &a)  ;
} ;

class DefsElement: public GroupContainer, public Transformable, public Stylable {
public:

    DefsElement() = default ;

    void parseAttributes(const Dictionary &a)  ;
} ;

class PathElement: public Element, public Transformable, public Stylable {
public:

    PathElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    PathData path_ ;
} ;

class RectElement: public Element, public Stylable, public Transformable {
public:

    RectElement() {}

    void parseAttributes(const Dictionary &a)  ;

    Length x_, y_, width_, height_, rx_, ry_ ;
} ;

class CircleElement: public Element,  public Stylable, public Transformable {
public:

    CircleElement() {}

    void parseAttributes(const Dictionary &a)  ;

    Length cx_, cy_, r_ ;
} ;

class LineElement: public Element,  public Stylable, public Transformable {
public:

    LineElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    Length x1_, y1_, x2_, y2_ ;
} ;

class EllipseElement: public Element, public Stylable, public Transformable {
public:

    EllipseElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    Length cx_, cy_, rx_, ry_ ;
} ;

class PolylineElement: public Element, public Stylable, public Transformable {
public:

    PolylineElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    PointList points_ ;
} ;

class PolygonElement: public Element , public Stylable, public Transformable {
public:

    PolygonElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    PointList points_ ;
} ;

class TextPosElement
{
public:

    TextPosElement() = default ;

    void parseAttributes(const Dictionary &pNode) ;

    Length x_{0}, y_{0}, dx_{0}, dy_{0} ;
    bool preserve_white_{false} ;
} ;


class TextElement: public Container<TextElement, TextSpanElement>, public TextPosElement, public Transformable {
public:

    TextElement() {}

    void parseAttributes(const Dictionary &a) ;
} ;


class TextSpanElement: public TextPosElement, public Container<TextSpanElement> {
public:

    TextSpanElement() = default ;

    void parseAttributes(const Dictionary &a) ;

    std::string text_ ;
} ;

template<typename T>
inline void parseAttribute(const char *name, const Dictionary &attr, T &t) {
    std::string value = attr.get(name) ;
    try {
        if ( !value.empty() )
            t.parse(value) ;
    }
    catch ( SVGDOMAttributeValueException &e ) {
        std::string reason = e.what() ;
        std::stringstream strm ;
        strm << "invalid value \"" << value << "\" of attribute \"" << name << "\"" ;
        if ( !reason.empty() ) strm << ": " << reason ;
        throw SVGDOMException(strm.str()) ;
    }
}

template<typename T>
inline void parseOptionalAttribute(const char *name, const Dictionary &attr, Attribute<T> &t) {

    std::string value = attr.get(name) ;
    try {
        if ( !value.empty() ) {
            T v ;
            v.parse(value) ;
            t.assign(v) ;
        }
    }
    catch ( SVGDOMAttributeValueException &e ) {
        std::string reason = e.what() ;
        std::stringstream strm ;
        strm << "invalid value \"" << value << "\" of attribute \"" << name << "\"" ;
        if ( !reason.empty() ) strm << ": " << reason ;
        throw SVGDOMException(strm.str()) ;
    }
}



class SVGElement: public GroupContainer, public Stylable, public FitToViewBox {

public:

    SVGElement(): x_{0.0}, y_{0.0},
        width_{1.0_perc}, height_{1.0_perc} {
    }

    void parseAttributes(const Dictionary &attrs) ;

    void parseCSS(const std::string &str) ;

    void registerNamedElement(const std::string &id, Element *e) {
        elements_.insert({"#" + id, e}) ;
    }

    Element *resolve(const std::string &uri) const ;


    Length x_, y_, width_, height_ ;


protected:

    std::map<std::string, svg::Element *> elements_ ;
    std::map<svg::Element *, std::string> refs_ ;

} ;

class SymbolElement: public SVGElement {

public:

    SymbolElement() = default ;
} ;

class UnsupportedElement: public Element {
public:
     bool canHaveChild(const std::shared_ptr<Element> &p) const override { return true ; }
     void parseAttributes(const Dictionary &) {}
};

} // namespace svg
} //namespace xg




#endif

