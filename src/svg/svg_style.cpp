#include "svg_style.hpp"
#include "svg_parse_util.hpp"
#include "svg_dom_exceptions.hpp"

#include <xg/util/strings.hpp>

using namespace std ;

namespace xg {
namespace svg {


void Style::parsePaint(const std::string &val, Paint &p, Element *r, bool fill)
{
    if ( val == "none" )
        p.type_ = PaintType::None ;
    else if ( val == "currentColor" )
        p.type_ = PaintType::CurrentColor ;
    else if ( startsWith(val, "url") ) {
        string id = parse_uri(val) ;

        if ( !id.empty() ) {
            p.clr_or_server_id_.set<string>(id) ;
            p.type_ = PaintType::PaintServer ;
        }
        else throw SVGDOMAttributeValueException("invalid paint url") ;
    }
    else
    {
        try {
            CSSColor clr(val) ;

            p.clr_or_server_id_.set<CSSColor>(clr) ;
            p.type_ = PaintType::SolidColor ;
        }
        catch ( CSSColorParseException &e ) {
            throw SVGDOMAttributeValueException(e.what()) ;
        }
    }
}

void Style::parseNameValue(const string &name, const string &value, Element *e) {
    string val = trimCopy(value) ;

    if ( val == "inherit" ) return ;

    if ( name == "fill-rule" )  {
        if ( val == "nonzero" )
            setAttribute<FillRule>(StyleAttributeType::FillRule, FillRule::NonZero) ;
        else if ( val == "evenodd" )
            setAttribute<FillRule>(StyleAttributeType::FillRule, FillRule::NonZero) ;
    }
    else if ( name == "fill-opacity" )  {
        float v ;
        if ( parse_number(val, v) )
            setAttribute<float>(StyleAttributeType::FillOpacity, v) ;

    }
    else if ( name == "opacity" )  {
        float v ;
        if ( parse_number(val, v) )
            setAttribute<float>(StyleAttributeType::Opacity, v) ;
    }
    else if ( name == "stoke-opacity" ) {
        float v ;
        if ( parse_number(val, v) )
            setAttribute<float>(StyleAttributeType::StrokeOpacity, v) ;
    }
    else if ( name == "clip-rule" ) ;
    else if ( name == "fill") {
        FillPaint p ;
        parsePaint(val, p, e, true) ;
        setAttribute<FillPaint>(StyleAttributeType::Fill, p) ;
    }
    else if ( name == "stroke" )
    {
        StrokePaint p ;
        parsePaint(val, p, e, false) ;
        setAttribute<StrokePaint>(StyleAttributeType::Stroke, p) ;
    }
    else if ( name == "stroke-width" ) {
        Length sw = Length::fromString(val) ;
        setAttribute<Length>(StyleAttributeType::StrokeWidth, sw) ;
    }
    else if ( name == "stroke-miterlimit" ) {
        float v ;
        if ( parse_number(val, v) )
            setAttribute<float>(StyleAttributeType::StrokeMiterLimit, v) ;
    }
    else if ( name == "stroke-dasharray" ) {
        if ( val == "none" ) ;
        else {
            vector<Length> dash_array = Length::parseList(val) ;
            setAttribute<vector<Length>>(StyleAttributeType::StrokeDashArray, dash_array) ;
        }
    }
    else if ( name == "stroke-dashoffset" ) {
        setAttribute<Length>(StyleAttributeType::StrokeDashOffset, Length::fromString(val)) ;
    }
    else if ( name == "stroke-linejoin" )  {
        if ( val == "miter" )
            setAttribute<LineJoinType>(StyleAttributeType::StrokeLineJoin, LineJoinType::Miter) ;
        else if ( val == "round" )
            setAttribute<LineJoinType>(StyleAttributeType::StrokeLineJoin, LineJoinType::Round) ;
        else if ( val == "bevel" )
            setAttribute<LineJoinType>(StyleAttributeType::StrokeLineJoin, LineJoinType::Bevel) ;
    }
    else if ( name == "stroke-linecap" )  {
        if ( val == "butt" )
            setAttribute<LineCapType>(StyleAttributeType::StrokeLineCap, LineCapType::Butt) ;
        else if ( val == "round" )
            setAttribute<LineCapType>(StyleAttributeType::StrokeLineCap, LineCapType::Round) ;
        else if ( val == "square" )
            setAttribute<LineCapType>(StyleAttributeType::StrokeLineCap, LineCapType::Square) ;
    }
    else if ( name == "font-family" ) {
        setAttribute<string>(StyleAttributeType::FontFamily, val) ;
    }
    else if ( name == "font-style" ) {
        if ( val == "normal" )
            setAttribute<FontStyle>(StyleAttributeType::FontStyle, FontStyle::Normal) ;
        else if ( val == "oblique" )
            setAttribute<FontStyle>(StyleAttributeType::FontStyle, FontStyle::Oblique) ;
        else if ( val == "italic" )
            setAttribute<FontStyle>(StyleAttributeType::FontStyle, FontStyle::Italic) ;
    }
    else if ( name == "font-variant" ) {
        if ( val == "normal" )
            setAttribute<FontVariant>(StyleAttributeType::FontVariant, FontVariant::Normal) ;
        else if ( val == "small-caps" )
            setAttribute<FontVariant>(StyleAttributeType::FontVariant, FontVariant::SmallCaps) ;
    }
    else if ( name == "font-weight" ) {
        if ( val == "normal" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::Normal) ;
        else if ( val == "bold" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::Bold) ;
        else if ( val == "bolder" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::Bolder) ;
        else if ( val == "lighter" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::Lighter) ;
        else if ( val == "100" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::W100) ;
        else if ( val == "200" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::W200) ;
        else if ( val == "300" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::W300) ;
        else if ( val == "400" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::W400) ;
        else if ( val == "500" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::W500) ;
        else if ( val == "600" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::W600) ;
        else if ( val == "700" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::W700) ;
        else if ( val == "800" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::W800) ;
        else if ( val == "900" )
            setAttribute<FontWeight>(StyleAttributeType::FontWeight, FontWeight::W900) ;
    }
    else if ( name == "font-stretch" ) {
        if ( val == "ultra-condensed" )
            setAttribute<FontStretch>(StyleAttributeType::FontStretch, FontStretch::UltraCondensed) ;
        else if ( val == "extra-condensed" )
            setAttribute<FontStretch>(StyleAttributeType::FontStretch, FontStretch::ExtraCondensed) ;
        else if ( val == "condensed" )
            setAttribute<FontStretch>(StyleAttributeType::FontStretch, FontStretch::Condensed) ;
        else if ( val == "narrower" )
            setAttribute<FontStretch>(StyleAttributeType::FontStretch, FontStretch::Narrower) ;
        else if ( val == "semi-condensed" )
            setAttribute<FontStretch>(StyleAttributeType::FontStretch, FontStretch::SemiCondensed) ;
        else if ( val == "semi-expanded" )
            setAttribute<FontStretch>(StyleAttributeType::FontStretch, FontStretch::SemiExpanded) ;
        else if ( val == "expanded" )
            setAttribute<FontStretch>(StyleAttributeType::FontStretch, FontStretch::Expanded) ;
        else if ( val == "wider" )
            setAttribute<FontStretch>(StyleAttributeType::FontStretch, FontStretch::Wider) ;
        else if ( val == "extra-expanded" )
            setAttribute<FontStretch>(StyleAttributeType::FontStretch, FontStretch::ExtraExpanded) ;
        else if ( val == "ultra-expanded" )
            setAttribute<FontStretch>(StyleAttributeType::FontStretch, FontStretch::UltraExpanded) ;
    }
    else if ( name == "font-size" )  {
        if ( val == "xx-small" )
            setAttribute<FontSize>(StyleAttributeType::FontSize, FontSize{FontSizeType::XXSmall}) ;
        else if ( val == "x-small" )
            setAttribute<FontSize>(StyleAttributeType::FontSize, FontSize{FontSizeType::XSmall}) ;
        else if ( val == "small" )
            setAttribute<FontSize>(StyleAttributeType::FontSize, FontSize{FontSizeType::Small}) ;
        else if ( val == "medium" )
            setAttribute<FontSize>(StyleAttributeType::FontSize, FontSize{FontSizeType::Medium}) ;
        else if ( val == "large" )
            setAttribute<FontSize>(StyleAttributeType::FontSize, FontSize{FontSizeType::Large}) ;
        else if ( val == "x-large" )
            setAttribute<FontSize>(StyleAttributeType::FontSize, FontSize{FontSizeType::XLarge}) ;
        else if ( val == "xx-large" )
            setAttribute<FontSize>(StyleAttributeType::FontSize, FontSize{FontSizeType::XXLarge}) ;
        else {
            setAttribute<FontSize>(StyleAttributeType::FontSize, FontSize{Length::fromString(val)}) ;
        }
    }
    else if ( name == "text-decoration" ) {
        if ( val == "underline" )
            setAttribute<TextDecoration>(StyleAttributeType::TextDecoration, TextDecoration::Underline) ;
        else if ( val == "overline" )
            setAttribute<TextDecoration>(StyleAttributeType::TextDecoration, TextDecoration::Overline) ;
        else if ( val == "strike" )
            setAttribute<TextDecoration>(StyleAttributeType::TextDecoration, TextDecoration::Strike) ;
        else if ( val == "line-through" )
            setAttribute<TextDecoration>(StyleAttributeType::TextDecoration, TextDecoration::Overline) ;
    }
    else if ( name == "text-anchor" ) {
        if ( val == "start" )
            setAttribute<TextAnchor>(StyleAttributeType::TextAnchor, TextAnchor::Start) ;
        else if ( val == "middle" )
            setAttribute<TextAnchor>(StyleAttributeType::TextAnchor, TextAnchor::Middle) ;
        else if ( val == "end" )
            setAttribute<TextAnchor>(StyleAttributeType::TextAnchor, TextAnchor::End) ;
    }
    else if ( name == "display" ) {
        if ( val == "none" )
            setAttribute<DisplayMode>(StyleAttributeType::Display, DisplayMode::None) ;
        else if ( val == "inline" )
            setAttribute<DisplayMode>(StyleAttributeType::Display, DisplayMode::Inline) ;
    }
    else if ( name == "visibility" ) {
        if ( val == "none" )
            setAttribute<VisibilityMode>(StyleAttributeType::Visibility, VisibilityMode::None) ;
        else if ( val == "hidden" ||  val == "collapsed" )
            setAttribute<VisibilityMode>(StyleAttributeType::Visibility, VisibilityMode::Hidden) ;
    }
    else if ( name == "shape-rendering" ) {
        if ( val == "auto" || val == "default" )
            setAttribute<ShapeQuality>(StyleAttributeType::ShapeRendering, ShapeQuality::Auto) ;
        else if ( val == "optimizeSpeed" )
            setAttribute<ShapeQuality>(StyleAttributeType::ShapeRendering, ShapeQuality::OptimizeSpeed) ;
        else if ( val == "crispEdges" )
            setAttribute<ShapeQuality>(StyleAttributeType::ShapeRendering, ShapeQuality::CrispEdges) ;
        else if ( val == "geometricPrecision" )
            setAttribute<ShapeQuality>(StyleAttributeType::ShapeRendering, ShapeQuality::GeometricPrecision) ;
    }
    else if ( name == "text-rendering" )  {
        if ( val == "auto" || val == "default" )
            setAttribute<TextQuality>(StyleAttributeType::TextRendering, TextQuality::Auto) ;
        else if ( val == "optimizeSpeed" )
            setAttribute<TextQuality>(StyleAttributeType::TextRendering, TextQuality::OptimizeSpeed) ;
        else if ( val == "optimizeLegibility" )
            setAttribute<TextQuality>(StyleAttributeType::TextRendering, TextQuality::OptimizeLegibility) ;
        else if ( val == "geometricPrecision" )
            setAttribute<TextQuality>(StyleAttributeType::TextRendering, TextQuality::GeometricPrecision) ;
    }
    else if ( name == "stop-opacity" )
    {
        float v ;
        if ( parse_number(val, v) )
            setAttribute<float>(StyleAttributeType::StopOpacity, v) ;

    }
    else if ( name == "stop-color" ) {
        try {
            CSSColor clr(val) ;
            setAttribute<CSSColor>(StyleAttributeType::StopColor, clr) ;
        }
        catch ( CSSColorParseException & ) {
            throw SVGDOMAttributeValueException("invalid color string for \"stop-color\"") ;
        }
    }
    else if ( name == "overflow" ) {
        if ( val == "visible" )
            setAttribute<OverflowType>(StyleAttributeType::Overflow, OverflowType::Visible) ;
        else if ( val == "auto" )
            setAttribute<OverflowType>(StyleAttributeType::Overflow, OverflowType::Auto) ;
        if ( val == "hidden" )
            setAttribute<OverflowType>(StyleAttributeType::Overflow, OverflowType::Hidden) ;
        else if ( val == "scroll" )
            setAttribute<OverflowType>(StyleAttributeType::Overflow, OverflowType::Scroll) ;
    }
    else if ( name == "clip-path" ) {
        string id = parse_uri(val) ;

        if ( !id.empty() )
            setAttribute<string>(StyleAttributeType::ClipPath, id) ;
        else
            throw SVGDOMAttributeValueException("invalid url for \"clip-path\"") ;
    }



}

void Style::fromStyleString(const string &str, Element *c) {
    static regex sr("([a-zA-Z-]+)[\\s]*:[\\s]*([^:;]+)[\\s]*[;]?") ;

    sregex_iterator it(str.begin(), str.end(), sr), end ;

    while ( it != end )  {
        parseNameValue(it->str(1), it->str(2), c) ;
        ++it ;
    }
}




} // namespace svg
} // namespace xg
