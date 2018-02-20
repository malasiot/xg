#include <xg/pen.hpp>

using namespace std ;

namespace xg {

Pen & Pen::setColor(const Color &brush) {
    line_color_ = brush ;
    return *this ;
}

Pen & Pen::setLineWidth(double width) {
    line_width_ = width ;
    return *this ;

}

Pen & Pen::setMiterLimit(double limit) {
    miter_limit_ = limit ;
    return *this ;
}

Pen & Pen::setLineJoin(LineJoin join) {
    line_join_ = join ;
    return *this ;
}

Pen & Pen::setLineCap(LineCap cap) {
    line_cap_ = cap ;
    return *this ;
}

Pen & Pen::setDashArray(const vector<double> &dashes) {
    dash_array_ = dashes ;
    return *this ;
}

Pen & Pen::setDashOffset(double offset) {
    dash_offset_ = offset ;
    return *this ;
}

Pen & Pen::setLineStyle(LineStyle style) {

    line_style_ = style ;

    if ( style == SolidLine ) {
        dash_array_.clear() ;
    }
    else if ( style == DashLine ) {
        dash_array_.resize(2) ;
        dash_array_[0] = 4.0 ;
        dash_array_[1] = 1.0 ;
    }
    else if ( style == DotLine ) {
        dash_array_.resize(2) ;
        dash_array_[0] = 1.0 ;
        dash_array_[1] = 1.0 ;
    }
    else if ( style == DashDotLine ) {
        dash_array_.resize(4) ;
        dash_array_[0] = 6.0 ;
        dash_array_[1] = 1.0 ;
        dash_array_[2] = 1.0 ;
        dash_array_[3] = 1.0 ;
    }
    return *this ;
}

Pen::Pen(const Color &clr, double width, LineStyle style): line_color_(clr) {
    setLineStyle(style) ;
    setLineWidth(width) ;
}


}
