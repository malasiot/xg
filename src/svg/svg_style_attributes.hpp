#ifndef __XG_SVG_STYLE_ATTRIBUTES_HPP__
#define __XG_SVG_STYLE_ATTRIBUTES_HPP__

#include "svg_length.hpp"
#include "svg_dom_exceptions.hpp"
#include "svg_parse_util.hpp"

#include <xg/util/variant.hpp>
#include <xg/color.hpp>

namespace xg {
namespace svg {

enum class FillRule { EvenOdd, NonZero } ;
enum class ClipRule { EvenOdd, NonZero } ;
enum class LineJoinType { Miter, Round, Bevel } ;
enum class LineCapType { Butt, Round, Square } ;
enum class FontStyle { Normal, Oblique, Italic } ;
enum class FontVariant { Normal, SmallCaps } ;
enum class FontWeight { Normal, Bold, Bolder, Lighter,
                        W100, W200, W300, W400, W500, W600, W700, W800, W900 } ;
enum class FontStretch { Normal, UltraCondensed, ExtraCondensed, Condensed,
                         Narrower, SemiCondensed, SemiExpanded, Expanded,
                         Wider, ExtraExpanded, UltraExpanded } ;
enum class FontSizeType { Length, XXSmall, XSmall, Small, Medium, Large, XLarge, XXLarge, Larger, Smaller } ;

enum class TextDecoration { None, Underline, Overline, Strike } ;
enum class TextAnchor { Start, Middle, End } ;
enum class ShapeQuality { Auto, OptimizeSpeed, CrispEdges, GeometricPrecision } ;
enum class TextQuality { Auto, OptimizeSpeed, OptimizeLegibility, GeometricPrecision } ;

enum class DisplayMode { None, Inline, BBox } ;
enum class VisibilityMode { Visible, Hidden, Collapse } ;

enum class PaintType { None, SolidColor, CurrentColor, PaintServer } ;
enum class OverflowType { Visible, Hidden, Scroll, Auto } ;

struct FontSize {

    FontSize(FontSizeType type): type_(type) {}
    FontSize(Length l): val_(l), type_(FontSizeType::Length) {}

    bool parse(const std::string &v) ;
    FontSizeType type_ ;
    Length val_ ;
protected:
    friend class Style ;
    FontSize() = default ;
};

class URI {
public:
    URI() = default ;

    bool parse(const std::string &v) {
        id_ = parse_uri(v) ;
        return !id_.empty() ;
    }

    const std::string id() const { return id_ ; }

protected:

    std::string id_ ;

} ;


struct Paint {
    Paint(PaintType t): type_(t) {}

    bool parse(const std::string &val) ;

    PaintType type() const { return type_ ; }

    const CSSColor &color() const { return clr_or_server_id_.get<CSSColor>() ; }
    const std::string serverId() const { return clr_or_server_id_.get<std::string>() ; }

protected:

    PaintType type_ ;

    Variant<CSSColor, std::string> clr_or_server_id_ ;
} ;

struct FillPaint: public Paint {
    FillPaint(const std::string &id): Paint(PaintType::PaintServer) {
        clr_or_server_id_.set<std::string>(id) ;
    }
    FillPaint(): Paint(PaintType::SolidColor) {
        clr_or_server_id_.set<CSSColor>(0, 0, 0) ;
    }
    FillPaint(const CSSColor &clr): Paint(PaintType::SolidColor) {
        clr_or_server_id_.set<CSSColor>(clr) ;
    }
} ;

struct StrokePaint: public Paint {
    StrokePaint(const std::string &id): Paint(PaintType::PaintServer) {
        clr_or_server_id_.set<std::string>(id) ;
    }
    StrokePaint(): Paint(PaintType::None) {
    }
} ;

#define SVG_STYLE_ATTRIBUTE(aname, atype, avar, adef)\
    protected:\
    std::shared_ptr<atype> avar ;\
    public:\
    void set##aname(const atype &v) {\
    if ( !avar ) avar.reset(new atype(v)) ;\
    else *avar = v ;\
}\
    atype get##aname() const {\
    if ( !avar ) return adef ;\
    else return *avar ;\
}\



}
}











#endif
