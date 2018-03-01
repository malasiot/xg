#include "svg_dom.hpp"
#include "svg_parse_util.hpp"

#include <xg/util/strings.hpp>

using namespace std ;

namespace xg {
namespace svg {

void DocumentNode::parseAttributes(const xg::Dictionary &attrs) {

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

void LinearGradient::parseAttributes(const Dictionary &attrs)
{
    GradientElement::parseAttributes(attrs) ;

    parse_element_attribute("x1", attrs, x1_) ;
    parse_element_attribute("x2", attrs, x2_) ;
    parse_element_attribute("y1", attrs, y1_) ;
    parse_element_attribute("y2", attrs, y2_) ;
}

void RadialGradient::parseAttributes(const Dictionary &attrs)
{
    GradientElement::parseAttributes(attrs) ;

    parse_element_attribute("cx", attrs, cx_) ;
    parse_element_attribute("cy", attrs, cy_) ;
    parse_element_attribute("fx", attrs, fx_) ;
    parse_element_attribute("fy", attrs, fy_) ;
    parse_element_attribute("r", attrs, r_) ;
}


void Pattern::parseAttributes(const Dictionary &attrs)
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
}
}
