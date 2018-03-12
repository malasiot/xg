#include <xg/backends/cairo/canvas.hpp>
#include <xg/canvas.hpp>

#include <cairo/cairo.h>
#include <cassert>
#include <cmath>
#include <regex>

#ifdef _WIN32
#include <cairo-win32.h>
#else
#include <cairo/cairo-ft.h>
#include <fontconfig/fcfreetype.h>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include "text_layout.hpp"
#include "font_manager.hpp"

#endif

#include <mutex>
#include <iostream>

using namespace std;

namespace xg {
namespace detail {

Backend::State::State(): font_("Arial", 10) {

}

Backend::Backend() {
    State st ;
    state_.push(std::move(st)) ;
}

void Backend::set_cairo_stroke(const Pen &pen) {
    cairo_t *cr = (cairo_t *)cr_  ;

    const Color &clr = pen.lineColor() ;

    if ( clr.a()  == 1.0 ) cairo_set_source_rgb(cr, clr.r(), clr.g(), clr.b()) ;
    else cairo_set_source_rgba(cr, clr.r(), clr.g(), clr.b(), clr.a() ) ;

    cairo_set_line_width (cr, pen.lineWidth());

    cairo_set_miter_limit (cr, pen.miterLimit()) ;

    switch ( pen.lineCap() ) {
    case LineCap::Butt:
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT) ;
        break ;
    case LineCap::Round:
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND) ;
        break ;
    case LineCap::Square:
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE) ;
        break ;
    }

    switch ( pen.lineJoin() ) {
    case LineJoin::Miter:
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER) ;
        break ;
    case LineJoin::Round:
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND) ;
        break ;
    case LineJoin::Bevel:
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL) ;
        break ;
    }

    double dash_offset = pen.dashOffset() ;

    const auto &dash_array = pen.dashArray() ;

    if ( dash_array.empty() )
        cairo_set_dash(cr, 0, 0, 0) ;
    else
        cairo_set_dash(cr, &dash_array[0], dash_array.size(), dash_offset) ;
}

void Backend::cairo_apply_linear_gradient(const LinearGradientBrush &lg) {
    cairo_t *cr = cr_  ;
    cairo_pattern_t *pattern;
    cairo_matrix_t matrix;

    double x0 = lg.x0(), y0 = lg.y0(), x1 = lg.x1(), y1 = lg.y1() ;

    pattern = cairo_pattern_create_linear ( x0, y0, x1, y1 ) ;

    Matrix2d tr = lg.transform() ;
    cairo_matrix_init (&matrix, tr.m1(), tr.m2(), tr.m3(), tr.m4(), tr.m5(), tr.m6());
    cairo_matrix_invert (&matrix);
    cairo_pattern_set_matrix (pattern, &matrix);

    if ( lg.spread() == SpreadMethod::Reflect )
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REFLECT);
    else if ( lg.spread() == SpreadMethod::Repeat )
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
    else
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);

    for( const auto &stop: lg.stops() ) {
        const Color &clr = stop.clr_ ;
        cairo_pattern_add_color_stop_rgba (pattern, stop.offset_, clr.r(), clr.g(), clr.b(), clr.a() * lg.fillOpacity() );
    }

    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);
}

void Backend::cairo_apply_radial_gradient(const RadialGradientBrush &rg) {
    cairo_t *cr = (cairo_t *)cr_  ;
    cairo_pattern_t *pattern;
    cairo_matrix_t matrix;

    double fx = rg.fx(), fy=rg.fy(), cx=rg.cx(), cy=rg.cy(), r=rg.radius() ;

    pattern = cairo_pattern_create_radial ( fx,	fy, 0.0, cx, cy, r) ;

    Matrix2d tr = rg.transform() ;
    cairo_matrix_init (&matrix, tr.m1(), tr.m2(), tr.m3(), tr.m4(), tr.m5(), tr.m6());
    cairo_matrix_invert (&matrix);
    cairo_pattern_set_matrix (pattern, &matrix);

    if ( rg.spread() == SpreadMethod::Reflect )
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REFLECT);
    else if ( rg.spread() == SpreadMethod::Repeat )
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
    else
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);

    for( const auto &stop: rg.stops() ) {
        const Color &clr = stop.clr_ ;
        cairo_pattern_add_color_stop_rgba (pattern, stop.offset_, clr.r(), clr.g(), clr.b(), clr.a() * rg.fillOpacity() );
    }

    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);
}

void Backend::cairo_apply_pattern(const PatternBrush &pat) {

    Canvas &c = pat.pattern() ;

    cairo_pattern_t *pattern = cairo_pattern_create_for_surface (c.surf_);

    if ( pat.spread() == SpreadMethod::Reflect )
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REFLECT);
    else if ( pat.spread() == SpreadMethod::Repeat )
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
    else
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);

    cairo_matrix_t matrix;

    Matrix2d tr = pat.transform() ;
    cairo_matrix_init (&matrix, tr.m1(), tr.m2(), tr.m3(), tr.m4(), tr.m5(), tr.m6());
    cairo_matrix_invert (&matrix);
    cairo_pattern_set_matrix (pattern, &matrix);

    cairo_pattern_set_filter (pattern, CAIRO_FILTER_BEST); //?

    cairo_set_source (cr_, pattern);

    cairo_pattern_destroy (pattern);
}
/*
void Backend::cairo_apply_pattern(const PatternBrush &pat)
{
    cairo_t *cr = (cairo_t *)cr_, *cr_pattern;
    cairo_pattern_t *pattern;

    double sw = pat.surf_.width_, sh = pat.surf_.height_ ;

    double width, height, x = 0.0, y = 0.0 ;

    if (pat.pu_ == PatternBrush::ObjectBoundingBox )
    {
        width = sw * vboxw ;
        height = sh * vboxh ;
        x = vboxx ;
        y = vboxy ;
    }
    else
    {
        width = sw ;
        height = sh ;
    }

    if ( width == 0.0 || height == 0.0 ) return ;

    cairo_surface_t *surface = cairo_surface_create_similar (cairo_get_target (cr),
                                                                 CAIRO_CONTENT_COLOR_ALPHA, width, height);
    cr_pattern = cairo_create (surface);

    cairo_save(cr_pattern) ;

    if (pat.pu_ == PatternBrush::ObjectBoundingBox )
    {
        cairo_matrix_t bboxmatrix;
        cairo_matrix_init (&bboxmatrix, vboxw, 0, 0, vboxh, 0, 0);
        cairo_transform(cr_pattern, &bboxmatrix)  ;
    }

    // replay pattern recording surface

    cairo_set_source_surface (cr_pattern, (cairo_surface_t *)pat.surf_.handle_, 0.0, 0.0);
    if ( pat.fill_opacity_ == 1.0 ) cairo_paint (cr_pattern);
    else cairo_paint_with_alpha(cr_pattern, pat.fill_opacity_);

    cairo_restore(cr_pattern) ;

  //  cairo_surface_write_to_png(surface, "/tmp/ff.png") ;

    pattern = cairo_pattern_create_for_surface (surface);
    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);

    cairo_matrix_t patm;
    cairo_matrix_init(&patm,pat.tr_.m[0],pat.tr_.m[1],
           pat.tr_.m[2],pat.tr_.m[3],
           pat.tr_.m[4],pat.tr_.m[5]) ;

//    cairo_matrix_invert(&patm) ;

    cairo_pattern_set_matrix (pattern, &patm);
    cairo_pattern_set_filter (pattern, CAIRO_FILTER_BEST); //?

    cairo_set_source (cr, pattern);

    cairo_pattern_destroy (pattern);
    cairo_destroy (cr_pattern);
    cairo_surface_destroy (surface);

}
*/

void Backend::fill_stroke_shape() {

    const State &state = state_.top();

    if ( state.brush_ )  {
        const auto &br = state.brush_ ;

        set_cairo_fill(br) ;

        if ( state.pen_  ) cairo_fill_preserve(cr_) ;
        else cairo_fill (cr_);
    }

    if ( state.pen_ )  {
        const Pen &pen = *state.pen_ ;
        set_cairo_stroke(pen) ;
        cairo_stroke(cr_) ;
    }

}

void Backend::set_cairo_fill(const std::shared_ptr<Brush> &br) {

    if ( br->fillRule() == FillRule::EvenOdd)
        cairo_set_fill_rule (cr_, CAIRO_FILL_RULE_EVEN_ODD);
    else if ( br->fillRule() == FillRule::NonZero )
        cairo_set_fill_rule (cr_, CAIRO_FILL_RULE_WINDING);

    if ( const auto &brush = dynamic_cast<SolidBrush *>(br.get()) )
    {
        const Color &clr = brush->color() ;

        if ( clr.a() * br->fillOpacity()  == 1.0 ) cairo_set_source_rgb(cr_, clr.r(), clr.g(), clr.b()) ;
        else cairo_set_source_rgba(cr_, clr.r(), clr.g(), clr.b(), clr.a() * br->fillOpacity() ) ;
    }
    else if ( const auto &brush = dynamic_cast<LinearGradientBrush *>(br.get()) )
        cairo_apply_linear_gradient(*brush) ;
    else if ( const auto &brush = dynamic_cast<RadialGradientBrush *>(br.get()) )
        cairo_apply_radial_gradient(*brush) ;
    else if ( const auto &brush = dynamic_cast<PatternBrush *>(br.get()) )
        cairo_apply_pattern(*brush) ;
}


void Backend::line_path(double x0, double y0, double x1, double y1) {
    cairo_move_to(cr_, x0, y0) ;
    cairo_line_to(cr_, x1, y1) ;
}

void Backend::polyline_path(double *pts, int n, bool close) {
    if ( n < 2 ) return ;

    cairo_move_to(cr_, pts[0], pts[1]) ;

    for(int i=1, k=2 ; i<n ; i++, k+=2 )
        cairo_line_to(cr_, pts[k], pts[k+1]) ;

    if ( close ) cairo_close_path(cr_) ;

    double x0, y0, x1, y1 ;
    cairo_path_extents(cr_, &x0, &y0, &x1, &y1) ;
    set_object_bbox(x0, y0, x1, y1) ;
}


void Backend::set_object_bbox(double x0, double y0, double x1, double y1) {
    vbox_.extend(x0, y0);
    vbox_.extend(x1, y1);
}


void Backend::path(const Path &path) {

    cairo_new_path(cr_) ;

    for( const auto &pcb: path.commands() ) {

        switch( pcb.cmd_ ) {
        case Path::MoveToCmd:
            cairo_move_to(cr_, pcb.arg0_, pcb.arg1_) ;
            break ;
        case Path::LineToCmd:
            cairo_line_to(cr_, pcb.arg0_, pcb.arg1_) ;
            break ;
        case Path::CurveToCmd:
            cairo_curve_to(cr_, pcb.arg0_, pcb.arg1_, pcb.arg2_, pcb.arg3_, pcb.arg4_, pcb.arg5_) ;
            break ;
        case Path::ClosePathCmd:;
            cairo_close_path(cr_) ;
            break ;
        }
    }

    double x0, y0, x1, y1 ;
    cairo_path_extents(cr_, &x0, &y0, &x1, &y1) ;
    set_object_bbox(x0, y0, x1, y1) ;
}


void Backend::rect_path(double x0, double y0, double w, double h) {
    cairo_rectangle(cr_, x0, y0, w, h);
    set_object_bbox(x0, y0, x0+w, y0+h) ;
}


const hb_tag_t KernTag = HB_TAG('k', 'e', 'r', 'n'); // kerning operations
static hb_feature_t KerningOn = { KernTag, 1, 0, std::numeric_limits<unsigned int>::max() };

hb_feature_t hb_features[] = { KerningOn } ;

static bool shape_text(const std::string &text, cairo_scaled_font_t *sf, cairo_glyph_t *&cglyphs, int &num_glyphs, const std::string &lang = "en")
{
    hb_buffer_t *buffer = hb_buffer_create();
    hb_buffer_set_unicode_funcs(buffer, hb_unicode_funcs_get_default());

    size_t len = text.length() ;

    hb_buffer_set_direction(buffer, HB_DIRECTION_LTR) ;
    hb_buffer_set_script(buffer, HB_SCRIPT_COMMON);
    hb_buffer_set_language(buffer, hb_language_from_string(lang.c_str(), -1)) ;
    hb_buffer_add_utf8(buffer, text.c_str(), len, 0, len);

    FT_Face face = cairo_ft_scaled_font_lock_face(sf) ;

    if ( face == 0 ) return false ;

    hb_font_t *font = hb_ft_font_create(face, NULL);

    hb_shape(font, buffer, hb_features, 1);

    hb_font_destroy(font);

    cairo_ft_scaled_font_unlock_face(sf) ;

    num_glyphs = hb_buffer_get_length(buffer);

    hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer, NULL);
    hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer, NULL);

    double x = 0, y = 0 ;

    cairo_glyph_t *cairo_glyphs = cairo_glyph_allocate (num_glyphs + 1);

    for (unsigned i=0; i<num_glyphs; i++)
    {
        cairo_glyphs[i].index = glyphs[i].codepoint ;
        cairo_glyphs[i].x = x + (positions[i].x_offset/64);
        cairo_glyphs[i].y = y - (positions[i].y_offset/64);

        x +=  positions[i].x_advance/64;
        y += -positions[i].y_advance/64;
    }

    cglyphs = cairo_glyphs;

    hb_buffer_destroy(buffer) ;
}

cairo_surface_t *cairo_create_image_surface(const Image &im)
{
    cairo_surface_t *psurf ;

    int width = im.width(), height = im.height() ;

    int src_stride = im.stride() ;

    psurf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height) ;

    // this is needed to work with transparency
    cairo_t *cr2 = cairo_create (psurf);
    cairo_set_operator (cr2, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba (cr2, 0, 0, 0, 1);
    cairo_paint (cr2);
    cairo_destroy (cr2);

    unsigned char *dst = cairo_image_surface_get_data(psurf) ;
    int dst_stride = cairo_image_surface_get_stride(psurf);

    unsigned char *dp, *drp = dst ;
    unsigned char *sp, *srp = (unsigned char *)im.pixels() ;

    if ( im.format() == ImageFormat::RGB24  )
    {
        for (int i = 0; i < height; i++)
        {
            dp = drp; sp = srp ;

            for (int j = 0; j < width; j++)
            {
                unsigned char red =   sp[0]  ;
                unsigned char green = sp[1]  ;
                unsigned char blue =  sp[2]  ;
                unsigned char alpha = 255 ;
                unsigned int p = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
                *((int32_t *)dp) = p ;

                dp += 4 ;
                sp += 3 ;
            }

            drp += dst_stride ;
            srp += src_stride ;
        }
    }
    else if ( im.format() == ImageFormat::ARGB32 )
    {
        for (int i = 0; i < height; i++)
        {
            dp = drp; sp = srp ;

            for (int j = 0; j < width; j++)
            {
                unsigned char alpha = sp[0] ;
                unsigned char red =   sp[1] ;
                unsigned char green = sp[2] ;
                unsigned char blue =  sp[3] ;

                unsigned int p = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);

                *((uint32_t *)dp) = p ;

                dp += 4 ;
                sp += 4 ;
            }

            drp += dst_stride ;
            srp += src_stride ;
        }
    }


    return psurf ;
}







////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void Backend::cairo_apply_font(const Font &font) {
    cairo_font_face_t *font_face = CairoFontFaceCache::instance().query_font_face_fc(font.family(), font.style(), font.weight()) ;

    cairo_set_font_face(cr_, font_face) ;
    cairo_set_font_size(cr_, font.size()) ;
}
*/

Backend::~Backend() {
    cairo_surface_finish (surf_);
    cairo_surface_destroy (surf_);
    cairo_destroy(cr_) ;
}


} // namespace detail

void Canvas::setPen(const Pen &pen) {
    state_.top().pen_ = std::make_shared<Pen>(pen) ;
}

void Canvas::setBrush(const SolidBrush &br) {
    state_.top().brush_ = std::make_shared<SolidBrush>(br) ;
}

void Canvas::setBrush(const LinearGradientBrush &br) {
    state_.top().brush_ = std::make_shared<LinearGradientBrush>(br) ;
}


void Canvas::setBrush(const RadialGradientBrush &br) {
    state_.top().brush_ = std::make_shared<RadialGradientBrush>(br) ;
}

void Canvas::setBrush(const PatternBrush &br) {
    state_.top().brush_ = std::make_shared<PatternBrush>(br) ;
}

void Canvas::save() {
    cairo_save(cr_) ;
    state_.push(state_.top()) ;
}

void Canvas::restore() 	{
    cairo_restore(cr_) ;
    state_.pop() ;
}


void Canvas::setFont(const Font &font) {
    state_.top().font_ = font ;
}

void Canvas::clearBrush() {
    state_.top().brush_.reset() ;
}

void Canvas::clearPen() {
    state_.top().pen_.reset() ;
}


void Canvas::drawText(const std::string &text, double x0, double y0, double width, double height, unsigned int flags)
{
    const Font &f = state_.top().font_ ;

    cairo_scaled_font_t *scaled_font = FontManager::instance().createFont(f) ;

    cairo_set_scaled_font(cr_, scaled_font) ;

    TextLayout layout(text, scaled_font, width) ;
    layout.run() ;

    double y = 0, tx = 0, ty = 0 ;

    if ( flags & TextAlignVCenter )
        ty = ( height - layout.height())/2;
    else if ( flags & TextAlignBottom )
        ty = height - layout.height()  ;

    for (const TextLine &line: layout.lines() ) {

        if ( flags & TextAlignHCenter )
            tx =  ( width - line.width_ )/2.0 ;
        else if ( flags & TextAlignRight )
            tx =  width - line.width_;

        if ( line.first_line_ )
            y += line.ascent_ ;

        unsigned num_glyphs = line.numGlyphs() ;

        cairo_glyph_t *cairo_glyphs = line.glyphs_ ;

        cairo_save(cr_) ;

        cairo_translate(cr_, x0 + tx, y0 + y + ty) ;

  //      cairo_show_glyphs(cr_, cairo_glyphs, num_glyphs) ;
        cairo_glyph_path(cr_, cairo_glyphs, num_glyphs);

        cairo_fill(cr_) ;

   //     fill_stroke_shape() ;
#if 0
        cairo_rectangle(cr_, 0, 0, layout.width(), -line.ascent_) ;
        cairo_rectangle(cr_, 0, 0, layout.width(), line.descent_) ;
#endif

        y += line.height_ ;
#if 0
        cairo_set_source_rgb(cr_, 1, 0, 0) ;

        cairo_stroke(cr_) ;
#endif
        cairo_restore(cr_) ;

    }


#if 0
    cairo_scaled_font_t *scaled_font = FontManager::instance().createFont(f) ;

    if ( !scaled_font ) return ;

    cairo_set_scaled_font(cr_, scaled_font) ;

    // do text shaping based on created font

    cairo_glyph_t *glyphs ;
    int num_glyphs =0 ;

    if ( !detail::shape_text(text, scaled_font, glyphs, num_glyphs) ) return ;

    // compute text placement

    cairo_text_extents_t extents ;

    cairo_glyph_extents(cr_, glyphs, num_glyphs, &extents);

    cairo_save(cr_) ;

    double tx, ty, tw = extents.width, th = extents.height ;

    if ( flags & TextAlignHCenter )
        tx = x0 + (width - tw - extents.x_bearing)/2.0 ;
    else if ( flags & TextAlignRight )
        tx = x0 + width - tw - extents.x_bearing;
    else
        tx = x0 - extents.x_bearing ;

    if ( flags & TextAlignTop )
        ty = y0 - extents.y_bearing;
    else if ( flags & TextAlignVCenter )
        ty = y0 - (th + extents.y_bearing) + ( height + th )/2;
    else if ( flags & TextAlignBaseline )
        ty = y0 + height ;
    else
        ty = y0 + height  - ( th + extents.y_bearing );

    cairo_save(cr_) ;

    cairo_translate(cr_, tx, ty) ;

    cairo_glyph_path(cr_, glyphs, num_glyphs);

    fill_stroke_shape() ;

    cairo_rectangle(cr_, 0, 0, tw, th) ;

    cairo_set_source_rgb(cr_, 1, 0, 0) ;

    cairo_restore(cr_) ;
#endif
}



void Canvas::drawText(const string &str, double x0, double y0)
{
    const Font &f = state_.top().font_ ;
    cairo_scaled_font_t *scaled_font = FontManager::instance().createFont(f) ;

    if ( !scaled_font ) return ;

    cairo_set_scaled_font(cr_, scaled_font) ;

    // do text shaping based on created font

    cairo_glyph_t *glyphs ;
    int num_glyphs =0 ;

    if ( !detail::shape_text(str, scaled_font, glyphs, num_glyphs) ) return ;

    // compute text placement

    cairo_text_extents_t extents ;

    cairo_glyph_extents(cr_, glyphs, num_glyphs, &extents);

    cairo_save(cr_) ;

    cairo_translate(cr_, x0 + extents.x_bearing, y0) ;

    cairo_glyph_path(cr_, glyphs, num_glyphs);

    fill_stroke_shape() ;

    cairo_rectangle(cr_, 0, 0, extents.width,  extents.y_bearing) ;

    cairo_set_source_rgb(cr_, 1, 0, 0) ;
    cairo_stroke(cr_) ;

    cairo_restore(cr_) ;
}


static void cairo_push_transform(cairo_t *cr, const Matrix2d &a)
{
    cairo_matrix_t matrix;
    cairo_matrix_init (&matrix, a.m1(), a.m2(), a.m3(), a.m4(), a.m5(), a.m6());
    cairo_transform (cr, &matrix);
}

void Canvas::setClipRect(double x0, double y0, double w, double h)
{
    cairo_rectangle(cr_, x0, y0, w, h) ;
    cairo_clip(cr_) ;
}

void Canvas::setClipPath(const Path &p, FillRule rule)
{
    path(p) ;

    if ( rule == FillRule::EvenOdd )
        cairo_set_fill_rule(cr_, CAIRO_FILL_RULE_EVEN_ODD) ;
    else
        cairo_set_fill_rule(cr_, CAIRO_FILL_RULE_WINDING) ;

    cairo_clip(cr_) ;
}

void Canvas::drawLine(double x0, double y0, double x1, double y1) {
    line_path(x0, y0, x1, y1) ;
    fill_stroke_shape() ;
}

void Canvas::drawLine(const Point2d &p1, const Point2d &p2)
{
    drawLine(p1.x(), p1.y(), p2.x(), p2.y()) ;
}

void Canvas::drawRect(double x0, double y0, double w, double h) {
    rect_path(x0, y0, w, h) ;
    fill_stroke_shape() ;
}

void Canvas::drawRect(const Rectangle2d &r) {
    drawRect(r.x(), r.y(), r.width(), r.height()) ;
}

void Canvas::drawPolyline(double *pts, int nPts) {
    polyline_path(pts, nPts, false) ;
    fill_stroke_shape() ;
}

void Canvas::drawPolygon(double *pts, int nPts)
{
    polyline_path(pts, nPts, true) ;
    fill_stroke_shape() ;
}

void Canvas::drawPath(const Path &p) {
    path(p) ;
    fill_stroke_shape() ;
}

void Canvas::drawCircle(double cx, double cy, double r)
{
    cairo_arc (cr_, cx, cy, r, 0.0, 2*M_PI) ;
    fill_stroke_shape() ;
}

void Canvas::drawCircle(const Point2d &center, double r) {
    drawCircle(center.x(), center.y(), r) ;
}

#define SVG_ARC_MAGIC ((double) 0.5522847498)

static void cairo_elliptical_arc_to(cairo_t *cr, double x2, double y2)
{
    double x1, y1 ;
    double cx, cy, rx, ry ;

    cairo_get_current_point (cr, &x1, &y1);
    rx = x2 - x1 ;
    ry = y2 - y1 ;

    if ( rx > 0 && ry > 0 )
    {
        cx = x1 ;  	cy = y2 ;

        cairo_curve_to(cr,
                       cx + SVG_ARC_MAGIC * rx, cy - ry,
                       cx + rx, cy - SVG_ARC_MAGIC * ry,
                       x2, y2) ;
    }
    else if ( rx < 0 && ry > 0 )
    {
        rx = -rx ;	cx = x2 ;	cy = y1 ;

        cairo_curve_to(cr,
                       cx + rx, cy + SVG_ARC_MAGIC * ry,
                       cx + SVG_ARC_MAGIC * rx,  cy + ry,
                       x2, y2) ;
    }
    else if ( rx < 0 && ry < 0 )
    {
        rx = -rx ; ry = -ry ;	cx = x1 ;	cy = y2 ;

        cairo_curve_to(cr,
                       cx - rx*SVG_ARC_MAGIC, cy + ry,
                       cx - rx,  cy + SVG_ARC_MAGIC *ry,
                       x2, y2) ;
    }
    else {
        ry = -ry ;	cx = x2 ;	cy = y1 ;
        cairo_curve_to(cr,
                       cx - rx, cy - ry*SVG_ARC_MAGIC,
                       cx - rx*SVG_ARC_MAGIC,  cy - ry,
                       x2, y2) ;
    }

}

void Canvas::drawEllipse(double xp, double yp, double rxp, double ryp) {

    cairo_move_to(cr_, xp, yp - ryp) ;
    cairo_elliptical_arc_to(cr_, xp + rxp, yp) ;
    cairo_elliptical_arc_to(cr_, xp, yp + ryp) ;
    cairo_elliptical_arc_to(cr_, xp - rxp, yp) ;
    cairo_elliptical_arc_to(cr_, xp , yp - ryp) ;

    fill_stroke_shape() ;
}

Canvas::Canvas(double width, double height, double dpix, double dpiy): width_(width), height_(height), dpi_x_(dpix), dpi_y_(dpiy) {
}

ImageCanvas::ImageCanvas(double w, double h, double dpi): Canvas(w, h, dpi, dpi) {
    surf_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h) ;
    cr_ = cairo_create(surf_) ;
}

Image ImageCanvas::getImage()
{
    char *src = (char *)cairo_image_surface_get_data(surf_) ;
    unsigned width = cairo_image_surface_get_width(surf_) ;
    unsigned height = cairo_image_surface_get_height(surf_) ;
    unsigned src_stride = cairo_image_surface_get_stride(surf_) ;
    cairo_format_t src_format = cairo_image_surface_get_format(surf_) ;

    Image im(width, height, ImageFormat::ARGB32) ;

    unsigned dst_stride = im.stride() ;
    char *dst, *p, *q ;
    dst = im.pixels() ;
    uint i, j ;

    if ( src_format == CAIRO_FORMAT_ARGB32 ) {
        for( i=0 ; i<height ; i++, dst += dst_stride, src += src_stride ) {
            for( j=0, p=src, q=dst ; j<width ; j++ ) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
                char a = *p++, r = *p++, g = *p++, b = *p++ ;
#else

                char b = *p++, g = *p++, r = *p++, a = *p++ ;
#endif
                *q++ = a ; *q++ = r ; *q++ = g ; *q++ = b ;
            }
        }
    }

    return im ;

}


void Canvas::drawImage(const Image &im,  double opacity )
{
    cairo_surface_t *imsurf = detail::cairo_create_image_surface(im) ;

    cairo_save(cr_) ;

    cairo_set_source_surface(cr_, (cairo_surface_t *)imsurf, 0, 0);

    cairo_paint_with_alpha (cr_, opacity);

    cairo_surface_destroy(imsurf) ;
}


/*
Backend &Backend::drawImage(const std::string &pngImageFile, double x0, double y0, double w, double h, ViewBoxPolicy policy, ViewBoxAlign align,
                        double opacity )
{
    cairo_t *cr = (cairo_t *)cr_  ;

    cairo_surface_t *imsurf = cairo_image_surface_create_from_png(pngImageFile.c_str()) ;

    int width = cairo_image_surface_get_width(imsurf) ;
    int height = cairo_image_surface_get_height(imsurf) ;

    Transform trs = getViewBoxTransform(w, h, width, height, 0, 0, policy, align ) ;

    cairo_save(cr) ;

    cairo_translate(cr, x0, y0) ;
    cairo_push_transform(cr, trs) ;

    cairo_set_source_surface(cr, (cairo_surface_t *)imsurf, 0, 0);

    set_object_bbox(x0, y0, w, h) ;

    cairo_paint_with_alpha (cr, opacity);
    cairo_restore(cr) ;

    cairo_surface_destroy(imsurf) ;

    return *this ;

}

cv::Mat ImageSurface::getImage(bool transparency) const {

    cairo_surface_t *imsurf = (cairo_surface_t *)handle_ ;
    int width = cairo_image_surface_get_width(imsurf) ;
    int height = cairo_image_surface_get_height(imsurf) ;

    cv::Mat res(height, width, (transparency ) ? CV_8UC4: CV_8UC3) ;

    int dstride = res.step[0] ;

    unsigned char *data = cairo_image_surface_get_data(imsurf) ;
    int sstride = cairo_image_surface_get_stride(imsurf);

    unsigned char *dp, *drp = (unsigned char *)res.data ;
    unsigned char *sp, *srp = (unsigned char *)data ;

    for (int i = 0; i < height; i++)
    {
        dp = drp; sp = srp ;

        for (int j = 0; j < width; j++)
        {
            uint32_t val = *((uint32_t *)sp) ;
            *dp++ = ( val ) & 0xff ; *dp++ = ( val >> 8 ) & 0xff ;
            *dp++ = ( val >> 16 ) & 0xff ;

            if ( transparency ) *dp++ = ( val >> 24 ) & 0xff ;
            sp += 4 ;
        }

        drp += dstride ;
        srp += sstride ;
    }


    return res ;
}
*/
void Canvas::setTransform(const Matrix2d &tr) {
    cairo_push_transform(cr_, tr) ;
}


void Canvas::setAntialias(bool anti_alias)
{
    if ( anti_alias )
        cairo_set_antialias (cr_, CAIRO_ANTIALIAS_DEFAULT);
    else
        cairo_set_antialias (cr_, CAIRO_ANTIALIAS_NONE);
}

PatternCanvas::PatternCanvas(double width, double height): Canvas(width, height, 92, 92) {
    cairo_rectangle_t r{0, 0, width, height} ;
    surf_ = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, &r) ;
    cr_ = cairo_create(surf_) ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////




} // namespace xg
