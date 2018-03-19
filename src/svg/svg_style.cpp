#include "svg_style.hpp"
#include "svg_parse_util.hpp"
#include "svg_dom_exceptions.hpp"

#include <xg/util/strings.hpp>

using namespace std ;

namespace xg {
namespace svg {


bool Paint::parse(const std::string &val)
{
    if ( val == "none" )
        type_ = PaintType::None ;
    else if ( val == "currentColor" )
        type_ = PaintType::CurrentColor ;
    else if ( startsWith(val, "url") ) {
        string id = parse_uri(val) ;

        if ( !id.empty() ) {
            clr_or_server_id_.set<string>(id) ;
            type_ = PaintType::PaintServer ;
        }
        else return false ;
    }
    else
    {
        try {
            CSSColor clr(val) ;

            clr_or_server_id_.set<CSSColor>(clr) ;
            type_ = PaintType::SolidColor ;
        }
        catch ( CSSColorParseException &e ) {
            return false ;
        }
    }

    return true ;
}

bool Style::parseOpacity(const string &str, float &v) {
    return parse_number(str, v) ;
}

void Style::parseNameValue(const string &name, const string &value) {
    string val = trimCopy(value) ;

    if ( val == "inherit" ) return ;

    if ( name == "fill-rule" )  {
        if ( val == "nonzero" )
            setFillRule(FillRule::NonZero) ;
        else if ( val == "evenodd" )
            setFillRule(FillRule::EvenOdd) ;
    }
    else if ( name == "fill-opacity" )  {
        float v ;
        if ( parseOpacity(val, v) ) setFillOpacity(v) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "stroke-opacity" )  {
        float v ;
        if ( parseOpacity(val, v) ) setStrokeOpacity(v) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "opacity" )  {
        float v ;
        if ( parseOpacity(val, v) ) setOpacity(v) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "clip-rule" ) {
        if ( val == "nonzero" )
            setClipRule(ClipRule::NonZero) ;
        else if ( val == "evenodd" )
            setClipRule(ClipRule::EvenOdd) ;
    }
    else if ( name == "fill") {
        parseAttribute(name, val, fill_paint_) ;
    }
    else if ( name == "stroke" )
    {
        parseAttribute(name, val, stroke_paint_) ;
    }
    else if ( name == "stroke-width" ) {
        parseAttribute(name, val, stroke_width_) ;
    }
    else if ( name == "stroke-miterlimit" ) {
        float v ;
        if ( parse_number(val, v) )
            setMiterLimit(v) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "stroke-dasharray" ) {
        if ( val == "none" ) ;
        else {
            parseAttribute(name, val, dash_array_) ;
        }
    }
    else if ( name == "stroke-dashoffset" ) {
        parseAttribute(name, val, dash_offset_) ;
    }
    else if ( name == "stroke-linejoin" )  {
        if ( val == "miter" )
            setLineJoin(LineJoinType::Miter) ;
        else if ( val == "round" )
            setLineJoin(LineJoinType::Round) ;
        else if ( val == "bevel" )
            setLineJoin(LineJoinType::Bevel) ;
    }
    else if ( name == "stroke-linecap" )  {
        if ( val == "butt" )
            setLineCap(LineCapType::Butt) ;
        else if ( val == "round" )
            setLineCap(LineCapType::Round) ;
        else if ( val == "square" )
            setLineCap(LineCapType::Square) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "font-family" ) {
        setFontFamily(val) ;
    }
    else if ( name == "font-style" ) {
        if ( val == "normal" )
            setFontStyle(FontStyle::Normal) ;
        else if ( val == "oblique" )
            setFontStyle(FontStyle::Oblique) ;
        else if ( val == "italic" )
            setFontStyle(FontStyle::Italic) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "font-variant" ) {
        if ( val == "normal" )
            setFontVariant(FontVariant::Normal) ;
        else if ( val == "small-caps" )
            setFontVariant(FontVariant::SmallCaps) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "font-weight" ) {
        if ( val == "normal" )
            setFontWeight(FontWeight::Normal) ;
        else if ( val == "bold" )
            setFontWeight(FontWeight::Bold) ;
        else if ( val == "bolder" )
            setFontWeight(FontWeight::Bolder) ;
        else if ( val == "lighter" )
            setFontWeight(FontWeight::Lighter) ;
        else if ( val == "100" )
            setFontWeight(FontWeight::W100) ;
        else if ( val == "200" )
            setFontWeight(FontWeight::W200) ;
        else if ( val == "300" )
            setFontWeight(FontWeight::W300) ;
        else if ( val == "400" )
            setFontWeight(FontWeight::W400) ;
        else if ( val == "500" )
            setFontWeight(FontWeight::W500) ;
        else if ( val == "600" )
            setFontWeight(FontWeight::W600) ;
        else if ( val == "700" )
            setFontWeight(FontWeight::W700) ;
        else if ( val == "800" )
            setFontWeight(FontWeight::W800) ;
        else if ( val == "900" )
            setFontWeight(FontWeight::W900) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "font-stretch" ) {
        if ( val == "ultra-condensed" )
            setFontStretch(FontStretch::UltraCondensed) ;
        else if ( val == "extra-condensed" )
            setFontStretch(FontStretch::ExtraCondensed) ;
        else if ( val == "condensed" )
            setFontStretch(FontStretch::Condensed) ;
        else if ( val == "narrower" )
            setFontStretch(FontStretch::Narrower) ;
        else if ( val == "semi-condensed" )
            setFontStretch(FontStretch::SemiCondensed) ;
        else if ( val == "semi-expanded" )
            setFontStretch(FontStretch::SemiExpanded) ;
        else if ( val == "expanded" )
            setFontStretch(FontStretch::Expanded) ;
        else if ( val == "wider" )
            setFontStretch(FontStretch::Wider) ;
        else if ( val == "extra-expanded" )
            setFontStretch(FontStretch::ExtraExpanded) ;
        else if ( val == "ultra-expanded" )
            setFontStretch(FontStretch::UltraExpanded) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "font-size" )  {
        parseAttribute(name, val, font_size_) ;
    }
    else if ( name == "text-decoration" ) {
        if ( val == "underline" )
            setTextDecoration(TextDecoration::Underline) ;
        else if ( val == "overline" )
            setTextDecoration(TextDecoration::Overline) ;
        else if ( val == "strike" )
            setTextDecoration(TextDecoration::Strike) ;
        else if ( val == "line-through" )
            setTextDecoration(TextDecoration::Overline) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "text-anchor" ) {
        if ( val == "start" )
            setTextAnchor(TextAnchor::Start) ;
        else if ( val == "middle" )
            setTextAnchor(TextAnchor::Middle) ;
        else if ( val == "end" )
            setTextAnchor(TextAnchor::End) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "display" ) {
        if ( val == "none" )
            setDisplay(DisplayMode::None) ;
        else if ( val == "inline" )
            setDisplay(DisplayMode::Inline) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "visibility" ) {
        if ( val == "visible" )
            setVisibility(VisibilityMode::Visible) ;
        else if ( val == "hidden" ||  val == "collapsed" )
            setVisibility(VisibilityMode::Hidden) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "shape-rendering" ) {
        if ( val == "auto" || val == "default" )
            setShapeQuality(ShapeQuality::Auto) ;
        else if ( val == "optimizeSpeed" )
            setShapeQuality(ShapeQuality::OptimizeSpeed) ;
        else if ( val == "crispEdges" )
            setShapeQuality(ShapeQuality::CrispEdges) ;
        else if ( val == "geometricPrecision" )
            setShapeQuality(ShapeQuality::GeometricPrecision) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "text-rendering" )  {
        if ( val == "auto" || val == "default" )
            setTextQuality(TextQuality::Auto) ;
        else if ( val == "optimizeSpeed" )
            setTextQuality(TextQuality::OptimizeSpeed) ;
        else if ( val == "optimizeLegibility" )
            setTextQuality(TextQuality::OptimizeLegibility) ;
        else if ( val == "geometricPrecision" )
            setTextQuality(TextQuality::GeometricPrecision) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "stop-opacity" ) {
        float v ;
        if ( parseOpacity(val, v) )
            setStopOpacity(v) ;
        else
            throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "stop-color" ) {
        parseAttribute(name, val, stop_color_) ;
    }
    else if ( name == "overflow" ) {
        if ( val == "visible" )
            setOverflow(OverflowType::Visible) ;
        else if ( val == "auto" )
            setOverflow(OverflowType::Auto) ;
        else if ( val == "hidden" )
            setOverflow(OverflowType::Hidden) ;
        else if ( val == "scroll" )
            setOverflow(OverflowType::Scroll) ;
        else throw SVGDOMAttributeValueException(name, val) ;
    }
    else if ( name == "clip-path" ) {
        parseAttribute(name, val, clip_path_) ;
    }

}

void Style::fromStyleString(const string &str) {
    static regex sr("([a-zA-Z-]+)[\\s]*:[\\s]*([^:;]+)[\\s]*[;]?") ;

    sregex_iterator it(str.begin(), str.end(), sr), end ;

    while ( it != end )  {
        parseNameValue(it->str(1), it->str(2)) ;
        ++it ;
    }
}

#define SVG_STYLE_ATTRIBUTE_INHERIT(a) a = other.a

Style::Style(const Style &other) {
    SVG_STYLE_ATTRIBUTE_INHERIT(fill_rule_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(clip_rule_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(clip_path_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(shape_quality_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(stroke_width_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(dash_offset_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(font_size_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(miter_limit_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(line_cap_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(line_join_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(dash_array_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(fill_paint_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(stroke_paint_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(fill_opacity_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(stroke_opacity_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(font_family_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(font_style_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(font_weight_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(font_variant_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(font_stretch_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(text_decoration_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(text_anchor_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(visibility_) ;
    SVG_STYLE_ATTRIBUTE_INHERIT(text_quality_) ;

}

#define SVG_STYLE_ATTRIBUTE_COPY(a) if ( other.a ) a = other.a

void Style::extend(const Style &other) {

    SVG_STYLE_ATTRIBUTE_COPY(fill_rule_) ;
    SVG_STYLE_ATTRIBUTE_COPY(clip_rule_) ;
    SVG_STYLE_ATTRIBUTE_COPY(clip_path_) ;
    SVG_STYLE_ATTRIBUTE_COPY(shape_quality_) ;
    SVG_STYLE_ATTRIBUTE_COPY(stroke_width_) ;
    SVG_STYLE_ATTRIBUTE_COPY(dash_offset_) ;
    SVG_STYLE_ATTRIBUTE_COPY(font_size_) ;
    SVG_STYLE_ATTRIBUTE_COPY(miter_limit_) ;
    SVG_STYLE_ATTRIBUTE_COPY(line_cap_) ;
    SVG_STYLE_ATTRIBUTE_COPY(line_join_) ;
    SVG_STYLE_ATTRIBUTE_COPY(dash_array_) ;
    SVG_STYLE_ATTRIBUTE_COPY(fill_paint_) ;
    SVG_STYLE_ATTRIBUTE_COPY(stroke_paint_) ;
    SVG_STYLE_ATTRIBUTE_COPY(fill_opacity_) ;
    SVG_STYLE_ATTRIBUTE_COPY(stroke_opacity_) ;
    SVG_STYLE_ATTRIBUTE_COPY(opacity_) ;
    SVG_STYLE_ATTRIBUTE_COPY(stop_color_) ;
    SVG_STYLE_ATTRIBUTE_COPY(stop_opacity_) ;
    SVG_STYLE_ATTRIBUTE_COPY(overflow_) ;
    SVG_STYLE_ATTRIBUTE_COPY(font_family_) ;
    SVG_STYLE_ATTRIBUTE_COPY(font_style_) ;
    SVG_STYLE_ATTRIBUTE_COPY(font_weight_) ;
    SVG_STYLE_ATTRIBUTE_COPY(font_variant_) ;
    SVG_STYLE_ATTRIBUTE_COPY(font_stretch_) ;
    SVG_STYLE_ATTRIBUTE_COPY(text_decoration_) ;
    SVG_STYLE_ATTRIBUTE_COPY(text_anchor_) ;
    SVG_STYLE_ATTRIBUTE_COPY(display_) ;
    SVG_STYLE_ATTRIBUTE_COPY(visibility_) ;
    SVG_STYLE_ATTRIBUTE_COPY(text_quality_) ;
}

bool FontSize::parse(const string &val) {
    if ( val == "xx-small" )
        type_ = FontSizeType::XXSmall ;
    else if ( val == "x-small" )
        type_ = FontSizeType::XSmall ;
    else if ( val == "small" )
        type_ = FontSizeType::Small ;
    else if ( val == "medium" )
        type_ = FontSizeType::Medium ;
    else if ( val == "large" )
        type_ = FontSizeType::Large ;
    else if ( val == "x-large" )
        type_ = FontSizeType::XLarge ;
    else if ( val == "xx-large" )
        type_ = FontSizeType::XXLarge ;
    else {
        type_ = FontSizeType::Length ;
        return val_.parse(val) ;
    }

    return true ;
}

template<>
void Style::parseAttribute(const std::string &name, const std::string &val, std::shared_ptr<CSSColor> &a) {

    try {
        CSSColor clr(val) ;
        a.reset(new CSSColor(clr)) ;
    }
    catch ( CSSColorParseException & ) {
        throw SVGDOMAttributeValueException(name, val) ;
    }

}


} // namespace svg
} // namespace xg
