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

    OverflowType ov = e.style_.getOverflow() ;

    if ( ov == OverflowType::Scroll || ov == OverflowType::Hidden )
        canvas_.setClipRect(xx, yy, sw, sh) ;

    renderChildren(e);

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

void RenderingContext::preRenderShape(Element &e, const Style &s, const Matrix2d &t, const Rectangle2d &bounds)
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
            Element *p = e.root().resolve(cp_id) ;

            if ( p ) {
                auto e = dynamic_cast<ClipPathElement *>(p) ;
                if ( e ) applyClipPath(e) ;
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

    GradientSpreadMethod sm = e.spreadMethod() ;
    GradientUnits gu = e.gradientUnits() ;

    double ix1, iy1, ix2, iy2 ;

    ix1 = ( gu == GradientUnits::ObjectBoundingBox ) ?
                x1.value() : toPixels(x1, LengthDirection::Horizontal) ;

    iy1 = ( gu == GradientUnits::ObjectBoundingBox ) ?
                y1.value() : toPixels(y1, LengthDirection::Vertical) ;

    iy2 = ( gu == GradientUnits::ObjectBoundingBox ) ?
                y2.value() : toPixels(y2, LengthDirection::Vertical) ;

    ix2 = ( gu == GradientUnits::ObjectBoundingBox ) ?
                x2.value() : toPixels(x2, LengthDirection::Horizontal) ;

    Matrix2d gtrans = e.gradientTransform() ;

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

    vector<StopElement *> stop_elements ;
    e.collectStops(stop_elements);

    for( const StopElement *p: stop_elements ) {
        const CSSColor &stop_clr = p->style_.getStopColor() ;
        float stop_opacity =  p->style_.getStopOpacity() ;
        float offset = p->offset_.value() ;

        brush.addStop(offset, Color(stop_clr, stop_opacity * a * gopac)) ;
    }

    canvas_.setBrush(brush) ;
}

void RenderingContext::setRadialGradientBrush(RadialGradientElement &e, float a)
{
    double ifx, ify, icx, icy, ir ;

    Length cx = e.cx(), cy = e.cy(), fx = e.fx(), fy = e.fy(), r = e.r() ;

    GradientSpreadMethod sm = e.spreadMethod() ;
    GradientUnits gu = e.gradientUnits() ;


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

    Matrix2d gtrans = e.gradientTransform() ;

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

    vector<StopElement *> stop_elements ;
    e.collectStops(stop_elements);

    for( const StopElement *p: stop_elements ) {
        const CSSColor &stop_clr = p->style_.getStopColor() ;
        float stop_opacity =  p->style_.getStopOpacity() ;
        float offset = p->offset_.value() ;

        brush.addStop(offset, Color(stop_clr, stop_opacity * a * gopac)) ;
    }

    canvas_.setBrush(brush) ;

}

void RenderingContext::setPatternBrush(PatternElement &e, float a)
{
    Length px = e.x(), py = e.y(), pw = e.width(), ph = e.height() ;
    PatternUnits pu = e.patternUnits(), pcu = e.patternContentUnits() ;
    Matrix2d trans = e.patternTransform() ;

    double tile_w = ( pu == PatternUnits::ObjectBoundingBox ) ? pw.value() * obbox_.width() : toPixels(pw, LengthDirection::Horizontal) ;
    double tile_h = ( pu == PatternUnits::ObjectBoundingBox ) ? ph.value() * obbox_.height(): toPixels(ph, LengthDirection::Vertical) ;
    double tile_x = ( pu == PatternUnits::ObjectBoundingBox ) ? px.value() * obbox_.x() : toPixels(px, LengthDirection::Horizontal) ;
    double tile_y = ( pu == PatternUnits::ObjectBoundingBox ) ? py.value() * obbox_.y() : toPixels(py, LengthDirection::Vertical) ;

    Matrix2d tile_transform, pattern_transform ;

    if ( e.hasViewBox() ) {
        ViewBox vbox = e.viewBox() ;
        tile_transform = e.preserveAspectRatio().getViewBoxTransform(tile_w, tile_h, vbox.width_, vbox.height_, vbox.x_, vbox.y_) ;
    }
    else {

        if ( pcu == PatternUnits::ObjectBoundingBox )
            tile_transform.scale(obbox_.width(), obbox_.height());
    }

    // create a new canvas to draw pattern on

    std::shared_ptr<Canvas> pattern(new PatternCanvas(tile_w, tile_h)) ;

    // recursively render children of the element into this canvas
    RenderingContext pctx(*pattern) ;

    pattern->setTransform(tile_transform) ;

    vector<Element *> children ;
    e.collectChildren(children);
    for( auto c: children ) pctx.render(c) ;

    // Compute pattern space transformation.

    pattern_transform.translate(tile_x, tile_y);
    pattern_transform.postmult(trans);

    PatternBrush brush(pattern) ;
    brush.setTransform(pattern_transform) ;
    brush.setSpread(SpreadMethod::Repeat) ;

    float gopac = states_.back().getOpacity() ;
    brush.setFillOpacity(gopac * a);

    canvas_.setBrush(brush) ;

}

void RenderingContext::setPaint(Element &e)
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

        string ps_id = fill_paint.clr_or_server_id_.get<string>() ;
        Element *elem = e.root().resolve(ps_id) ;

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
    preRenderShape(e, e.style_, e.trans_, Rectangle2d()) ;
    setPaint(e) ;
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

    preRenderShape(rect, rect.style_, rect.trans_, Rectangle2d(xp, yp, wp, hp)) ;
    setPaint(rect) ;

    canvas_.drawPath(Path().addRoundedRect(xp, yp, wp, hp, rxp, ryp)) ;

    postRenderShape() ;
}

void RenderingContext::render(EllipseElement &e)
{
    double cx = toPixels(e.cx_, LengthDirection::Horizontal) ;
    double cy = toPixels(e.cy_, LengthDirection::Vertical) ;
    double rx = toPixels(e.rx_, LengthDirection::Horizontal) ;
    double ry = toPixels(e.ry_, LengthDirection::Vertical) ;

    preRenderShape(e, e.style_, e.trans_, Rectangle2d(cx-rx, cy-ry, 2*rx, 2*ry)) ;
    setPaint(e) ;

    canvas_.drawEllipse(cx, cy, rx, ry) ;

    postRenderShape() ;

}

void RenderingContext::render(CircleElement &e)
{
    double cx = toPixels(e.cx_, LengthDirection::Horizontal) ;
    double cy = toPixels(e.cy_, LengthDirection::Vertical) ;
    double r = toPixels(e.r_, LengthDirection::Absolute) ;

    preRenderShape(e, e.style_, e.trans_, Rectangle2d(cx-r, cy-r, 2*r, 2*r)) ;
    setPaint(e) ;
    canvas_.drawCircle(cx, cy, r) ;
    postRenderShape() ;

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

void RenderingContext::render(Element *e) {
    if ( auto p = dynamic_cast<SVGElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<RectElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<PathElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<PolygonElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<PolylineElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<CircleElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<EllipseElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<GroupElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<SymbolElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<UseElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<ImageElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<TextElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<TextSpanElement *>(e) ) render(*p) ;
}

void RenderingContext::renderChildren(const Element &e)
{
    for( const auto c: e.children() ) {
        render(c.get()) ;
    }
}

void RenderingContext::render(SymbolElement &)
{

}

void RenderingContext::render(GroupElement &g) {
     preRenderShape(g, g.style_, g.trans_, Rectangle2d()) ;
     renderChildren(g) ;
     postRenderShape() ;
}

void RenderingContext::render(UseElement &)
{

}




}
}
