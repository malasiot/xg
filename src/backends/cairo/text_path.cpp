#include <xg/path.hpp>

#include <cairo/cairo.h>
#include "font_manager.hpp"
#include "text_layout_engine.hpp"
namespace xg {

static void from_cairo_path(cairo_t *cr, Path &p)
{
    cairo_path_t *path = cairo_copy_path (cr);

    for ( int i=0 ; i < path->num_data; i += path->data[i].header.length)
    {
        cairo_path_data_t *data;

        data = &path->data[i];
        switch ( data->header.type )
        {
        case CAIRO_PATH_MOVE_TO:
            p.moveTo(data[1].point.x, data[1].point.y) ;
            break ;
        case CAIRO_PATH_LINE_TO:
            p.lineTo(data[1].point.x, data[1].point.y) ;
            break;
        case CAIRO_PATH_CURVE_TO:
            p.curveTo(data[1].point.x, data[1].point.y,
                    data[2].point.x, data[2].point.y,
                    data[3].point.x, data[3].point.y) ;
            break;
        case CAIRO_PATH_CLOSE_PATH:
            p.closePath() ;
            break;
        }
    }
}

Path &Path::addText(const std::string &text, double x0, double y0, const Font &f)
{

    cairo_surface_t *surface = cairo_recording_surface_create (CAIRO_CONTENT_ALPHA, NULL);
    cairo_t *cr = cairo_create(surface) ;

    cairo_scaled_font_t *scaled_font = FontManager::instance().createFont(f) ;

    cairo_set_scaled_font(cr, scaled_font);

    TextLayoutEngine layout(text, f) ;
    layout.run() ;

    const TextLine &line = layout.lines()[0] ;

    unsigned num_glyphs = line.numGlyphs() ;

    double x = x0 ;
    cairo_glyph_t *cairo_glyphs = cairo_glyph_allocate (num_glyphs + 1);

    for ( unsigned i=0; i<num_glyphs; i++) {
        const Glyph &g = line.glyphs()[i] ;
        cairo_glyphs[i].index = g.index_ ;
        cairo_glyphs[i].x = x + g.x_offset_ ;
        cairo_glyphs[i].y = y0 - g.y_offset_ ;

        x +=  g.x_advance_;
    }

    cairo_glyph_path(cr, cairo_glyphs, num_glyphs);

    cairo_glyph_free(cairo_glyphs) ;

    from_cairo_path(cr, *this) ;

    cairo_scaled_font_destroy(scaled_font) ;

    cairo_destroy(cr) ;

    cairo_surface_destroy(surface) ;

    return *this ;
}

Path &Path::addGlyphs(const std::vector<Glyph> &glyphs, const std::vector<Point2d> &pos, const Font &f)
{
    cairo_surface_t *surface = cairo_recording_surface_create (CAIRO_CONTENT_ALPHA, NULL);
    cairo_t *cr = cairo_create(surface) ;

    cairo_scaled_font_t *scaled_font = FontManager::instance().createFont(f) ;

    cairo_set_scaled_font(cr, scaled_font);

    unsigned num_glyphs = glyphs.size() ;

    cairo_glyph_t *cairo_glyphs = cairo_glyph_allocate (num_glyphs + 1);

    for ( unsigned i=0; i<num_glyphs; i++) {
        const Glyph &g = glyphs[i] ;
        cairo_glyphs[i].index = g.index_ ;
        cairo_glyphs[i].x = pos[i].x() ;
        cairo_glyphs[i].y = pos[i].y() ;
    }

    cairo_glyph_path(cr, cairo_glyphs, num_glyphs);

    cairo_glyph_free(cairo_glyphs) ;

    from_cairo_path(cr, *this) ;

    cairo_scaled_font_destroy(scaled_font) ;

    cairo_destroy(cr) ;

    cairo_surface_destroy(surface) ;

    return *this ;
}

}
