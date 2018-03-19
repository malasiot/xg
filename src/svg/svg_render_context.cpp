#include "svg_render_context.hpp"
#include <xg/text_layout.hpp>

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
        return l.value() * font_sizes_.back() ;
    case LengthUnitType::EXS:
        return l.value() * font_sizes_.back() * 0.5;
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


void RenderingContext::pushState(const Style &st)
{
    // inherit last state
    if ( states_.empty() ) states_.push_back(Style()) ;
    else states_.push_back(states_.back()) ;

    Style &style = states_.back() ;

    style.extend(st) ;
}


void RenderingContext::popState() {
    states_.pop_back() ;
}

void RenderingContext::pushTransform(const Matrix2d &trs) {
    if ( transforms_.empty() )
        transforms_.push_back(trs) ;
    else {
        Matrix2d c(transforms_.back()) ;
        c.premult(trs) ;
        transforms_.emplace_back(c) ;
    }
}

void RenderingContext::popTransform() {
    transforms_.pop_back() ;
}

void RenderingContext::clip(Element &c, const Style &st) {

    string cp_id = st.getClipPath().id() ;
    if ( !cp_id.empty() ) {
        Element *p = c.document().resolve(cp_id) ;

        if ( p ) {
            auto e = dynamic_cast<ClipPathElement *>(p) ;
            if ( e ) applyClipPath(e) ;
            ;
        }
    }

}

void RenderingContext::preRenderShape(Element &e, const Style &s, const Matrix2d &t, const Rectangle2d &bounds)
{
    pushState(s) ;
    pushTransform(t) ;

    canvas_.save() ;
    canvas_.setTransform(t) ;

    obbox_ = bounds ;

    clip(e, s) ;
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
    Length x1 = e.x1_inherited(), y1 = e.y1_inherited(), x2 = e.x2_inherited(), y2 = e.y2_inherited() ;

    GradientSpreadMethod sm = e.spreadMethodInherited() ;
    GradientUnits gu = e.gradientUnitsInherited() ;
    Matrix2d gtrans = e.gradientTransformInherited() ;

    double ix1, iy1, ix2, iy2 ;

    ix1 = ( gu == GradientUnits::ObjectBoundingBox ) ?
                x1.value() : toPixels(x1, LengthDirection::Horizontal) ;

    iy1 = ( gu == GradientUnits::ObjectBoundingBox ) ?
                y1.value() : toPixels(y1, LengthDirection::Vertical) ;

    iy2 = ( gu == GradientUnits::ObjectBoundingBox ) ?
                y2.value() : toPixels(y2, LengthDirection::Vertical) ;

    ix2 = ( gu == GradientUnits::ObjectBoundingBox ) ?
                x2.value() : toPixels(x2, LengthDirection::Horizontal) ;


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
        const CSSColor &stop_clr = p->style().getStopColor() ;
        float stop_opacity =  p->style().getStopOpacity() ;
        float offset = p->offset() ;

        brush.addStop(offset, Color(stop_clr, stop_opacity * a * gopac)) ;
    }

    canvas_.setBrush(brush) ;
}

void RenderingContext::setRadialGradientBrush(RadialGradientElement &e, float a)
{
    double ifx, ify, icx, icy, ir ;

    Length cx = e.cx_inherited(), cy = e.cy_inherited(), fx = e.fx_inherited(), fy = e.fy_inherited(), r = e.r_inherited() ;

    GradientSpreadMethod sm = e.spreadMethodInherited() ;
    GradientUnits gu = e.gradientUnitsInherited() ;
    Matrix2d gtrans = e.gradientTransformInherited() ;


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
        const CSSColor &stop_clr = p->style().getStopColor() ;
        float stop_opacity =  p->style().getStopOpacity() ;
        float offset = p->offset() ;

        brush.addStop(offset, Color(stop_clr, stop_opacity * a * gopac)) ;
    }

    canvas_.setBrush(brush) ;

}

void RenderingContext::setPatternBrush(PatternElement &e, float a)
{
    Length px = e.xInherited(), py = e.yInherited(), pw = e.widthInherited(), ph = e.heightInherited() ;
    PatternUnits pu = e.patternUnitsInherited(), pcu = e.patternContentUnitsInherited() ;
    Matrix2d trans = e.patternTransformInherited() ;

    double tile_w = ( pu == PatternUnits::ObjectBoundingBox ) ? pw.value() * obbox_.width() : toPixels(pw, LengthDirection::Horizontal) ;
    double tile_h = ( pu == PatternUnits::ObjectBoundingBox ) ? ph.value() * obbox_.height(): toPixels(ph, LengthDirection::Vertical) ;
    double tile_x = ( pu == PatternUnits::ObjectBoundingBox ) ? px.value() * obbox_.x() : toPixels(px, LengthDirection::Horizontal) ;
    double tile_y = ( pu == PatternUnits::ObjectBoundingBox ) ? py.value() * obbox_.y() : toPixels(py, LengthDirection::Vertical) ;

    Matrix2d tile_transform, pattern_transform ;

    if ( e.viewBoxIsSet() ) {
        ViewBox vbox = e.viewBoxInherited() ;
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

    Paint stroke_paint = st.getStrokePaint() ;

    if ( stroke_paint.type() == PaintType::None ) ;
    else if ( stroke_paint.type() == PaintType::SolidColor ) {
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

        const auto &dash_array = st.getDashArray().values() ;

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
        float opacity = st.getOpacity() * opacity_.back();
        const CSSColor &css_clr = stroke_paint.color() ;
        Color clr(css_clr, stroke_opacity) ;

        pen.setColor(clr) ;

        canvas_.setPen(pen) ;
    }

    Paint fill_paint = st.getFillPaint() ;

    if ( fill_paint.type() == PaintType::None ) ;
    else if ( fill_paint.type() == PaintType::SolidColor ) {
        float fill_opacity = st.getFillOpacity() ;
        float opacity = st.getOpacity() * opacity_.back() ;
        const CSSColor &css_clr = fill_paint.color() ;
        Color clr(css_clr, fill_opacity) ;


        FillRule fill_rule = st.getFillRule() ;

        SolidBrush brush(clr) ;

        if ( fill_rule == FillRule::NonZero )
            brush.setFillRule(xg::FillRule::NonZero) ;
        else
            brush.setFillRule(xg::FillRule::EvenOdd) ;

        canvas_.setBrush(brush) ;


    }
    else if ( fill_paint.type() == PaintType::PaintServer ) {

        string ps_id = fill_paint.serverId() ;
        Element *elem = e.document().resolve(ps_id) ;

        float fill_opacity = st.getFillOpacity() * opacity_.back() ;

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

#if 0

class ClipCanvas: public Canvas {
public:
    ClipCanvas(Canvas &src): Canvas(src.width(), src.height(), src.dpiX(), src.dpiY()) {
            surf_ = cairo_surface_create_similar(src.surf_,  CAIRO_CONTENT_COLOR, src.width(), src.height()) ;
            cr_ = cairo_create(surf_) ;
        }

    void saveImage(const std::string &p) {
        cairo_surface_write_to_png(surf_, p.c_str()) ;
    }

};



void RenderingContext::applyClipPath(ClipPathElement *cp)
{
    std::shared_ptr<ClipCanvas> clip_canvas(new ClipCanvas(canvas_)) ;
//    clip_canvas.save() ;
//    clip_canvas.setBrush(SolidBrush(NamedColor::black())) ;
//    clip_canvas.drawRect(0, 0, canvas_.width(), canvas_.height()) ;
 //   clip_canvas.restore() ;


    RenderingContext clipCtx(*clip_canvas) ;
    clipCtx.obbox_ = obbox_ ;

    clipCtx.clip(*cp, cp->style()) ;

    Style cst ;
    cst.setFillPaint(FillPaint(NamedColor::white())) ;

    clipCtx.pushState(cp->style()) ;
    clipCtx.pushState(cst) ;

    Matrix2d trs ;

    if ( cp->clipPathUnits() == ClipPathUnits::ObjectBoundingBox ) {

        trs.scale(obbox_.width(), obbox_.height()) ;
        trs.translate(obbox_.x(), obbox_.y()) ;
        clipCtx.pushTransform(trs) ;
    }

   // clipCtx.pushTransform(cp->trans()) ;

    clip_canvas->save() ;
    clip_canvas->setTransform(trs);
    for( auto c: cp->children() ) {
        clipCtx.clip(c.get()) ;
    }

    clip_canvas->restore() ;


    xg::FillRule fr ;

    if ( cp->style().getClipRule() == ClipRule::EvenOdd )
        fr = xg::FillRule::EvenOdd ;
    else
        fr = xg::FillRule::NonZero ;

    canvas_.setMask(clip_canvas) ;


    //canvas_.setClipPath(clipCtx.clip_path_, fr) ;
}
#endif

void RenderingContext::applyClipPath(ClipPathElement *cp)
{
    ImageCanvas clip_canvas(canvas_.width(), canvas_.height()) ;

    RenderingContext clipCtx(clip_canvas, RenderingMode::Cliping) ;
    clipCtx.obbox_ = obbox_ ;

    clipCtx.clip(*cp, cp->style()) ;

    clipCtx.pushState(cp->style()) ;

    if ( cp->clipPathUnits() == ClipPathUnits::ObjectBoundingBox ) {
        Matrix2d trs ;
        trs.scale(obbox_.width(), obbox_.height()) ;
        trs.translate(obbox_.x(), obbox_.y()) ;
        clipCtx.pushTransform(trs) ;
    }

    clipCtx.pushTransform(cp->trans()) ;

    for( auto c: cp->children() ) {
        clipCtx.clip(c.get()) ;
    }

    xg::FillRule fr ;

    if ( cp->style().getClipRule() == ClipRule::EvenOdd )
        fr = xg::FillRule::EvenOdd ;
    else
        fr = xg::FillRule::NonZero ;

    canvas_.setClipPath(clipCtx.clip_path_, fr) ;
}

void RenderingContext::render(LineElement &e)
{
    double x1 = toPixels(e.x1(), LengthDirection::Horizontal) ;
    double y1 = toPixels(e.y1(), LengthDirection::Vertical) ;
    double x2 = toPixels(e.x2(), LengthDirection::Horizontal) ;
    double y2 = toPixels(e.y2(), LengthDirection::Vertical) ;

    if  ( rendering_mode_ == RenderingMode::Display ) {
        preRenderShape(e, e.style(), e.trans(), Rectangle2d()) ;
        setPaint(e) ;
        canvas_.drawLine(x1, y1, x2, y2) ;
        postRenderShape() ;
    }

}

void RenderingContext::render(PolygonElement &e)
{
    const auto &pts = e.points().points();
    if ( pts.size() == 0 ) return ;

    Path p ;
    p.addPolygon(pts) ;

    if  ( rendering_mode_ == RenderingMode::Display ) {
        preRenderShape(e, e.style(), e.trans(), p.extents()) ;
        setPaint(e) ;
        canvas_.drawPath(p) ;
        postRenderShape() ;
    } else {
        pushTransform(e.trans()) ;
        addClipPath(p) ;
        popTransform() ;
    }
}

void RenderingContext::render(PolylineElement &e)
{
    const auto &pts = e.points().points();
    if ( pts.size() == 0 ) return ;

    Path p ;
    p.addPolyline(pts) ;

    if  ( rendering_mode_ == RenderingMode::Display ) {
        preRenderShape(e, e.style(), e.trans(), p.extents()) ;
        setPaint(e) ;
        canvas_.drawPath(p) ;
        postRenderShape() ;
    } else {
        pushTransform(e.trans()) ;
        addClipPath(p) ;
        popTransform() ;
    }

}

void RenderingContext::render(PathElement &e) {
    const auto &p = e.data().path() ;

    if  ( rendering_mode_ == RenderingMode::Display ) {
        preRenderShape(e, e.style(), e.trans(), p.extents()) ;
        setPaint(e) ;
        canvas_.drawPath(p) ;
        postRenderShape() ;
    } else {
        pushTransform(e.trans()) ;
        addClipPath(p) ;
        popTransform() ;
    }
}

void RenderingContext::render(RectElement &rect)
{
    double rxp = toPixels(rect.rx(), LengthDirection::Horizontal) ;
    double ryp = toPixels(rect.ry(), LengthDirection::Vertical) ;
    double xp = toPixels(rect.x(), LengthDirection::Horizontal) ;
    double yp = toPixels(rect.y(), LengthDirection::Vertical) ;
    double wp = toPixels(rect.width(), LengthDirection::Horizontal) ;
    double hp = toPixels(rect.height(), LengthDirection::Vertical) ;

    if (rxp > fabs (wp / 2.)) rxp = fabs (wp / 2.);
    if (ryp > fabs (hp / 2.)) ryp = fabs (hp / 2.);

    if (rxp == 0) rxp = ryp;
    else if (ryp == 0) ryp = rxp ;

    if  ( rendering_mode_ == RenderingMode::Display ) {
        preRenderShape(rect, rect.style(), rect.trans(), Rectangle2d(xp, yp, wp, hp)) ;
        setPaint(rect) ;
        canvas_.drawPath(Path().addRoundedRect(xp, yp, wp, hp, rxp, ryp)) ;
        postRenderShape() ;
    } else {
        pushTransform(rect.trans()) ;
        addClipPath(Path().addRoundedRect(xp, yp, wp, hp, rxp, ryp)) ;
        popTransform() ;
    }
}

void RenderingContext::render(EllipseElement &e)
{
    double cx = toPixels(e.cx(), LengthDirection::Horizontal) ;
    double cy = toPixels(e.cy(), LengthDirection::Vertical) ;
    double rx = toPixels(e.rx(), LengthDirection::Horizontal) ;
    double ry = toPixels(e.ry(), LengthDirection::Vertical) ;

    if  ( rendering_mode_ == RenderingMode::Display ) {
        preRenderShape(e, e.style(), e.trans(), Rectangle2d(cx-rx, cy-ry, 2*rx, 2*ry)) ;
        setPaint(e) ;
        canvas_.drawEllipse(cx, cy, rx, ry) ;
        postRenderShape() ;
    } else {
        pushTransform(e.trans()) ;
        addClipPath(Path().addEllipse(cx, cy, rx, ry)) ;
        popTransform() ;
    }

}

void RenderingContext::render(CircleElement &e)
{
    double cx = toPixels(e.cx(), LengthDirection::Horizontal) ;
    double cy = toPixels(e.cy(), LengthDirection::Vertical) ;
    double r = toPixels(e.r(), LengthDirection::Absolute) ;

    if  ( rendering_mode_ == RenderingMode::Display ) {
        preRenderShape(e, e.style(), e.trans(), Rectangle2d(cx-r, cy-r, 2*r, 2*r)) ;
        setPaint(e) ;
        canvas_.drawCircle(cx, cy, r) ;
        postRenderShape() ;
    } else {
        pushTransform(e.trans()) ;
        addClipPath(Path().addEllipse(cx, cy, r, r)) ;
        popTransform() ;
    }
}

void RenderingContext::render(ImageElement &e) {

    if ( e.uri().empty() ) return ;

    pushState(e.style()) ;
    pushTransform(e.trans()) ;

    canvas_.save() ;
    canvas_.setTransform(e.trans());

    Style &st = states_.back() ;

    float ix = toPixels(e.x(), LengthDirection::Horizontal) ;
    float iy = toPixels(e.y(), LengthDirection::Vertical) ;
    float iw = toPixels(e.width(), LengthDirection::Horizontal) ;
    float ih = toPixels(e.height(), LengthDirection::Vertical) ;

    if ( iw == 0.0 || ih == 0 ) return ;

    float opc = st.getOpacity() ;

    Image im = e.document().loadImageResource(e.uri(), &e);
    if ( !im.pixels() ) return ;

    double width = im.width() ;
    double height = im.height() ;

    Matrix2d trs = e.preserveAspectRatio().getViewBoxTransform(iw, ih, width, height, 0, 0 ) ;

    trs.translate(ix, iy) ;

    canvas_.save() ;
    canvas_.setTransform(trs) ;
    canvas_.drawImage(im, opc) ;
    canvas_.restore() ;

    canvas_.restore() ;

    popTransform() ;
    popState() ;

}

Font RenderingContext::makeFont(const Style &st)
{
    string f_family = st.getFontFamily() ;
    FontStyle f_style = st.getFontStyle() ;
    FontVariant f_variant = st.getFontVariant() ;
    FontWeight f_weight = st.getFontWeight() ;

    FontSize f_size = st.getFontSize() ;
    double f_size_px, f_size_parent = font_sizes_.back(), f_size_prefered = 12 ;

    switch ( f_size.type_ ) {
    case FontSizeType::Larger:
        f_size_px = f_size_parent * 1.2 ;
        break ;
    case FontSizeType::Smaller:
        f_size_px = f_size_parent / 1.2 ;
        break ;
    case FontSizeType::Medium:
        f_size_px = f_size_prefered  ;
        break ;
    case FontSizeType::Large:
        f_size_px = f_size_prefered * 1.2  ;
        break ;
    case FontSizeType::XLarge:
        f_size_px = f_size_prefered * 1.5 ;
        break ;
    case FontSizeType::XXLarge:
        f_size_px = f_size_prefered * 2 ;
        break ;
    case FontSizeType::Small:
        f_size_px = f_size_prefered / 1.2 ;
        break ;
    case FontSizeType::XSmall:
        f_size_px = f_size_prefered / 1.5 ;
        break ;
    case FontSizeType::XXSmall:
        f_size_px = f_size_prefered  / 2 ;
        break ;
    case FontSizeType::Length:
        if ( f_size.val_.units() == LengthUnitType::Percentage )
            f_size_px = f_size.val_.value() * f_size_parent ;
        else
            f_size_px = toPixels(f_size.val_, LengthDirection::Vertical) ;
        break ;
    }


    Font f(f_family, f_size_px) ;

    switch ( f_style )
    {
    case FontStyle::Oblique:
        f.setStyle(xg::FontStyle::Oblique) ;
        break ;
    case FontStyle::Italic:
        f.setStyle(xg::FontStyle::Italic) ;
        break ;
    case FontStyle::Normal:
        f.setStyle(xg::FontStyle::Normal) ;
        break ;
    }

    switch ( f_weight )
    {
    case FontWeight::Normal:
    case FontWeight::W100:
    case FontWeight::W200:
    case FontWeight::W300:
    case FontWeight::W400:
        f.setWeight(xg::FontWeight::Normal) ;
        break ;
    case FontWeight::Bold:
    case FontWeight::Bolder:
    case FontWeight::W500:
    case FontWeight::W600:
    case FontWeight::W700:
    case FontWeight::W800:
    case FontWeight::W900:
        f.setWeight(xg::FontWeight::Bold) ;
        break ;
    }

    return f ;
}

void RenderingContext::render(TextElement &e)
{
    pushState(e.style()) ;
    pushTransform(e.trans()) ;

    canvas_.save() ;
    canvas_.setTransform(e.trans());

    double x, y, dx, dy ;

    x = toPixels(e.x(), LengthDirection::Horizontal) ;
    y = toPixels(e.y(), LengthDirection::Vertical) ;

    dx = toPixels(e.dx(), LengthDirection::Horizontal) ;
    dy = toPixels(e.dy(), LengthDirection::Vertical) ;

    cursor_x_ = x + dx ;
    cursor_y_ = y + dy ;

    for( auto &c: e.children() ) {
        if ( auto p = std::dynamic_pointer_cast<TSpanElement>(c) ) {
            render(*p) ;
        }
        else if ( auto p = std::dynamic_pointer_cast<TRefElement>(c) ) {
            render(*p) ;
        }
    }

    canvas_.restore() ;
    popTransform() ;
    popState() ;
}

void RenderingContext::render(TSpanElement &e)
{
    if ( e.text_.empty() ) return ;

    pushState(e.style()) ;

    Style &st = states_.back() ;

    double x, y, dx, dy ;

    x = ( e.xIsSet() ) ? toPixels(e.x(), LengthDirection::Horizontal) : cursor_x_ ;
    y = ( e.yIsSet() ) ? toPixels(e.y(), LengthDirection::Vertical) : cursor_y_ ;

    dx = toPixels(e.dx(), LengthDirection::Horizontal) ;
    dy = toPixels(e.dy(), LengthDirection::Vertical) ;

    Font f = makeFont(st) ;

    font_sizes_.push_back(f.size()) ;

    // we add a space here ?

    TextLayout tl(e.text_ + ' ', f) ;
    tl.compute() ;

    const auto &line = tl.lines()[0] ;


    double ofx = 0, ofy = 0 ;

    if ( st.getTextAnchor() == TextAnchor::Middle )
        ofx = -line.width()/2 ;
    else if ( st.getTextAnchor() == TextAnchor::End )
        ofx = -line.width() ;


    vector<Point2d> gpos ;

    double gx = 0 ;
    for ( const Glyph &g: line.glyphs() ) {
        gpos.emplace_back(gx + g.x_offset_, -g.y_offset_) ;
        gx +=  g.x_advance_;
    }

    obbox_ = Rectangle2d(x + dx, y + dy, line.width(), line.height()) ;

    cursor_x_ += gx + dx ;
    cursor_y_ += dy ;

    if ( rendering_mode_ == RenderingMode::Display ) {
        canvas_.save() ;
        canvas_.setFont(f) ;
        preRenderShape(e, e.style(), Matrix2d::translation(x + dx + ofx, y + dy + ofy), Rectangle2d(0, 0, line.width(), line.height())) ;
        setPaint(e) ;
        canvas_.drawGlyphs(line.glyphs(), gpos) ;
        postRenderShape();
        canvas_.restore();
    }
    else {
        pushTransform(Matrix2d::translation(x + dx, y + dy)) ;
        addClipPath(Path().addGlyphs(line.glyphs(), gpos, f)) ;
        popTransform() ;
    }

    font_sizes_.pop_back() ;
    popState() ;
}

void RenderingContext::render(TRefElement &e)
{
    Element *eref = e.document().resolve(e.href().uri()) ;

    pushState(e.style()) ;

    canvas_.save() ;

    if ( auto p = dynamic_cast<TSpanElement *>(eref) ) {
        render(*p) ;
    } else if ( auto p = dynamic_cast<TextElement *>(eref)  ) {
        for( auto &c: p->children() ) {
            if ( auto q = dynamic_cast<TSpanElement *>(c.get()))
                render(*q) ;
            else if ( auto q = dynamic_cast<TRefElement *>(c.get()))
                render(*q) ;
        }
    }

    canvas_.restore() ;

    popState() ;

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
    else if ( auto p = dynamic_cast<UseElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<ImageElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<TextElement *>(e) ) render(*p) ;

}

void RenderingContext::clip(Element *e) {
    if ( auto p = dynamic_cast<RectElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<PathElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<PolygonElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<CircleElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<EllipseElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<UseElement *>(e) ) render(*p) ;
    else if ( auto p = dynamic_cast<TextElement *>(e) ) render(*p) ;
}

void RenderingContext::renderChildren(const Element &e)
{
    for( const auto c: e.children() ) {
        render(c.get()) ;
    }
}

void RenderingContext::clipChildren(const Element &e)
{
    for( const auto c: e.children() ) {
        clip(c.get()) ;
    }
}


// This implementation does not comply fully to the standard in the sense that does not compute path unions
// For most shapes just appending paths into a single path object will work correctly though
// A test case is this one: https://dev.w3.org/SVG/tools/svgweb/samples/svg-files/clippath.svg

void RenderingContext::addClipPath(const Path &p)
{
    clip_path_.addPath(p.transformed(transforms_.back())) ;
}


void RenderingContext::render(SymbolElement &e, double pw, double ph)
{
    pushState(e.style()) ;

    float xx, yy, sw, sh ;

    xx = toPixels(e.x(), LengthDirection::Horizontal) ;
    yy = toPixels(e.y(), LengthDirection::Vertical) ;

    sw = ( pw == 0 ) ? toPixels(e.width(), LengthDirection::Horizontal) : pw ;
    sh = ( ph == 0 ) ? toPixels(e.height(), LengthDirection::Vertical) : ph ;

    ViewBox vbox = e.viewBox() ;

    if ( vbox.width_ == 0 ) vbox.width_ = sw ;
    if ( vbox.height_ == 0 ) vbox.height_ = sh ;

    view_boxes_.push_back(vbox) ;

    Matrix2d trs = e.preserveAspectRatio().getViewBoxTransform(sw, sh, vbox.width_, vbox.height_, vbox.x_, vbox.y_) ;

    canvas_.save() ;
    canvas_.setTransform(trs) ;

    OverflowType ov = e.style().getOverflow() ;

    if ( ov == OverflowType::Scroll || ov == OverflowType::Hidden )
        canvas_.setClipRect(xx, yy, sw, sh) ;

    renderChildren(e);

    canvas_.restore() ;

    view_boxes_.pop_back();

    popState() ;
}

void RenderingContext::render(GroupElement &g) {
    preRenderShape(g, g.style(), g.trans(), Rectangle2d()) ;

    float opacity = g.style().getOpacity() ;
    opacity_.push_back(opacity_.back() * opacity ) ;
    renderChildren(g) ;
    opacity_.pop_back() ;
    postRenderShape() ;
}

void RenderingContext::render(UseElement &e)
{
    Element * eref  = e.document().resolve(e.href().uri()) ;
    if ( !eref ) return ;

    float xx = toPixels(e.x(), LengthDirection::Horizontal) ;
    float yy = toPixels(e.y(), LengthDirection::Vertical) ;
    float sw = toPixels(e.width(), LengthDirection::Horizontal) ;
    float sh = toPixels(e.height(), LengthDirection::Vertical) ;

    Matrix2d trc = Matrix2d::translation(xx, yy), trans = e.trans() ;

    trc.premult(trans) ;

    if ( rendering_mode_ == RenderingMode::Display ) {
        pushState(e.style()) ;
        pushTransform(trc) ;

        canvas_.save() ;
        canvas_.setTransform(trc) ;

        if ( auto symbol = dynamic_cast<SymbolElement *>(eref) )
            render(*symbol, sw, sh) ;
        else
            render(eref) ;

        canvas_.restore() ;

        popTransform() ;
        popState() ;
    } else {
        pushTransform(trc) ;
        clip(eref) ;
        popTransform() ;
    }

}

void RenderingContext::setOverflow(const Style &st, const Rectangle2d &r) {
    OverflowType ov = st.getOverflow() ;

    if ( ov == OverflowType::Scroll || ov == OverflowType::Hidden )
        canvas_.setClipRect(r) ;
}


void RenderingContext::render(SVGElement &e) {
    pushState(e.style()) ;

    float xx, yy, sw, sh ;

    xx = toPixels(e.x(), LengthDirection::Horizontal) ;
    yy = toPixels(e.y(), LengthDirection::Vertical) ;
    sw = toPixels(e.width(), LengthDirection::Horizontal) ;
    sh = toPixels(e.height(), LengthDirection::Vertical) ;

    ViewBox vbox = e.viewBox() ;

    if ( vbox.width_ == 0 ) vbox.width_ = sw ;
    if ( vbox.height_ == 0 ) vbox.height_ = sh ;

    view_boxes_.push_back(vbox) ;

    Matrix2d trs = e.preserveAspectRatio().getViewBoxTransform(sw, sh, vbox.width_, vbox.height_, vbox.x_, vbox.y_) ;

    Matrix2d trc = Matrix2d::translation(xx, yy) ;

    trs.premult(trc) ;

    view2dev_ = trs ;

    setOverflow(e.style(), Rectangle2d(xx, yy, sw, sh)) ;

    canvas_.save() ;
    canvas_.setTransform(trs) ;

    renderChildren(e);

    canvas_.restore() ;

    view_boxes_.pop_back();
}



}
}
