#ifndef __XG_SVG_DOM_HPP__
#define __XG_SVG_DOM_HPP__

#include <string>
#include <xg/util/dictionary.hpp>
#include <xg/xform.hpp>
#include <xg/path.hpp>
#include <xg/canvas.hpp>

#include "svg_length.hpp"
#include "svg_dom_exceptions.hpp"
#include "svg_style.hpp"
#include "svg_dom_attributes.hpp"

namespace xg {
namespace svg {

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
class TSpanElement ;
class TRefElement ;
class SVGElement ;
class StyleElement ;
class Element ;

using ElementPtr = std::shared_ptr<Element> ;


class Stylable {
public:
    SVG_ELEMENT_ATTRIBUTE(style_, style, Style, Style())
} ;

class Transformable {
public:
    SVG_ELEMENT_ATTRIBUTE(trans_, trans, Matrix2d, Matrix2d())
} ;

class FitToViewBox {
public:

    SVG_ELEMENT_ATTRIBUTE(view_box_, viewBox, ViewBox, ViewBox())
    SVG_ELEMENT_ATTRIBUTE(preserve_aspect_ratio_, preserveAspectRatio, PreserveAspectRatio, PreserveAspectRatio())
} ;


enum class WhiteSpaceProcessing { Default, Preserve } ;

class Element
{
public:

    Element() = default ;
    virtual ~Element() = default ;

    void parseElementAttributes(const Dictionary &a) ;
    void parseStyleAttributes(const Dictionary &p, OptionalAttribute<Style> &style) ;
    void parseTransformAttribute(const Dictionary &p, OptionalAttribute<Matrix2d> &t) ;
    void parseViewBoxAttributes(const Dictionary &p, OptionalAttribute<ViewBox> &view_box,
                                OptionalAttribute<PreserveAspectRatio> &preserve_aspect_ratio );
    void parseTextPosAttributes(const Dictionary &attrs, OptionalAttribute<Length> &x, OptionalAttribute<Length> &y,
                                OptionalAttribute<Length> &dx, OptionalAttribute<Length> &dy);

    virtual bool canHaveChild(const ElementPtr &p) const { return false ; }

    bool addChild(const ElementPtr &p) {
        if ( !canHaveChild(p) ) return false ;
        children_.emplace_back(p) ;
        p->parent_ = this ;
        return true ;
    }

    void setDocument(SVGDocument *doc) { root_ = doc ; }

    SVGDocument &document() { return *root_;}

    std::string id() const ;

    WhiteSpaceProcessing space() const { return ws_ ; }

    const std::vector<ElementPtr> &children() const { return children_ ; }

    Element *parent() const { return parent_ ; }

protected:

    std::string id_ ;
    Element *parent_ = nullptr ;
    SVGDocument *root_ ;
    WhiteSpaceProcessing ws_ = WhiteSpaceProcessing::Default ;

    std::vector<ElementPtr> children_ ;
} ;



// Helper class to define container elements that can contain children belonging only within a set of types

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

class StyleElement: public Element {
public:

    StyleElement() = default ;

    void parseAttributes(const Dictionary &) ;

    SVG_ELEMENT_ATTRIBUTE(type_, type, std::string, std::string())
    SVG_ELEMENT_ATTRIBUTE(media_, media, std::string, std::string())
    SVG_ELEMENT_ATTRIBUTE(title_, title, std::string, std::string())
} ;


class StopElement: public Element, public Stylable {
public:

    StopElement() = default ;

    void parseAttributes(const Dictionary &p) ;

    SVG_ELEMENT_ATTRIBUTE(offset_, offset, float, 1.0)

} ;



enum class GradientSpreadMethod { Unknown, Pad, Reflect, Repeat } ;
enum class GradientUnits { UserSpaceOnUse, ObjectBoundingBox } ;

class GradientElement: public Container<StopElement>, public Stylable {
public:

    GradientElement() = default ;

    void parseAttributes(const Dictionary &pNode) ;

    GradientSpreadMethod spreadMethodInherited() ;
    GradientUnits gradientUnitsInherited() ;
    Matrix2d gradientTransformInherited() ;

    void collectStops(std::vector<StopElement *> &stops) ;

    SVG_ELEMENT_ATTRIBUTE(spread_method_, spreadMethod, GradientSpreadMethod, GradientSpreadMethod::Pad)
    SVG_ELEMENT_ATTRIBUTE(gradient_units_, gradientUnits, GradientUnits, GradientUnits::ObjectBoundingBox)
    SVG_ELEMENT_ATTRIBUTE(trans_, gradientTransform, Matrix2d, Matrix2d())
    SVG_ELEMENT_ATTRIBUTE(href_, href, URIReference, URIReference{})
} ;

class LinearGradientElement: public GradientElement {
public:

    LinearGradientElement() = default ;

    void parseAttributes(const Dictionary &a) ;

    Length x1_inherited() ;
    Length y1_inherited() ;
    Length x2_inherited() ;
    Length y2_inherited() ;

    SVG_ELEMENT_ATTRIBUTE(x1_, x1, Length, 0.0_perc)
    SVG_ELEMENT_ATTRIBUTE(y1_, y1, Length, 0.0_perc)
    SVG_ELEMENT_ATTRIBUTE(x2_, x2, Length, 100.0_perc)
    SVG_ELEMENT_ATTRIBUTE(y2_, y2, Length, 0.0_perc)

} ;

class RadialGradientElement: public GradientElement {
public:

    RadialGradientElement() = default ;

    void parseAttributes(const Dictionary &a) ;

    Length cx_inherited() ;
    Length cy_inherited() ;
    Length fx_inherited() ;
    Length fy_inherited() ;
    Length r_inherited() ;

    SVG_ELEMENT_ATTRIBUTE(cx_, cx, Length, 50._perc)
    SVG_ELEMENT_ATTRIBUTE(cy_, cy, Length, 50._perc)
    SVG_ELEMENT_ATTRIBUTE(fx_, fx, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(fy_, fy, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(r_, r, Length, 50._perc)

} ;

enum class PatternUnits { UserSpaceOnUse, ObjectBoundingBox } ;

class PatternElement:
        public GroupContainer,
        public Stylable

{
public:

    PatternElement() = default ;

    void parseAttributes(const Dictionary &a) ;

    PatternUnits patternUnitsInherited() ;
    PatternUnits patternContentUnitsInherited() ;
    Matrix2d patternTransformInherited() ;
    Length xInherited() ;
    Length yInherited() ;
    Length widthInherited() ;
    Length heightInherited() ;
    ViewBox viewBoxInherited() ;
    PreserveAspectRatio preserveAspectRatioInherited() ;


    void collectChildren(std::vector<Element *> &children);

    SVG_ELEMENT_ATTRIBUTE(pattern_units_, patternUnits, PatternUnits, PatternUnits::ObjectBoundingBox)
    SVG_ELEMENT_ATTRIBUTE(pattern_content_units_, patternContentUnits, PatternUnits, PatternUnits::UserSpaceOnUse)
    SVG_ELEMENT_ATTRIBUTE(trans_, patternTransform, Matrix2d, Matrix2d())

    SVG_ELEMENT_ATTRIBUTE(x_, x, Length, 0.0)
    SVG_ELEMENT_ATTRIBUTE(y_, y, Length, 0.0)
    SVG_ELEMENT_ATTRIBUTE(width_, width, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(height_, height, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(view_box_, viewBox, ViewBox, ViewBox())
    SVG_ELEMENT_ATTRIBUTE(preserve_aspect_ratio_, preserveAspectRatio, PreserveAspectRatio, PreserveAspectRatio())

    SVG_ELEMENT_ATTRIBUTE(href_, href, URIReference, URIReference{})
} ;


class ImageElement: public Element, public Transformable, public Stylable
{
public:

    ImageElement() = default ;

    void parseAttributes(const Dictionary &attrs) ;

    SVG_ELEMENT_ATTRIBUTE(uri_, uri, std::string, std::string())
    SVG_ELEMENT_ATTRIBUTE(x_, x, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(y_, y, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(width_, width, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(height_, height, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(preserve_aspect_ratio_, preserveAspectRatio, PreserveAspectRatio, PreserveAspectRatio{})

} ;

enum ClipPathUnits { UserSpaceOnUse, ObjectBoundingBox } ;

class ClipPathElement:
        public ShapeContainer,
        public Transformable, public Stylable
{
public:

    ClipPathElement() = default ;

    void parseAttributes(const Dictionary &a) ;

    SVG_ELEMENT_ATTRIBUTE(clip_path_units_, clipPathUnits, ClipPathUnits, ClipPathUnits::UserSpaceOnUse)
} ;

class UseElement: public GroupContainer, public Transformable, public Stylable
{
public:
    UseElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    SVG_ELEMENT_ATTRIBUTE(href_, href, URIReference, URIReference{})
    SVG_ELEMENT_ATTRIBUTE(x_, x, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(y_, y, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(width_, width, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(height_, height, Length, 0)
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

    SVG_ELEMENT_ATTRIBUTE(data_, data, PathData, PathData())
} ;

class RectElement: public Element, public Stylable, public Transformable {
public:

    RectElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    SVG_ELEMENT_ATTRIBUTE(x_, x, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(y_, y, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(width_, width, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(height_, height, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(rx_, rx, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(ry_, ry, Length, 0)
} ;

class CircleElement: public Element,  public Stylable, public Transformable {
public:

    CircleElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    SVG_ELEMENT_ATTRIBUTE(cx_, cx, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(cy_, cy, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(r_, r, Length, 0)
} ;

class LineElement: public Element,  public Stylable, public Transformable {
public:

    LineElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    SVG_ELEMENT_ATTRIBUTE(x1_, x1, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(y1_, y1, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(x2_, x2, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(y2_, y2, Length, 0)
} ;

class EllipseElement: public Element, public Stylable, public Transformable {
public:

    EllipseElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    SVG_ELEMENT_ATTRIBUTE(cx_, cx, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(cy_, cy, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(rx_, rx, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(ry_, ry, Length, 0)
} ;

class PolylineElement: public Element, public Stylable, public Transformable {
public:

    PolylineElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    SVG_ELEMENT_ATTRIBUTE(points_, points, PointList, PointList{})
} ;

class PolygonElement: public Element , public Stylable, public Transformable {
public:

    PolygonElement() = default ;

    void parseAttributes(const Dictionary &a)  ;

    SVG_ELEMENT_ATTRIBUTE(points_, points, PointList, PointList{})
} ;

enum class LengthAdjust { Spacing, GlyphsAndSpacing } ;

class TextPosElement {
public:
    SVG_ELEMENT_ATTRIBUTE(x_, x, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(y_, y, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(dx_, dx, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(dy_, dy, Length, 0)
} ;

class TextContentElement {
public:

    SVG_ELEMENT_ATTRIBUTE(text_length_, textLength, Length, 0)
    SVG_ELEMENT_ATTRIBUTE(adjust_, lengthAdjust, LengthAdjust, LengthAdjust::Spacing)
} ;

class TextElement: public Container<TextElement, TSpanElement, TRefElement>,
        public TextPosElement, public TextContentElement, public Stylable, public Transformable {
public:

    TextElement() {}

    void parseAttributes(const Dictionary &a) ;
} ;

class TRefElement: public Element, public TextPosElement, public Stylable {
public:

    TRefElement() = default ;

    void parseAttributes(const Dictionary &a) ;

    SVG_ELEMENT_ATTRIBUTE(href_, href, URIReference, URIReference{})
} ;


class TSpanElement:  public Container<TSpanElement, TRefElement>, public TextPosElement, public TextContentElement, public Stylable {
public:

    TSpanElement() = default ;
    TSpanElement(const std::string &text): text_(text) {}

    void parseAttributes(const Dictionary &a) ;

    std::string text_ ;
} ;


class SVGElement: public GroupContainer, public Stylable, public FitToViewBox {

public:

    SVGElement() = default ;

    void parseAttributes(const Dictionary &attrs) ;

    SVG_ELEMENT_ATTRIBUTE(x_, x, Length, 0.0)
    SVG_ELEMENT_ATTRIBUTE(y_, y, Length, 0.0)
    SVG_ELEMENT_ATTRIBUTE(width_, width, Length, 100.0_perc)
    SVG_ELEMENT_ATTRIBUTE(height_, height, Length, 100.0_perc)
} ;

class SymbolElement:  public GroupContainer, public Stylable, public FitToViewBox {

public:

    SymbolElement() = default ;

    void parseAttributes(const Dictionary &attrs) ;

    SVG_ELEMENT_ATTRIBUTE(x_, x, Length, 0.0)
    SVG_ELEMENT_ATTRIBUTE(y_, y, Length, 0.0)
    SVG_ELEMENT_ATTRIBUTE(width_, width, Length, 100.0_perc)
    SVG_ELEMENT_ATTRIBUTE(height_, height, Length, 100.0_perc)

} ;

class UnsupportedElement: public Element {
public:
    bool canHaveChild(const std::shared_ptr<Element> &p) const override { return true ; }
    void parseAttributes(const Dictionary &) {}
};

} // namespace svg
} //namespace xg




#endif

