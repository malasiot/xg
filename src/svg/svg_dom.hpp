#ifndef __XG_SVG_DOM_HPP__
#define __XG_SVG_DOM_HPP__

#include <string>
#include <xg/util/dictionary.hpp>
#include <xg/xform.hpp>

#include "svg_length.hpp"
#include "svg_dom_exceptions.hpp"
#include "svg_style.hpp"

namespace xg {
namespace svg {

class ViewBox {
  public:

    ViewBox() = default ;

    void parse(const std::string &str) ;

    float x_ = 0, y_ = 0, width_ = 0 , height_ = 0 ;
} ;

class PathData
{
  public:

    enum Command { MoveToCmd, ClosePathCmd, LineToCmd, CurveToCmd, SmoothCurveToCmd,
        QuadCurveToCmd, SmoothQuadCurveToCmd, EllipticArcToCmd } ;

    PathData() {}

    bool parse(const std::string &str) ;

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
} ;






#define DASH_ARRAY_LENGTH 10

struct Color {

    Color(double r, double g, double b, double a = 1.0) ;

    // initialize from a CSS2 name or rgb specification
    Color(const char *name, double alpha = 1.0) ;

    double r_, g_, b_, a_ ;
};


class PointList {
  public:

    bool parse(const std::string &str) ;

    std::vector<float> points_ ;

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
    Transform getViewBoxTransform(double sw, double sh, double vwidth, double vheight, double vx, double vy);

    bool defer_aspect_ratio_ ;
    ViewBoxAlign view_box_align_ ;
    ViewBoxPolicy view_box_policy_ ;
} ;

class Stylable {

public:

    virtual ~Stylable() {}

    void parseAttributes(const Dictionary &p) ;

    Style style_ ;
} ;

class Transformable
{
    public:

    virtual ~Transformable() {}

    void parseAttributes(const Dictionary &pNode) ;

    Transform trans_ ;
} ;

class FitToViewBox
{
  public:

    void parseAttributes(const Dictionary &attrs) ;

    ViewBox view_box_ ;
    PreserveAspectRatio preserve_aspect_ratio_ ;

} ;

class RenderingContext ;

class Element
{
  public:

    enum Type { DocumentElement, GroupElement, AnchorElement, PathElement, SwitchElement, DefsElement, UseElement, LineElement,
        RectElement, EllipseElement, CircleElement, PolygonElement, PolyLineElement, SymbolElement,
        MaskElement, ClipPathElement, ImageElement, MarkerElement, StopElement, PatternElement,
        LinearGradientElement, ConicalGradientElement, RadialGradientElement, FilterElement, MyltiImageElement,
        SubImageRefElement, SubImageElement, TextElement, TSpanElement, TRefElement, StyleElement
    } ;

  public:

    Element() { }
    virtual ~Element() { }

    virtual bool isContainer() const { return false ; }

    void parseAttributes(const Dictionary &pNode) ;

    virtual Type getType() const = 0 ;

    std::string id_ ;
    Element *parent_ = nullptr ;
} ;

typedef std::shared_ptr<Element> ElementPtr ;

class StyleDefinition: public Element
{
  public:

    StyleDefinition() {}

    Type getType() const { return Element::StyleElement ; }
    virtual bool fromXml(const Dictionary &pNode) ;

    std::string type_, media_ ;
} ;

class StylableElement: public Element, public Stylable {
} ;

class Stop: public Element, public Stylable
{
  public:

    Stop(): offset_(0.0) {
    }

    void parseAttributes(const Dictionary &p) ;

    Length offset_ ;
} ;

class Container {

public:

    virtual ~Container() {}

    std::vector<ElementPtr> children_ ;
};

enum class GradientSpreadMethod { Unknown, Pad, Reflect, Repeat } ;
enum class GradientUnits { UserSpaceOnUse, ObjectBoundingBox } ;

class GradientElement: public Element, public Stylable, public Container
{
  public:

    GradientElement(): spread_method_(GradientSpreadMethod::Pad), gradient_units_(GradientUnits::ObjectBoundingBox) { }

    void parseAttributes(const Dictionary &pNode) ;
    void parseStops(const Dictionary &pNode) ;

    enum Fallback { SpreadMethodDefined = 0x01, GradientUnitsDefined = 0x02, TransformDefined = 0x04 } ;
    void resolveHRef(RenderingContext *ctx) ;

    GradientSpreadMethod spread_method_ ;
    GradientUnits gradient_units_ ;
    Transform trans_ ;
    std::string href_ ;
} ;

class LinearGradient: public GradientElement
{
  public:

    LinearGradient() {
        x1_ = y1_ = y2_ = x2_ = Length(0.5_perc) ;
    }

    Type getType() const { return LinearGradientElement ; }

    void parseAttributes(const Dictionary &pNode) ;


    Length x1_, y1_, x2_, y2_ ;
} ;

class RadialGradient: public GradientElement
{
  public:

    RadialGradient() {
        cx_ = cy_ = r_ = fx_ = fy_  = Length(0.5_perc) ;
    }

    void parseAttributes(const Dictionary &pNode) ;

    Type getType() const { return RadialGradientElement ; }

    Length cx_, cy_, r_, fx_, fy_ ;
} ;

enum class PatternUnits { UserSpaceOnUse, ObjectBoundingBox } ;

class Pattern: public Element, public Stylable, public FitToViewBox, public Container
{
  public:


    Pattern(): pattern_content_units_(PatternUnits::UserSpaceOnUse), pattern_units_(PatternUnits::ObjectBoundingBox) {  }

    Type getType() const { return Element::PatternElement ; }

    void parseAttributes(const Dictionary &pNode) ;
    bool fromXml(const Dictionary &pNode) ;

    void render(RenderingContext *ctx) ;

    enum Fallback { PatternUnitsDefined = 0x01, PatternContentUnitsDefined = 0x02, TransformDefined = 0x04 } ;

    void ResolveHRef(RenderingContext *ctx) ;

    PatternUnits pattern_units_ ;
    PatternUnits pattern_content_units_ ;
    Transform trans_ ;
    std::string href_ ;
    Length x_, y_, width_, height_ ;

} ;


class Image: public Element, public Transformable, public Stylable
{
public:

    Image() {
        x_ = y_ = width_ = height_ = 0 ;
    }
    Type getType() const { return ImageElement ; }

    bool fromXml(const Dictionary &pNode) ;
    virtual void render(RenderingContext *)  ;

    std::string img_path_ ;
    Length x_, y_, width_, height_ ;
    PreserveAspectRatio preserve_aspect_ratio_ ;
} ;

class ClipPath: public Element, public Transformable, public Stylable, public Container
{
  public:

    enum ClipPathUnits { UserSpaceOnUse, ObjectBoundingBox } ;

    ClipPath(): clip_path_units_(UserSpaceOnUse) {}

    Type getType() const { return ClipPathElement ; }

    bool fromXml(const Dictionary &pNode) ;

    virtual void render(RenderingContext *) ;

    ClipPathUnits clip_path_units_ ;

} ;

class Use: public Element, public Transformable, public Stylable
{
     public:

    Use() { x_ = y_ =  0.0 ; }
    ~Use() { }

    Type getType() const { return UseElement ; }

    bool fromXml(const Dictionary &pNode)  ;
    virtual void render(RenderingContext *) ;

    Length x_, y_, width_, height_ ;
    std::string ref_id_ ;
} ;


class Group: public Element, public Transformable, public Stylable, public Container
{
  public:

    Group() {}

    Type getType() const { return GroupElement ; }

    bool fromXml(const Dictionary &pNode)  ;
    virtual void render(RenderingContext *)  ;
} ;

class Defs: public Element, public Transformable, public Stylable, public Container
{
  public:

    Defs() {}

    Type getType() const { return DefsElement ; }

    bool fromXml(const Dictionary &pNode)  ;
    virtual void render(RenderingContext *)  {}
} ;

class Path: public Element, public Transformable, public Stylable
{
  public:

    Path() {}

    Type getType() const { return PathElement ; }

    bool fromXml(const Dictionary &pNode) ;
    virtual void render(RenderingContext *) ;

    PathData data_ ;
} ;

class Rect: public Element, public Stylable, public Transformable
{
  public:

    Rect() {}

    Type getType() const { return RectElement ; }

    bool fromXml(const Dictionary &pNode) ;
    virtual void render(RenderingContext *)  ;

    Length x_, y_, width_, height_, rx_, ry_ ;

} ;

class Circle: public Element,  public Stylable, public Transformable
{
  public:

    Circle() {}

    Type getType() const { return CircleElement ; }

    bool fromXml(const Dictionary &pNode) ;
    void render(RenderingContext *) ;

    Length cx_, cy_, r_ ;

} ;

class Line: public Element,  public Stylable, public Transformable
{
  public:

    Line() {}

    Type getType() const { return LineElement ; }

    bool fromXml(const Dictionary &pNode) ;
    void render(RenderingContext *) ;

    Length x1_, y1_, x2_, y2_ ;
} ;

class Ellipse: public Element, public Stylable, public Transformable
{
  public:

    Ellipse() {}

    Type getType() const { return EllipseElement ; }

    bool fromXml(const Dictionary &pNode) ;
    void render(RenderingContext *ctx) ;

    Length cx_, cy_, rx_, ry_ ;

} ;

class PolyLine: public Element, public Stylable, public Transformable
{
  public:

    PolyLine() {}

    Type getType() const { return PolyLineElement ; }

    bool fromXml(const Dictionary &pNode) ;
    void render(RenderingContext *ctx) ;

    PointList points_ ;
} ;

class Polygon: public Element , public Stylable, public Transformable
{
  public:

    Polygon() {}

    Type getType() const { return PolygonElement ; }

    bool fromXml(const Dictionary &pNode) ;
    void render(RenderingContext *ctx) ;

    PointList points_ ;

} ;

class TextPosElement: public Element, public Stylable
{
  public:

    TextPosElement() {
        x_ = y_ = 0 ;
        dx_ = dy_ = 0 ;
        preserve_white_ = false ;
    }
    bool parseAttributes(const Dictionary &pNode) ;

    Length x_, y_, dx_, dy_ ;
    bool preserve_white_ ;

} ;

class SpanElement ;
class Text: public TextPosElement, public Transformable, public Container
{
  public:

    Text() {}


    Type getType() const { return TextElement ; }

    bool fromXml(const Dictionary &pNode) ;
    void render(RenderingContext *ctx) ;
} ;


class SpanElement: public TextPosElement, public Container
{
  public:

    SpanElement() {}

    Type getType() const { return TSpanElement ; }

    bool fromXml(const Dictionary &pNode) ;
    void render(RenderingContext *ctx) ;

    std::string text_ ;

} ;

template<typename T>
inline void parse_element_attribute(const char *name, const Dictionary &attr, T &t) {
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
class DocumentNode: public Element, public Stylable, public FitToViewBox, public Container {

  public:

    DocumentNode(): x_{0.0}, y_{0.0},
        width_{1.0_perc}, height_{1.0_perc} {
    }

    void parseAttributes(const Dictionary &attrs) ;

    bool isContainer() const override { return true ; }

    Type getType() const { return DocumentElement ; }

    void parseCSS(const std::string &str) ;

    Length x_, y_, width_, height_ ;

} ;

class Symbol: public DocumentNode {

  public:

    Symbol() {}

    Type getType() const { return SymbolElement ; }
    //bool fromXml(const Dictionary &pNode) ;
    void render(RenderingContext *ctx) {}

} ;








} // namespace svg


} //namespace xg




#endif

