#include "svg_dom.hpp"
#include "svg_parse_util.hpp"
#include "svg_dom.hpp"

#include <xg/util/strings.hpp>

using namespace std ;

namespace xg {
namespace svg {

void SVGElement::parseAttributes(const xg::Dictionary &attrs) {

    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseViewBoxAttributes(attrs,  view_box_, preserve_aspect_ratio_);

    parseAttribute("width", attrs, width_) ;
    parseAttribute("height", attrs, height_) ;

    parseAttribute("x", attrs, x_) ;
    parseAttribute("y", attrs, y_) ;
}


Element *SVGElement::resolve(const std::string &uri) const {
    if ( uri.empty() ) return nullptr ;
    auto it = elements_.find(uri) ;
    if ( it != elements_.end() ) {
        Element *p = it->second ;
        return p ;
    }
    return nullptr ;
}

void Element::parseElementAttributes(const Dictionary &attrs) {

    attrs.visit("id", [&](const string &v) {
        id_ = v ;
        root().registerNamedElement(v, this) ;
    }) ;

}

void Element::parseViewBoxAttributes(const Dictionary &attrs, ViewBox &vb, PreserveAspectRatio &par) {
    parseAttribute("viewBox", attrs, vb) ;
    parseAttribute("preserveAspectRatio", attrs, par) ;
}

void PreserveAspectRatio::parse(const string &str) {

    auto tokens = split(str) ;

    if ( tokens.empty() ) return ;

    string align = tokens[0] ;

    if ( align == "none" )
        view_box_align_ = NoViewBoxAlign ;
    else if ( align == "xMinYMin" )
        view_box_align_ = XMinYMin ;
    else if ( align == "xMidYMin" )
        view_box_align_ = XMidYMin ;
    else if ( align == "xMaxYMin" )
        view_box_align_ = XMaxYMin ;
    else if ( align == "xMinYMid" )
        view_box_align_ = XMinYMid ;
    else if ( align == "xMidYMid" )
        view_box_align_ = XMidYMid ;
    else if ( align == "xMaxYMid" )
        view_box_align_ = XMaxYMid ;
    else if ( align == "xMinYMax" )
        view_box_align_ = XMinYMax ;
    else if ( align == "xMidYMax" )
        view_box_align_ = XMidYMax ;
    else if ( align == "xMaxYMax" )
        view_box_align_ = XMaxYMax ;
    else
        throw SVGDOMAttributeValueException("invalid align property") ;

    if ( tokens.size() < 2 ) return ;

    string policy = tokens[1] ;

    if ( policy == "meet" )
        view_box_policy_ = MeetViewBoxPolicy ;
    else if ( policy == "slice" )
        view_box_policy_ = SliceViewBoxPolicy ;
    else
        throw SVGDOMAttributeValueException("invalid policy") ;

}

void PreserveAspectRatio::constrainViewBox(double width, double height, ViewBox &orig)
{
    double origx = orig.x_ ;
    double origy = orig.y_ ;
    double origw = orig.width_ ;
    double origh = orig.height_ ;

    double neww, newh;

    if ( view_box_policy_ == MeetViewBoxPolicy )
    {
        neww = width ;
        newh = height ;

        if ( height * origw > width * origh )
            newh = origh * width / origw ;
        else
            neww = origw * height / origh;
    }
    else
    {
        neww = width ;
        newh = height ;

        if ( height * origw < width * origh )
            newh = origh * width / origw ;
        else
            neww = origw * height / origh;
    }

    if ( view_box_align_ == XMinYMin  || view_box_align_ == XMinYMid  || view_box_align_ == XMinYMax  ) ;
    else if ( view_box_align_ == XMidYMin  ||	view_box_align_ == XMidYMid  || view_box_align_ == XMidYMax  )
        origx -= (neww - width) / 2 ;
    else
        origx -= neww - width ;

    if ( view_box_align_ == XMinYMin || view_box_align_ == XMidYMin || view_box_align_ == XMaxYMin ) ;
    else if ( view_box_align_ == XMinYMid || view_box_align_ == XMidYMid || view_box_align_ == XMaxYMid )
        origy -= (newh - height) / 2;
    else
        origy -= newh - height ;

    origw = neww ;
    origh = newh ;

    orig.x_ = origx ;
    orig.y_ = origy ;
    orig.width_ = origw ;
    orig.height_ = origh ;
}

Matrix2d PreserveAspectRatio::getViewBoxTransform(double sw, double sh, double vwidth, double vheight, double vx, double vy)
{
    Matrix2d trs ;

    if ( vwidth != 0.0 && vheight != 0.0 )
    {
        double vboxx = vx ;
        double vboxy = vy ;
        double vboxw = vwidth ;
        double vboxh = vheight ;

        double ofx = 0, ofy = 0 ;
        double aspScaleX = 1.0 ;
        double aspScaleY = 1.0 ;

        if ( view_box_align_ != NoViewBoxAlign )
        {
            ViewBox vbox ;

            vbox.x_ = vboxx ;
            vbox.y_ = vboxy ;
            vbox.width_ = vboxw ;
            vbox.height_ = vboxh ;

            constrainViewBox(sw, sh, vbox) ;

            ofx = vbox.x_ ;
            ofy = vbox.y_ ;

            aspScaleX = vbox.width_/vwidth ;
            aspScaleY = vbox.height_/vheight ;
        }
        else {
            aspScaleX = sw/vboxw ;
            aspScaleY = sh/vboxh ;
        }

        trs.translate(-vx, -vy) ;
        trs.scale(aspScaleX, aspScaleY) ;
        trs.translate(ofx, ofy) ;
    }

    return trs ;

}

void Element::parseStyleAttributes(const Dictionary &p, Style &style) {
    for( const auto &lp: p ) {
        string key = lp.first, val = lp.second ;

        if ( key == "style" )
            style.fromStyleString(val, this) ;
        else
            style.parseNameValue(key, val, this) ;
    }

}

void ViewBox::parse(const string &str) {
    if ( !parse_coordinate_list(str, x_, y_, width_, height_) )
        throw SVGDOMAttributeValueException() ;
    if ( width_ < 0 || height_ < 0 )
        throw SVGDOMAttributeValueException("negative dimensions not allowed") ;
}

void Element::parseTransformAttribute(const Dictionary &attrs, Matrix2d &t) {
    attrs.visit("transform", [&](const string &v){
        if ( !parse_transform(v, t) )
            throw SVGDOMAttributeValueException("invalid transform");
    }) ;
}


#define INHERIT_ATTRIBUTE(className, attrName, attrType)\
    if ( attrName.hasValue() ) return attrName.value() ;\
    Element *p = root().resolve(href_) ;\
    className *q = nullptr ;\
    \
    while (p) {\
        q = dynamic_cast<className *>(p) ;\
        if ( !q ) break ;\
        if ( q->attrName.hasValue() ) \
            return q->attrName.value() ;\
     \
        p = root().resolve(q->href_) ;\
    }\
    \
    return attrName.value() ;\

void GradientElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;

    for( const auto &lp: attrs ) {
        string key = lp.first, val = lp.second ;

        if ( key == "gradientUnits" ) {
            if ( val == "userSpaceOnUse" )
                gradient_units_.assign(GradientUnits::UserSpaceOnUse) ;
            else if ( val == "objectBoundingBox" )
                gradient_units_.assign(GradientUnits::ObjectBoundingBox) ;
        }
        else if ( key == "gradientTransform" ) {
            Matrix2d t ;
            if ( !parse_transform(val, t) )
                throw SVGDOMAttributeValueException("invalid transform string") ;
            trans_.assign(t) ;
        }
        else if ( key == "spreadMethod" ) {
            if ( val == "pad" )
                spread_method_.assign(GradientSpreadMethod::Pad) ;
            else if ( val == "repeat" )
                spread_method_.assign(GradientSpreadMethod::Repeat) ;
            else if ( val == "reflect" )
                spread_method_.assign(GradientSpreadMethod::Reflect) ;
        }
        else if ( key == "xlink:href" ) {
            href_ = val ;
        }
    }
}

GradientSpreadMethod GradientElement::spreadMethod() {
    INHERIT_ATTRIBUTE(GradientElement, spread_method_, GradientSpreadMethod) ;
}

GradientUnits GradientElement::gradientUnits() {
    INHERIT_ATTRIBUTE(GradientElement, gradient_units_, GradientUnits) ;
}


Matrix2d GradientElement::gradientTransform() {
    INHERIT_ATTRIBUTE(GradientElement, trans_, Matrix2d) ;
}

void GradientElement::collectStops(std::vector<StopElement *> &stops)
{
    vector<StopElement *> this_element_stops ;
    for( const auto &c: children() ) {
        auto p = dynamic_cast<StopElement *>(c.get()) ;
        if ( p )
            this_element_stops.emplace_back(p) ;
    }

    if ( !this_element_stops.empty() ) {
        std::swap(this_element_stops, stops) ;
        return ;
    }

    // no stops found we need to look on referenced elements

    Element *p = root().resolve(href_) ;
    GradientElement *q = nullptr ;

    if (p) {
        q = dynamic_cast<GradientElement *>(p) ;
        if ( !q ) return ;

        q->collectStops(stops) ;
    }

}

void LinearGradientElement::parseAttributes(const Dictionary &attrs)
{
    GradientElement::parseAttributes(attrs) ;

    parseOptionalAttribute("x1", attrs, x1_) ;
    parseOptionalAttribute("x2", attrs, x2_) ;
    parseOptionalAttribute("y1", attrs, y1_) ;
    parseOptionalAttribute("y2", attrs, y2_) ;
}


Length LinearGradientElement::x1() {
    INHERIT_ATTRIBUTE(LinearGradientElement,  x1_, Length)
}

Length LinearGradientElement::y1() {
    INHERIT_ATTRIBUTE(LinearGradientElement,  y1_, Length)
}

Length LinearGradientElement::x2() {
    INHERIT_ATTRIBUTE(LinearGradientElement,  x2_, Length)
}

Length LinearGradientElement::y2() {
    INHERIT_ATTRIBUTE(LinearGradientElement,  y2_, Length)
}

void RadialGradientElement::parseAttributes(const Dictionary &attrs)
{
    GradientElement::parseAttributes(attrs) ;

    parseOptionalAttribute("cx", attrs, cx_) ;
    parseOptionalAttribute("cy", attrs, cy_) ;
    parseOptionalAttribute("r", attrs, r_) ;
    parseOptionalAttribute("fx", attrs, fx_) ;
    parseOptionalAttribute("fy", attrs, fy_) ;

    if ( !fx_.hasValue() ) fx_.setDefault(cx_.value()) ;
    if ( !fy_.hasValue() ) fy_.setDefault(cy_.value()) ;
}

Length RadialGradientElement::cx() {
    INHERIT_ATTRIBUTE(RadialGradientElement,  cx_, Length)
}

Length RadialGradientElement::cy() {
    INHERIT_ATTRIBUTE(RadialGradientElement,  cy_, Length)
}

Length RadialGradientElement::fx() {
    INHERIT_ATTRIBUTE(RadialGradientElement,  fx_, Length)
}

Length RadialGradientElement::fy() {
    INHERIT_ATTRIBUTE(RadialGradientElement,  fy_, Length)
}

Length RadialGradientElement::r() {
    INHERIT_ATTRIBUTE(RadialGradientElement, r_, Length)
}

void PatternElement::collectChildren(std::vector<Element *> &child_list)
{
    vector<Element *> this_element_children ;
    for( const auto &c: children() ) {
        this_element_children.emplace_back(c.get()) ;
    }

    if ( !this_element_children.empty() ) {
        std::swap(this_element_children, child_list) ;
        return ;
    }

    // no child elements found we need to look on referenced elements

    Element *p = root().resolve(href_) ;
    PatternElement *q = nullptr ;

    if (p) {
        q = dynamic_cast<PatternElement *>(p) ;
        if ( !q ) return ;

        q->collectChildren(child_list) ;
    }

}

void PatternElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;

    parseOptionalAttribute("viewBox", attrs, view_box_) ;
    parseOptionalAttribute("preserveAspectRatio", attrs, preserve_aspect_ratio_) ;


    for( const auto &lp: attrs ) {
        string key = lp.first, val = lp.second ;

        if ( key == "patternUnits" ) {
            if ( val == "userSpaceOnUse" )
                pattern_units_.assign(PatternUnits::UserSpaceOnUse) ;
            else if ( val == "objectBoundingBox" )
                pattern_units_.assign(PatternUnits::ObjectBoundingBox) ;
        }
        else if ( key == "patternTransform" ) {
            Matrix2d t ;
            if ( !parse_transform(val, t) )
                throw SVGDOMAttributeValueException("invalid transform string") ;
            trans_.assign(t) ;
        }
        else if ( key == "patternContentUnits" ) {
            if ( val == "userSpaceOnUse" )
                pattern_content_units_.assign(PatternUnits::UserSpaceOnUse) ;
            else if ( val == "objectBoundingBox" )
                pattern_content_units_.assign(PatternUnits::ObjectBoundingBox) ;
        }
        else if ( val == "xlink:href" ) {
            href_ = val ;
        }
    }

    parseOptionalAttribute("x", attrs, x_) ;
    parseOptionalAttribute("y", attrs, y_) ;
    parseOptionalAttribute("width", attrs, width_) ;
    parseOptionalAttribute("height", attrs, height_) ;
}

PatternUnits PatternElement::patternUnits() {
    INHERIT_ATTRIBUTE(PatternElement,  pattern_units_, PatternUnits)
}

PatternUnits PatternElement::patternContentUnits() {
    INHERIT_ATTRIBUTE(PatternElement,  pattern_content_units_, PatternUnits)
}

Matrix2d PatternElement::patternTransform() {
    INHERIT_ATTRIBUTE(PatternElement,  trans_, Matrix2d)
}

Length PatternElement::x() {
    INHERIT_ATTRIBUTE(PatternElement,  x_, Length)
}

Length PatternElement::y() {
    INHERIT_ATTRIBUTE(PatternElement,  y_, Length)
}

Length PatternElement::width() {
    INHERIT_ATTRIBUTE(PatternElement,  width_, Length)
}

Length PatternElement::height() {
    INHERIT_ATTRIBUTE(PatternElement,  height_, Length)
}

ViewBox PatternElement::viewBox() {
    INHERIT_ATTRIBUTE(PatternElement,  view_box_, ViewBox)
}

PreserveAspectRatio PatternElement::preserveAspectRatio() {
    INHERIT_ATTRIBUTE(PatternElement,  preserve_aspect_ratio_, PreserveAspectRatio)
}

void StopElement::parseAttributes(const Dictionary &attrs) {

    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;

    string offset = attrs.get("offset") ;
    if ( !offset.empty() ) {
        Length l = Length::fromString(offset) ;
        float perc ;
        if ( l.units() == LengthUnitType::Number )
            perc = l.value() ;
        else if ( l.units() == LengthUnitType::Percentage )
            perc = l.value() / 100.0 ;
        else
            throw SVGDOMAttributeValueException("invalid stop offset value") ;

        offset_.assign(std::min<float>(std::max<float>(perc, 0.0), 1.0)) ;
    }
}


void ImageElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseTransformAttribute(attrs, trans_) ;

    parseAttribute("x", attrs, x_) ;
    parseAttribute("y", attrs, y_) ;
    parseAttribute("width", attrs, width_) ;
    parseAttribute("height", attrs, height_) ;

    parseAttribute("preserveAspectRatio", attrs, preserve_aspect_ratio_) ;
    parseAttribute("xlink::href", attrs, uri_) ;
}

void StyleElement::parseAttributes(const Dictionary &attrs) {
    parseElementAttributes(attrs) ;

    media_ = attrs.get("media") ;
    type_ = attrs.get("type") ;
    title_ = attrs.get("title") ;
}

void ClipPathElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseTransformAttribute(attrs, trans_) ;

    attrs.visit("clipPathUnits", [&](const string &val) {
        if ( val == "userSpaceOnUse" )
             clip_path_units_ = ClipPathUnits::UserSpaceOnUse ;
         else if ( val == "objectBoundingBox" )
             clip_path_units_ = ClipPathUnits::ObjectBoundingBox ;
    }) ;

}

void UseElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseTransformAttribute(attrs, trans_) ;

    parseAttribute("x", attrs, x_) ;
    parseAttribute("y", attrs, y_) ;
    parseAttribute("width", attrs, width_) ;
    parseAttribute("height", attrs, height_) ;

    href_ = attrs.get("xlink:href") ;
 }

void GroupElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseTransformAttribute(attrs, trans_) ;
}

void DefsElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseTransformAttribute(attrs, trans_) ;
}

void PathElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseTransformAttribute(attrs, trans_) ;

    parseAttribute("d", attrs, path_) ;
}

void RectElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseTransformAttribute(attrs, trans_) ;

    parseAttribute("x", attrs, x_) ;
    parseAttribute("y", attrs, y_) ;
    parseAttribute("width", attrs, width_) ;
    parseAttribute("height", attrs, height_) ;
    parseAttribute("rx", attrs, rx_) ;
    parseAttribute("rx", attrs, ry_) ;
}

void CircleElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseTransformAttribute(attrs, trans_) ;

    parseAttribute("cx", attrs, cx_) ;
    parseAttribute("cy", attrs, cy_) ;
    parseAttribute("r", attrs, r_) ;
}

void EllipseElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseTransformAttribute(attrs, trans_) ;

    parseAttribute("cx", attrs, cx_) ;
    parseAttribute("cy", attrs, cy_) ;
    parseAttribute("rx", attrs, rx_) ;
    parseAttribute("ry", attrs, ry_) ;
}

void LineElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseTransformAttribute(attrs, trans_) ;

    parseAttribute("x1", attrs, x1_) ;
    parseAttribute("y1", attrs, y1_) ;
    parseAttribute("x2", attrs, x2_) ;
    parseAttribute("y2", attrs, y2_) ;
}

void PolylineElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseTransformAttribute(attrs, trans_) ;

    parseAttribute("points", attrs, points_) ;
}

void PolygonElement::parseAttributes(const Dictionary &attrs)
{
    parseElementAttributes(attrs) ;
    parseStyleAttributes(attrs, style_) ;
    parseTransformAttribute(attrs, trans_) ;

    parseAttribute("points", attrs, points_) ;
}

void PointList::parse(const string &str)
{
    vector<float> pts ;
    if ( !parse_coordinate_list(str, pts) || pts.size() % 2 )
        throw SVGDOMAttributeValueException("invalid point list") ;
    for( uint i=0 ; i<pts.size() ; i+=2 )
        points_.emplace_back(pts[i], pts[i+1]) ;
}

void PathData::parse(const string &str) {

     bool inside_group = false, is_first = true ;
     float arg1, arg2, arg3, arg4, arg5, arg6, arg7 ;

     static regex path_rx("(?:([mMsShHvVlLcCQqQtTaA]+)([^mMsShHvVlLcCqQtTaAzZ]+))|([zZ])[\\s]*") ;

     sregex_iterator it(str.begin(), str.end(), path_rx) ;
     sregex_iterator end ;

     while ( it != end ) {
         string args ;
         char cmd ;
         vector<float> argList ;


         if ( it->str(3).empty() ) {
             cmd = it->str(1).at(0) ;
             args = it->str(2) ;

             if ( !parse_coordinate_list(args, argList) )
                 throw SVGDOMAttributeValueException("invalid path data string") ;
         }
         else
             cmd = 'z' ;

         if ( cmd == 'M' || cmd == 'm' )  {

             bool is_rel = ( cmd == 'm') ;
             inside_group = true ;

             // the first is a move
             if ( cmd == 'm' ) path_.moveToRel(argList[0], argList[1]) ;
             else path_.moveTo(argList[0], argList[1]) ;

             // the rest of coordinates are interpreted as lineto

             is_first = false ;

             for(int i=2 ; i<argList.size() ; i += 2) {
                 arg1 = argList[i] ;
                 arg2 = argList[i+1] ;

                 if ( is_rel ) path_.lineToRel(arg1, arg2) ;
                 else path_.lineTo(arg1, arg2) ;
             }
         }
         else if ( cmd == 'z' )  {
             inside_group = false ;
             path_.closePath() ;
         }
         else if ( cmd == 'l' || cmd == 'L' ) {
             bool is_rel = ( cmd == 'l' ) ;

             for(int i=0 ; i<argList.size() ; i += 2) {
                 arg1 = argList[i] ;
                 arg2 = argList[i+1] ;

                 if ( is_rel ) path_.lineToRel(arg1, arg2) ;
                 else path_.lineTo(arg1, arg2) ;
             }
         }
         else if ( cmd == 'h' || cmd == 'H' ) {
             bool is_rel = (cmd == 'h') ;

             for( int i=0 ; i<argList.size() ; i++ ) {
                 float arg = argList[i] ;

                 if ( is_rel ) path_.lineToHorzRel(arg) ;
                 else path_.lineToHorz(arg) ;
             }
         }
         else if ( cmd == 'v' || cmd == 'V' )  {
             bool is_rel = (cmd == 'v') ;

             for( int i=0 ; i<argList.size() ; i++ ) {
                 float arg = argList[i] ;

                 if ( is_rel ) path_.lineToVertRel(arg) ;
                 else path_.lineToVert(arg) ;
             }
         }
         else if ( cmd == 'c' || cmd == 'C' ) {
             bool is_rel = (cmd == 'c') ;

             for ( int i=0 ; i<argList.size() ; i+=6 ) {
                 arg1 = argList[i] ;
                 arg2 = argList[i+1] ;
                 arg3 = argList[i+2] ;
                 arg4 = argList[i+3] ;
                 arg5 = argList[i+4] ;
                 arg6 = argList[i+5] ;

                 if ( is_rel ) path_.curveToRel(arg1, arg2, arg3, arg4, arg5, arg6) ;
                 else path_.curveTo(arg1, arg2, arg3, arg4, arg5, arg6) ;
             }
         }
         else if ( cmd == 's' || cmd == 'S' )  {
             bool is_rel = (cmd == 's') ;

             for( int i=0 ; i<argList.size() ; i+=4 ) {
                 arg3 = argList[i] ;
                 arg4 = argList[i+1] ;
                 arg5 = argList[i+2] ;
                 arg6 = argList[i+3] ;

                 if ( is_rel ) path_.smoothCurveToRel(arg3, arg4, arg5, arg6) ;
                 else path_.smoothCurveTo(arg3, arg4, arg5, arg6) ;
             }
         }
         else if ( cmd == 'Q' || cmd == 'q' ) {
             bool is_rel = (cmd == 'q') ;

             for( int i=0 ; i<argList.size() ; i+=4 ) {
                 arg1 = argList[i] ;
                 arg2 = argList[i+1] ;
                 arg3 = argList[i+2] ;
                 arg4 = argList[i+3] ;

                 if ( is_rel ) path_.quadToRel(arg1, arg2, arg3, arg4) ;
                 else path_.quadTo(arg1, arg2, arg3, arg4) ;
             }
         }
         else if ( cmd == 'T' || cmd == 't' ) {
             bool is_rel = (cmd == 't') ;

             for( int i=0 ; i<argList.size() ; i+=2 ) {
                 arg3 = argList[i] ;
                 arg4 = argList[i+1] ;

                 if ( is_rel ) path_.smoothQuadToRel(arg3, arg4) ;
                 else path_.smoothQuadToRel(arg3, arg4) ;
             }
         }
         else if ( cmd == 'A' || cmd == 'a' ) {
             bool is_rel = (cmd == 'a') ;

             for ( int k=0 ; k<argList.size() ; k+=7 ) {
                 arg1 = argList[k] ;
                 arg2 = argList[k+1] ;
                 arg3 = argList[k+2] ;
                 arg4 = argList[k+3] ;
                 arg5 = argList[k+4] ;
                 arg6 = argList[k+5] ;
                 arg7 = argList[k+6] ;

                 if ( is_rel ) path_.arcToRel(arg1, arg2, arg3, arg4, arg5, arg6, arg7) ;
                 else path_.arcTo(arg1, arg2, arg3, arg4, arg5, arg6, arg7) ;
             }
         }

         ++it ;
     }

}

void TextElement::parseAttributes(const Dictionary &a)
{

}


}
}
