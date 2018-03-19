#ifndef __XG_SVG_STYLE_HPP__
#define __XG_SVG_STYLE_HPP__

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cassert>

#include "svg_style_attributes.hpp"



namespace xg {
namespace svg {

class Element ;


class Style
{
public:

    Style(const Style &other);
    Style() = default ;

    SVG_STYLE_ATTRIBUTE(FillRule, FillRule, fill_rule_, FillRule::NonZero)
    SVG_STYLE_ATTRIBUTE(ClipRule, ClipRule, clip_rule_, ClipRule::NonZero)
    SVG_STYLE_ATTRIBUTE(ClipPath, URI, clip_path_, URI())
    SVG_STYLE_ATTRIBUTE(ShapeQuality, ShapeQuality, shape_quality_, ShapeQuality::Auto)
    SVG_STYLE_ATTRIBUTE(StrokeWidth, Length, stroke_width_, 1.0)
    SVG_STYLE_ATTRIBUTE(DashOffset,  Length, dash_offset_, 0.0)
    SVG_STYLE_ATTRIBUTE(FontSize,  FontSize, font_size_, FontSizeType::Medium)
    SVG_STYLE_ATTRIBUTE(MiterLimit,  float, miter_limit_, 1.0)
    SVG_STYLE_ATTRIBUTE(LineCap,  LineCapType, line_cap_, LineCapType::Round)
    SVG_STYLE_ATTRIBUTE(LineJoin,  LineJoinType, line_join_, LineJoinType::Round)
    SVG_STYLE_ATTRIBUTE(DashArray,  LengthList, dash_array_, LengthList())
    SVG_STYLE_ATTRIBUTE(FillPaint,  FillPaint, fill_paint_, FillPaint())
    SVG_STYLE_ATTRIBUTE(StrokePaint,  StrokePaint, stroke_paint_, StrokePaint())
    SVG_STYLE_ATTRIBUTE(FillOpacity,  float, fill_opacity_, 1.0)
    SVG_STYLE_ATTRIBUTE(StrokeOpacity,  float, stroke_opacity_, 1.0)
    SVG_STYLE_ATTRIBUTE(Opacity,  float, opacity_, 1.0)
    SVG_STYLE_ATTRIBUTE(StopColor,  CSSColor, stop_color_, NamedColor::black())
    SVG_STYLE_ATTRIBUTE(StopOpacity,  float, stop_opacity_, 1.0)
    SVG_STYLE_ATTRIBUTE(Overflow,  OverflowType, overflow_, OverflowType::Hidden)
    SVG_STYLE_ATTRIBUTE(FontFamily,  std::string, font_family_, "serif")
    SVG_STYLE_ATTRIBUTE(FontStyle,  FontStyle, font_style_, FontStyle::Normal)
    SVG_STYLE_ATTRIBUTE(FontWeight, FontWeight, font_weight_, FontWeight::Normal)
    SVG_STYLE_ATTRIBUTE(FontVariant,  FontVariant, font_variant_, FontVariant::Normal)
    SVG_STYLE_ATTRIBUTE(FontStretch,  FontStretch, font_stretch_, FontStretch::Normal)
    SVG_STYLE_ATTRIBUTE(TextDecoration,  TextDecoration, text_decoration_, TextDecoration::None)
    SVG_STYLE_ATTRIBUTE(TextAnchor,  TextAnchor, text_anchor_, TextAnchor::Start)
    SVG_STYLE_ATTRIBUTE(Display,  DisplayMode, display_, DisplayMode::Inline)
    SVG_STYLE_ATTRIBUTE(Visibility,  VisibilityMode, visibility_, VisibilityMode::Visible)
    SVG_STYLE_ATTRIBUTE(TextQuality, TextQuality, text_quality_, TextQuality::Auto)

    void parseNameValue(const std::string &name, const std::string &val) ;
    void fromStyleString(const std::string &str) ;

    void extend(const Style &other) ;


private:

    template<typename T>
    void parseAttribute(const std::string &name, const std::string &val, std::shared_ptr<T> &a) {
        T tmp ;
        if ( !tmp.parse(val) )
            throw SVGDOMAttributeValueException(name, val) ;
        else
            a.reset(new T(tmp)) ;
    }

    static bool parseOpacity(const std::string &str, float &v);

} ;

template<>
void Style::parseAttribute(const std::string &name, const std::string &val, std::shared_ptr<CSSColor> &a) ;

} // namespace svg
} //namespace xg




#endif

