#include "svg_dom.hpp"
#include "svg_parse_util.hpp"

#include <xg/util/strings.hpp>

using namespace std ;

namespace xg {
namespace svg {

void SVGElement::parseAttributes(const xg::Dictionary &attrs) {

    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    FitToViewBox::parseAttributes(attrs) ;

    parse_element_attribute("width", attrs, width_) ;
    parse_element_attribute("height", attrs, height_) ;

    parse_element_attribute("x", attrs, x_) ;
    parse_element_attribute("y", attrs, y_) ;
}

void Element::parseAttributes(const Dictionary &attrs) {
    id_ = attrs.get("id") ;
}

void FitToViewBox::parseAttributes(const Dictionary &attrs) {

    parse_element_attribute("viewBox", attrs, view_box_) ;
    parse_element_attribute("preserveAspectRatio", attrs, preserve_aspect_ratio_) ;
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

}

void Stylable::parseAttributes(const Dictionary &p) {
    for( const auto &lp: p ) {
        string key = lp.first, val = lp.second ;

        if ( key == "style" )
            style_.fromStyleString(val) ;
        else
            style_.parseNameValue(key, val) ;
    }

}

void ViewBox::parse(const string &str) {
    if ( !parse_coordinate_list(str, x_, y_, width_, height_) )
        throw SVGDOMAttributeValueException() ;
    if ( width_ < 0 || height_ < 0 )
        throw SVGDOMAttributeValueException("negative dimensions not allowed") ;
}

void Transformable::parseAttributes(const Dictionary &attrs) {
    parse_element_attribute("transform", attrs, trans_) ;
}

static void parse_number_list(const char *&p, vector<float> &numbers)
{
    eat_white(p) ;
    if ( *p++ != '(' )
        throw SVGDOMAttributeValueException("invalid transform string") ;

    const char *start = p ;
    while ( *p && *p != ')' ) ++p ;

    string s(start, p) ;
    if ( !parse_coordinate_list(s, numbers) )
        throw SVGDOMAttributeValueException("invalid transform string") ;

    ++p ;

}

void Transform::parse(const string &str)
{
    const char *p = str.c_str() ;
    eat_white(p) ;

    while ( *p ) {
        vector<float> nums ;
         if ( strncmp(p, "matrix", 6) == 0 ) {
             p += 6 ;

             parse_number_list(p, nums) ;

             if ( nums.size() >= 6 ) {
                 Matrix2d m(nums[0], nums[1], nums[2], nums[3], nums[4], nums[5]) ;
                 m_.premult(m) ;
             }
         }
         else if ( strncmp(p, "translate", 9) == 0 ) {
             p += 9 ;

             parse_number_list(p, nums) ;

             if ( nums.size() >= 2 )
                 m_.translate(nums[0], nums[1]) ;
             else if ( nums.size() == 1 )
                 m_.translate(nums[0], 0.0) ;
         }
         else if ( strncmp(p, "rotate", 6) == 0 )  {
             p += 6 ;

             parse_number_list(p, nums) ;

             if ( nums.size() == 1 )
                 m_.rotate(nums[0]) ;
             else if ( nums.size() == 3 )
                 m_.rotate(nums[0], Point2d(nums[1], nums[2])) ;

         }
         else if ( strncmp(p, "scale", 5) == 0 )  {
             p += 5 ;

             parse_number_list(p, nums) ;

             if ( nums.size() == 1 )
                 m_.scale(nums[0], nums[0]) ;
             else if ( nums.size() >= 2 )
                 m_.scale(nums[0], nums[1]) ;
         }
         else if ( strncmp(p, "skewX", 5) == 0 ) {
             p += 5 ;

             parse_number_list(p, nums) ;

             if ( nums.size() >= 1 )
                 m_.skew(nums[0], 0.0) ;
         }
         else if ( strncmp(p, "skewY", 5) == 0 ) {
             p += 5 ;

             parse_number_list(p, nums) ;

             if ( nums.size() >= 1 )
                 m_.skew(0.0, nums[0]) ;
         }

         eat_white_comma(p) ;
     }
}

void GradientElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;

    for( const auto &lp: attrs ) {
        string key = lp.first, val = lp.second ;

        if ( key == "gradientUnits" ) {
            if ( val == "userSpaceOnUse" )
                gradient_units_ = GradientUnits::UserSpaceOnUse ;
            else if ( val == "objectBoundingBox" )
                gradient_units_ = GradientUnits::ObjectBoundingBox ;
        }
        else if ( key == "gradientTransform" ) {
            trans_.parse(val) ;
        }
        else if ( key == "spreadMethod" ) {
            if ( val == "pad" )
                spread_method_ = GradientSpreadMethod::Pad ;
            else if ( val == "repeat" )
                spread_method_ = GradientSpreadMethod::Repeat ;
            else if ( val == "reflect" )
                spread_method_ = GradientSpreadMethod::Reflect ;
        }
        else if ( key == "xlink:href" )
            href_ = val ;
    }
}

void LinearGradientElement::parseAttributes(const Dictionary &attrs)
{
    GradientElement::parseAttributes(attrs) ;

    parse_element_attribute("x1", attrs, x1_) ;
    parse_element_attribute("x2", attrs, x2_) ;
    parse_element_attribute("y1", attrs, y1_) ;
    parse_element_attribute("y2", attrs, y2_) ;
}

void RadialGradientElement::parseAttributes(const Dictionary &attrs)
{
    GradientElement::parseAttributes(attrs) ;

    parse_element_attribute("cx", attrs, cx_) ;
    parse_element_attribute("cy", attrs, cy_) ;
    parse_element_attribute("fx", attrs, fx_) ;
    parse_element_attribute("fy", attrs, fy_) ;
    parse_element_attribute("r", attrs, r_) ;
}


void PatternElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    FitToViewBox::parseAttributes(attrs) ;

    for( const auto &lp: attrs ) {
        string key = lp.first, val = lp.second ;

        if ( key == "patternUnits" ) {
            if ( val == "userSpaceOnUse" )
                pattern_units_ = PatternUnits::UserSpaceOnUse ;
            else if ( val == "objectBoundingBox" )
                pattern_units_ = PatternUnits::ObjectBoundingBox ;
        }
        else if ( key == "patternTransform" ) {
            trans_.parse(val) ;
        }
        else if ( key == "patternContentUnits" ) {
            if ( val == "userSpaceOnUse" )
                pattern_content_units_ = PatternUnits::UserSpaceOnUse ;
            else if ( val == "objectBoundingBox" )
                pattern_content_units_ = PatternUnits::ObjectBoundingBox ;
        }
        else if ( key == "xlink:href" )
            href_ = val ;
    }

    parse_element_attribute("x", attrs, x_) ;
    parse_element_attribute("y", attrs, y_) ;
    parse_element_attribute("width", attrs, width_) ;
    parse_element_attribute("height", attrs, height_) ;
}

void Stop::parseAttributes(const Dictionary &attrs) {

    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;

    parse_element_attribute("offset", attrs, offset_) ;
}

void ImageElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    Transformable::parseAttributes(attrs) ;

    parse_element_attribute("x", attrs, x_) ;
    parse_element_attribute("y", attrs, y_) ;
    parse_element_attribute("width", attrs, width_) ;
    parse_element_attribute("height", attrs, height_) ;

    parse_element_attribute("preserveAspectRatio", attrs, preserve_aspect_ratio_) ;
    parse_element_attribute("xlink::href", attrs, uri_) ;
}

void StyleElement::parseAttributes(const Dictionary &attrs) {
    Element::parseAttributes(attrs) ;

    media_ = attrs.get("media") ;
    type_ = attrs.get("type") ;
    title_ = attrs.get("title") ;
}

void ClipPathElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    Transformable::parseAttributes(attrs) ;

    attrs.visit("clipPathUnits", [&](const string &val) {
        if ( val == "userSpaceOnUse" )
             clip_path_units_ = ClipPathUnits::UserSpaceOnUse ;
         else if ( val == "objectBoundingBox" )
             clip_path_units_ = ClipPathUnits::ObjectBoundingBox ;
    }) ;

}

void UseElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    Transformable::parseAttributes(attrs) ;

    parse_element_attribute("x", attrs, x_) ;
    parse_element_attribute("y", attrs, y_) ;
    parse_element_attribute("width", attrs, width_) ;
    parse_element_attribute("height", attrs, height_) ;

    parse_element_attribute("xlink::href", attrs, uri_) ;
}

void GroupElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    Transformable::parseAttributes(attrs) ;
}

void DefsElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    Transformable::parseAttributes(attrs) ;
}

void PathElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    Transformable::parseAttributes(attrs) ;

    parse_element_attribute("d", attrs, path_) ;
}

void RectElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    Transformable::parseAttributes(attrs) ;

    parse_element_attribute("x", attrs, x_) ;
    parse_element_attribute("y", attrs, y_) ;
    parse_element_attribute("width", attrs, width_) ;
    parse_element_attribute("height", attrs, height_) ;
    parse_element_attribute("rx", attrs, rx_) ;
    parse_element_attribute("rx", attrs, ry_) ;
}

void CircleElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    Transformable::parseAttributes(attrs) ;

    parse_element_attribute("cx", attrs, cx_) ;
    parse_element_attribute("cy", attrs, cy_) ;
    parse_element_attribute("r", attrs, r_) ;
}

void EllipseElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    Transformable::parseAttributes(attrs) ;

    parse_element_attribute("cx", attrs, cx_) ;
    parse_element_attribute("cy", attrs, cy_) ;
    parse_element_attribute("rx", attrs, rx_) ;
    parse_element_attribute("ry", attrs, ry_) ;
}

void LineElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    Transformable::parseAttributes(attrs) ;

    parse_element_attribute("x1", attrs, x1_) ;
    parse_element_attribute("y1", attrs, y1_) ;
    parse_element_attribute("x2", attrs, x2_) ;
    parse_element_attribute("y2", attrs, y2_) ;
}

void PolylineElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    Transformable::parseAttributes(attrs) ;

    parse_element_attribute("points", attrs, points_) ;
}

void PolygonElement::parseAttributes(const Dictionary &attrs)
{
    Element::parseAttributes(attrs) ;
    Stylable::parseAttributes(attrs) ;
    Transformable::parseAttributes(attrs) ;

    parse_element_attribute("points", attrs, points_) ;
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

void Visitor::visit(Element *e) {
    if ( auto p = dynamic_cast<SVGElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<RectElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<PathElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<PolygonElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<PolylineElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<CircleElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<EllipseElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<DefsElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<GroupElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<SymbolElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<UseElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<ClipPathElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<ImageElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<TextElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<TextSpanElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<LinearGradientElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<RadialGradientElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<PatternElement *>(e) ) visit(*p) ;
    else if ( auto p = dynamic_cast<StyleElement *>(e) ) visit(*p) ;
}

void Visitor::visitChildren(Element *e)
{
    for( auto c: e->children_ ) visit(c.get()) ;
}


}
}
