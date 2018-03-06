#include "svg_render_context.hpp"

using namespace std ;

namespace xg {
namespace svg {

float RenderingContext::toPixels(const Length &l, LengthDirection dir, bool scale_to_viewport) {
    double factor = 1.0 ;

    static const double default_font_size = 12.0 ;

    switch( l.units() )  {
    case LengthUnitType::CM:
        factor = 1/2.54 ;
        break ;
    case LengthUnitType::MM:
        factor = 1/25.4 ;
        break ;
    case LengthUnitType::PT:
        factor = 1/72.0 ;
        break ;
    case LengthUnitType::PC:
        factor = 1/6.0 ;
        break ;
    }

    switch( l.units() )
    {
    case LengthUnitType::Number:
    case LengthUnitType::PX:
        return l.value() ;
    case LengthUnitType::CM:
    case LengthUnitType::MM:
    case LengthUnitType::IN:
    case LengthUnitType::PT:
    case LengthUnitType::PC:
        if ( dir == LengthDirection::Horizontal  ) return  l.value() * factor * dpi_x_ ;
        else if ( dir == LengthDirection::Vertical ) return  l.value() * factor * dpi_y_ ;
        else if ( dir == LengthDirection::Absolute ) return  l.value() * factor*sqrt(dpi_x_ * dpi_y_) ;
    case LengthUnitType::EMS:
        return l.value() * default_font_size ;
    case LengthUnitType::EXS:
        return l.value() * default_font_size * 0.5;
    case LengthUnitType::Percentage: {
        float fx = ( scale_to_viewport ) ? view_boxes_.back().width_ : obbox_.width() ;
        float fy = ( scale_to_viewport ) ? view_boxes_.back().height_ : obbox_.height() ;

        if ( dir == LengthDirection::Horizontal  ) return  l.value() *  fx ;
        else if ( dir == LengthDirection::Vertical ) return  l.value() * fy ;
        else if ( dir == LengthDirection::Absolute ) return  l.value() * sqrt(fx * fy) ;
    }
    }

    return 0 ;
}

void RenderingContext::render(SVGElement &e) {
    pushState(e.style_) ;

    float xx, yy, sw, sh ;

    xx = toPixels(e.x_, LengthDirection::Horizontal) ;
    yy = toPixels(e.y_, LengthDirection::Vertical) ;
    sw = toPixels(e.width_, LengthDirection::Horizontal) ;
    sh = toPixels(e.height_, LengthDirection::Vertical) ;

    ViewBox vbox = e.view_box_ ;

    if ( vbox.width_ == 0 ) vbox.width_ = sw ;
    if ( vbox.height_ == 0 ) vbox.height_ = sh ;

    view_boxes_.push_back(vbox) ;

    Matrix2d trs = e.preserve_aspect_ratio_.getViewBoxTransform(sw, sh, vbox.width_, vbox.height_, vbox.x_, vbox.y_) ;

    view2dev_ = trs ;

    canvas_.save() ;
    canvas_.setTransform(trs) ;

    renderChildren(&e);

    canvas_.restore() ;

    view_boxes_.pop_back();
}


void RenderingContext::pushState(const Style &st)
{
    // inherit last state
    if ( states_.empty() ) states_.push_back(Style()) ;
    else states_.push_back(states_.back()) ;

    Style &style = states_.back() ;

    //style.resetNonInheritable()

    style.extend(st) ;
}


void RenderingContext::popState() {
    states_.pop_back() ;
}

void RenderingContext::pushTransform(const Matrix2d &trs) {
    transforms_.push_back(trs) ;
}

void RenderingContext::popTransform() {
    transforms_.pop_back() ;
}

ElementPtr RenderingContext::lookupRef(const std::string &name) {
    auto it = refs_.find(name) ;

    if ( it == refs_.end() ) return nullptr ;
    else return (*it).second ;
}

void RenderingContext::populateRefs(const ElementPtr &p) {
    string id = p->id_ ;

    if ( !id.empty() ) refs_['#' + id] = p ;

    for( const auto el: p->children_ ) {
        populateRefs(el) ;
    }

}

void RenderingContext::preRenderShape(const Style &s, const Matrix2d &t, const Rectangle2d &bounds)
{
    pushState(s) ;
    pushTransform(t) ;

    Style &st = states_.back() ;

    canvas_.save() ;
    canvas_.setTransform(t) ;

    obbox_ = bounds ;

    if ( rendering_mode_ == RenderingMode::Display ) {
        string cp_id = st.getClipPath() ;
        if ( !cp_id.empty() ) {
            ElementPtr p = lookupRef(cp_id) ;

            if ( p ) {
                auto e = std::dynamic_pointer_cast<ClipPathElement>(p) ;
                if ( e ) applyClipPath(e.get()) ;
            }
        }
    }
}

void RenderingContext::setShapeAntialias (ShapeQuality aa) {
    switch ( aa ) {
    case ShapeQuality::Auto:
    case ShapeQuality::CrispEdges:
    case ShapeQuality::GeometricPrecision:
        canvas_.setAntialias(true);
        break ;
    case ShapeQuality::OptimizeSpeed:
        canvas_.setAntialias(false);
        break ;
    }
}


void RenderingContext::setLinearGradientBrush(LinearGradientElement &e, float a)
{
    Length x1 = e.x1(), y1 = e.y1(), x2 = e.x2(), y2 = e.y2() ;

    GradientSpreadMethod sm = e.spread_method_.value() ;
    GradientUnits gu = e.gradient_units_.value() ;

    double ix1, iy1, ix2, iy2 ;

    ix1 = ( gu == GradientUnits::ObjectBoundingBox ) ?
                x1.value() : toPixels(x1, LengthDirection::Horizontal) ;

    iy1 = ( gu == GradientUnits::ObjectBoundingBox ) ?
                y1.value() : toPixels(y1, LengthDirection::Vertical) ;

    iy2 = ( gu == GradientUnits::ObjectBoundingBox ) ?
                y2.value() : toPixels(y2, LengthDirection::Vertical) ;

    ix2 = ( gu == GradientUnits::ObjectBoundingBox ) ?
                x2.value() : toPixels(ctx, LengthDirection::Horizontal) ;

    Matrix2d gtrans = e.trans_.value() ;

    if ( gu == GradientUnits::ObjectBoundingBox ) {
        Matrix2d obbm ;

        obbm.scale(obbox_.width(), obbox_.height()) ;
        obbm.translate(obbox_.x(), obbox_.y()) ;

        gtrans.premult(obbm) ;
    }

    LinearGradientBrush brush(ix1, iy1, ix2, iy2) ;
    brush.setTransform(gtrans) ;

    if ( sm == GradientSpreadMethod::Reflect )
        brush.setSpread(SpreadMethod::Reflect) ;
    else if ( sm == GradientSpreadMethod::Repeat )
        brush.setSpread(SpreadMethod::Repeat) ;
    else
        brush.setSpread(SpreadMethod::Pad) ;

    float gopac = states_.back().getOpacity() ;

    for( const auto &c: e.children_ ) {
        auto p = dynamic_cast<StopElement *>(c.get()) ;
        if ( p ) {
           const CSSColor &stop_clr = p->style_.getStopColor() ;
           float stop_opacity =  p->style_.getStopOpacity() ;
           float offset = p->offset_.value() ;

           brush.addStop(offset, Color(stop_clr, stop_opacity * a * gopac)) ;
        }

    }

    canvas_.setBrush(brush) ;
}

void RenderingContext::setRadialGradientBrush(RadialGradientElement &e, float a)
{
    double ifx, ify, icx, icy, ir ;

    Length cx = e.cx_, cy = e.cy_, fx = e.fx_, fy = e.fy_, r = e.r_ ;

    GradientSpreadMethod sm = e.spread_method_.value() ;
    GradientUnits gu = e.gradient_units_.value() ;

    double ix1, iy1, ix2, iy2 ;

    icx = ( gu == GradientUnits::ObjectBoundingBox ) ?
                cx.value() : toPixels(cx, LengthDirection::Horizontal) ;

    icy = ( gu == GradientUnits::ObjectBoundingBox ) ?
                cy.value() : toPixels(cy, LengthDirection::Vertical) ;

    ifx = ( gu == GradientUnits::ObjectBoundingBox ) ?
                fx.value() : toPixels(fx, LengthDirection::Horizontal) ;

    ify = ( gu == GradientUnits::ObjectBoundingBox ) ?
                fy.value() : toPixels(fy, LengthDirection::Vertical) ;

    ir = ( gu == GradientUnits::ObjectBoundingBox ) ?
                r.value() : toPixels(r, LengthDirection::Absolute) ;

    Matrix2d gtrans = e.trans_.value() ;

    if ( gu == GradientUnits::ObjectBoundingBox ) {
        Matrix2d obbm ;

        obbm.scale(obbox_.width(), obbox_.height()) ;
        obbm.translate(obbox_.x(), obbox_.y()) ;

        gtrans.premult(obbm) ;
    }

    RadialGradientBrush brush(icx, icy, ir, ifx, ify) ;
    brush.setTransform(gtrans) ;

    if ( sm == GradientSpreadMethod::Reflect )
        brush.setSpread(SpreadMethod::Reflect) ;
    else if ( sm == GradientSpreadMethod::Repeat )
        brush.setSpread(SpreadMethod::Repeat) ;
    else
        brush.setSpread(SpreadMethod::Pad) ;

    float gopac = states_.back().getOpacity() ;

    for( const auto &c: e.children_ ) {
        auto p = dynamic_cast<StopElement *>(c.get()) ;
        if ( p ) {
           const CSSColor &stop_clr = p->style_.getStopColor() ;
           float stop_opacity =  p->style_.getStopOpacity() ;
           float offset = p->offset_.value() ;

           brush.addStop(offset, Color(stop_clr, stop_opacity * a * gopac)) ;
        }
    }

    canvas_.setBrush(brush) ;

}

void RenderingContext::setPatternBrush(PatternElement &e, float a)
{

}

void RenderingContext::setPaint()
{
    Style &st = states_.back() ;

    setShapeAntialias(st.getShapeQuality()) ;

    /*
      double x1, y1, x2, y2 ;
      cairo_path_extents(cr, &x1, &y1, &x2, &y2) ;

      ctx->extentBoundingBox(x1, y1, x2, y2) ;
*/

    Paint stroke_paint = st.getStrokePaint() ;

    if ( stroke_paint.type_ == PaintType::None ) ;
    else if ( stroke_paint.type_ == PaintType::SolidColor ) {
        Pen pen ;
        pen.setLineWidth(toPixels(st.getStrokeWidth(), LengthDirection::Absolute)) ;
        pen.setMiterLimit(st.getMiterLimit()) ;

        switch ( st.getLineCap() ) {
        case LineCapType::Butt:
            pen.setLineCap(LineCap::Butt)  ;
            break ;
        case LineCapType::Round:
            pen.setLineCap(LineCap::Round)  ;
            break ;
        case LineCapType::Square:
            pen.setLineCap(LineCap::Square)  ;
            break ;
        }

        switch ( st.getLineJoin() ) {
        case LineJoinType::Bevel:
            pen.setLineJoin(LineJoin::Bevel)  ;
            break ;
        case LineJoinType::Round:
            pen.setLineJoin(LineJoin::Round)  ;
            break ;
        case LineJoinType::Miter:
            pen.setLineJoin(LineJoin::Miter)  ;
            break ;
        }

        std::vector<Length> dash_array = st.getDashArray() ;

        if ( !dash_array.empty() ) {
            vector<double> dashes ;
            dashes.reserve(dash_array.size()) ;
            for( const auto &l: dash_array ) {
                dashes.emplace_back(toPixels(l, LengthDirection::Absolute)) ;
            }

            pen.setDashArray(dashes) ;
            pen.setDashOffset(toPixels(st.getDashOffset(), LengthDirection::Absolute)) ;
        }

        float stroke_opacity = st.getStrokeOpacity() ;
        float opacity = st.getOpacity() ;
        const CSSColor &css_clr = stroke_paint.clr_or_server_id_.get<CSSColor>() ;
        Color clr(css_clr, stroke_opacity * opacity) ;
        pen.setColor(clr) ;

        canvas_.setPen(pen) ;
    }

    Paint fill_paint = st.getFillPaint() ;

    if ( fill_paint.type_ == PaintType::None ) ;
    else if ( fill_paint.type_ == PaintType::SolidColor ) {
        float fill_opacity = st.getFillOpacity() ;
        float opacity = st.getOpacity() ;
        const CSSColor &css_clr = fill_paint.clr_or_server_id_.get<CSSColor>() ;
        Color clr(css_clr, fill_opacity * opacity) ;

        canvas_.setBrush(SolidBrush(clr)) ;
    }
    else if ( fill_paint.type_ == PaintType::PaintServer ) {
     //  ElementPtr elem = lookupRef(fill_paint.clr_or_server_id_.get<std::string>()) ;
        Element *elem = fill_paint.clr_or_server_id_.get<Element *>() ;

        float fill_opacity = st.getFillOpacity() ;

        if ( elem ) {
            if ( auto p = dynamic_cast<LinearGradientElement *>(elem) ) {
                setLinearGradientBrush(*p, fill_opacity) ;
            } else if ( auto p = dynamic_cast<RadialGradientElement *>(elem) ) {
                setRadialGradientBrush(*p, fill_opacity) ;
            } else if ( auto p = dynamic_cast<PatternElement *>(elem) ) {
                setPatternBrush(*p, fill_opacity) ;
            }
        }
    }

}

void RenderingContext::postRenderShape()
{
    canvas_.restore() ;

    popTransform() ;
    popState() ;
}

void RenderingContext::applyClipPath(ClipPathElement *e)
{

}

void RenderingContext::render(CircleElement &)
{

}

void RenderingContext::render(LineElement &)
{

}

void RenderingContext::render(PolygonElement &)
{

}

void RenderingContext::render(PolylineElement &)
{

}

void RenderingContext::render(PathElement &e) {
    preRenderShape(e.style_, e.trans_, Rectangle2d()) ;
    setPaint() ;
    canvas_.drawPath(e.path_.path_) ;
    postRenderShape() ;
}

void RenderingContext::render(RectElement &rect)
{


    double rxp = toPixels(rect.rx_, LengthDirection::Horizontal) ;
    double ryp = toPixels(rect.ry_, LengthDirection::Vertical) ;
    double xp = toPixels(rect.x_, LengthDirection::Horizontal) ;
    double yp = toPixels(rect.y_, LengthDirection::Vertical) ;
    double wp = toPixels(rect.width_, LengthDirection::Horizontal) ;
    double hp = toPixels(rect.height_, LengthDirection::Vertical) ;

    if (rxp > fabs (wp / 2.)) rxp = fabs (wp / 2.);
    if (ryp > fabs (hp / 2.)) ryp = fabs (hp / 2.);

    if (rxp == 0) rxp = ryp;
    else if (ryp == 0) ryp = rxp ;

    preRenderShape(rect.style_, rect.trans_, Rectangle2d(xp, yp, wp, hp)) ;
    setPaint() ;


    canvas_.drawPath(Path().addRoundedRect(xp, yp, wp, hp, rxp, ryp)) ;

    postRenderShape() ;
}

void RenderingContext::render(EllipseElement &)
{

}

void RenderingContext::render(ImageElement &)
{

}

void RenderingContext::render(TextElement &)
{

}

void RenderingContext::render(TextSpanElement &)
{

}

void RenderingContext::renderChildren(const Element *e)
{
    for( const auto c: e->children_ ) {
        if ( auto p = std::dynamic_pointer_cast<SVGElement>(c) ) render(*p) ;
        else if ( auto p = std::dynamic_pointer_cast<RectElement>(c) ) render(*p) ;
        else if ( auto p = std::dynamic_pointer_cast<PathElement>(c) ) render(*p) ;
        else if ( auto p = std::dynamic_pointer_cast<PolygonElement>(c) ) render(*p) ;
        else if ( auto p = std::dynamic_pointer_cast<PolylineElement>(c) ) render(*p) ;
        else if ( auto p = std::dynamic_pointer_cast<CircleElement>(c) ) render(*p) ;
        else if ( auto p = std::dynamic_pointer_cast<EllipseElement>(c) ) render(*p) ;
        else if ( auto p = std::dynamic_pointer_cast<GroupElement>(c) ) render(*p) ;
        else if ( auto p = std::dynamic_pointer_cast<SymbolElement>(c) ) render(*p) ;
        else if ( auto p = std::dynamic_pointer_cast<UseElement>(c) ) render(*p) ;
        else if ( auto p = std::dynamic_pointer_cast<ImageElement>(c) ) render(*p) ;
        else if ( auto p = std::dynamic_pointer_cast<TextElement>(c) ) render(*p) ;
        else if ( auto p = std::dynamic_pointer_cast<TextSpanElement>(c) ) render(*p) ;
    }
}

void RenderingContext::render(SymbolElement &)
{

}

void RenderingContext::render(GroupElement &g) {
     preRenderShape(g.style_, g.trans_, Rectangle2d()) ;
     renderChildren(&g) ;
     postRenderShape() ;

}

void RenderingContext::render(UseElement &)
{

}




}
}
