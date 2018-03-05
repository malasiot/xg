#ifndef __XG_SVG_STYLE_HPP__
#define __XG_SVG_STYLE_HPP__

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cassert>

#include "svg_length.hpp"

#include <xg/util/variant.hpp>
#include <xg/color.hpp>

namespace xg {
namespace svg {

enum class StyleAttributeType { Font, FontFamily, FontSize, FontSizeAdjust,
    FontStretch, FontStyle, FontVariant, FontWeight,
    TextDirection, TextLetterSpacing, TextDecoration, TextUnicodeBidi,
    TextWordSpacing, Clip, Color, Cursor, Display, Overflow,
    Visibility, ClipPath, ClipRule, Mask, Opacity, EnableBackground,
    Filter, FloodColor, FloodOpacity, LightingColor, StopColor, StopOpacity,
    PointerEvents, ColorInterpolation, ColorInterpolationFilters, ColorProfile,
    ColorRendering, Fill, FillOpacity, FillRule, ImageRendering,
    Marker, MarkerEnd, MarkerMid, MarkerStart, ShapeRendering,
    Stroke, StrokeDashArray, StrokeDashOffset, StrokeLineCap, StrokeLineJoin,
    StrokeMiterLimit, StrokeOpacity, StrokeWidth, TextRendering,
    AlignmentBaseline, BaselineShift, DominantBaseline, GlyphOrientationHorizontal,
    GlyphOrientationVertical, Kerning, TextAnchor, WritingMode } ;

enum class FillRule { EvenOdd, NonZero } ;
enum class LineJoinType { Miter, Round, Bevel } ;
enum class LineCapType { Butt, Round, Square } ;
enum class FontStyle { Normal, Oblique, Italic } ;
enum class FontVariant { Normal, SmallCaps } ;
enum class FontWeight { Normal, Bold, Bolder, Lighter,
    W100, W200, W300, W400, W500, W600, W700, W800, W900 } ;
enum class FontStretch { UltraCondensed, ExtraCondensed, Condensed,
    Narrower, SemiCondensed, SemiExpanded, Expanded,
    Wider, ExtraExpanded, UltraExpanded } ;
enum class FontSizeType { Length, XXSmall, XSmall, Small, Medium, Large, XLarge, XXLarge, Larger, Smaller } ;

enum class TextDecoration { Underline, Overline, Strike } ;
enum class TextAnchor { Start, Middle, End } ;
enum class ShapeQuality { Auto, OptimizeSpeed, CrispEdges, GeometricPrecision } ;
enum class TextQuality { Auto, OptimizeSpeed, OptimizeLegibility, GeometricPrecision } ;

enum class DisplayMode { None, Inline, BBox } ;
enum class VisibilityMode { None, Hidden } ;

enum class PaintType { None, SolidColor, CurrentColor, PaintServer } ;
enum class OverflowType { Visible, Hidden, Scroll, Auto } ;

struct FontSize {
    FontSize(FontSizeType type): type_(type) {}
    FontSize(Length l): val_(l), type_(FontSizeType::Length) {}

    FontSizeType type_ ;
    Length val_ ;
};

struct Paint {
    Paint(): type_(PaintType::None) {}

    PaintType type_ ;

    Variant<CSSColor, std::string> clr_or_server_id_ ;
} ;

class Style
{
  public:

    using dash_array_t = std::vector<Length> ;

    FillRule getfillRule() const { return findAttribute<FillRule>(StyleAttributeType::FillRule, FillRule::NonZero) ;  }
    std::string getClipPath() const { return findAttribute<std::string>(StyleAttributeType::ClipPath, std::string()) ; }
    ShapeQuality getShapeQuality() const { return findAttribute<ShapeQuality>(StyleAttributeType::ShapeRendering, ShapeQuality::Auto) ;}
    Length getStrokeWidth() const { return findAttribute<Length>(StyleAttributeType::StrokeWidth, 1.0) ;}
    float getMiterLimit() const { return findAttribute<float>(StyleAttributeType::StrokeMiterLimit, 1.0) ;}
    LineCapType getLineCap() const { return findAttribute<LineCapType>(StyleAttributeType::StrokeLineCap, LineCapType::Round) ;}
    LineJoinType getLineJoin() const { return findAttribute<LineJoinType>(StyleAttributeType::StrokeLineJoin, LineJoinType::Round) ;}

    dash_array_t getDashArray() const { return findAttribute<dash_array_t>(StyleAttributeType::StrokeDashArray, dash_array_t()) ; }
    Length getDashOffset() const { return findAttribute<Length>(StyleAttributeType::StrokeDashOffset, 0.0) ; }

    Paint getFillPaint() const { return findAttribute<Paint>(StyleAttributeType::Fill, Paint()) ; }
    Paint getStrokePaint() const { return findAttribute<Paint>(StyleAttributeType::Stroke, Paint()) ; }
    float getFillOpacity() const { return findAttribute<float>(StyleAttributeType::FillOpacity, 1.0) ; }
    float getOpacity() const { return findAttribute<float>(StyleAttributeType::Opacity, 1.0) ; }


    void parseNameValue(const std::string &name, const std::string &val) ;
    void fromStyleString(const std::string &str) ;

    bool hasAttribute(StyleAttributeType f) const {
        return attributes_.find(f) != attributes_.end() ;
    }


    template<typename T>
    T findAttribute(StyleAttributeType type, const T &default_val) const {
        auto it = attributes_.find(type) ;
        if ( it == attributes_.end() ) return default_val ;
        else {
            return it->second.get<T>() ;
        }
    }

    template<typename T>
    void setAttribute(StyleAttributeType t, const T&val) {
        attributes_[t].set<T>(val) ;
    }

    void extend(const Style &other) {
        for( const auto &lp: other.attributes_ ) {
            attributes_.insert({lp.first, lp.second}) ;
        }
    }

private:


    using attribute_value_t = Variant<float, std::string, Length, CSSColor, Paint, dash_array_t, FillRule, LineJoinType, LineCapType,  FontStyle,
        FontVariant, FontWeight, FontStretch, FontSize, TextDecoration, TextAnchor,
        ShapeQuality, TextQuality, DisplayMode, VisibilityMode, OverflowType>;

    std::map<StyleAttributeType, attribute_value_t> attributes_ ;

    void parsePaint(const std::string &str, Paint &p)  ;
} ;

} // namespace svg
} //namespace xg




#endif

