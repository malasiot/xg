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
#include <unicode/unistr.h>
#include <unicode/uscript.h>
#include <unicode/ubidi.h>

#include "scrptrun.h"

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
    case LineCapButt:
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT) ;
        break ;
    case LineCapRound:
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND) ;
        break ;
    case LineCapSquare:
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE) ;
        break ;
    }

    switch ( pen.lineJoin() ) {
    case LineJoinMiter:
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER) ;
        break ;
    case LineJoinRound:
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND) ;
        break ;
    case LineJoinBevel:
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

    Eigen::Matrix3d tr = lg.transform().matrix() ;
    cairo_matrix_init (&matrix, tr(0, 0), tr(0, 1), tr(0, 2), tr(1, 0), tr(1, 1), tr(1, 2));

    if ( lg.units() == ObjectBoundingBox ) {
        cairo_matrix_t bboxmatrix;
        cairo_matrix_init (&bboxmatrix, vbox_.maxX() - vbox_.minX(), 0, 0, vbox_.maxY() - vbox_.minY(), vbox_.minX(), vbox_.minY());
        cairo_matrix_multiply (&matrix, &matrix, &bboxmatrix);
    }

    cairo_matrix_invert (&matrix);
    cairo_pattern_set_matrix (pattern, &matrix);

    if ( lg.spread() == ReflectSpread )
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REFLECT);
    else if ( lg.spread() == RepeatSpread )
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

    Eigen::Matrix3d tr = rg.transform().matrix() ;
    cairo_matrix_init (&matrix, tr(0, 0), tr(0, 1), tr(0, 2), tr(1, 0), tr(1, 1), tr(1, 2));

    if ( rg.units() == ObjectBoundingBox )
    {
        cairo_matrix_t bboxmatrix;
        cairo_matrix_init (&bboxmatrix, vbox_.maxX() - vbox_.minX(), 0, 0, vbox_.maxY() - vbox_.minY(), vbox_.minX(), vbox_.minY());
        cairo_matrix_multiply (&matrix, &matrix, &bboxmatrix);
    }

    cairo_matrix_invert (&matrix);
    cairo_pattern_set_matrix (pattern, &matrix);

    if ( rg.spread() == ReflectSpread )
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REFLECT);
    else if ( rg.spread() == RepeatSpread )
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

void Backend::cairo_apply_pattern(const PatternBrush &pat) {}
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
    cairo_t *cr = (cairo_t *)cr_  ;

    const State &state = state_.top();

    if ( state.brush_ )  {
        const auto &br = state.brush_ ;

        set_cairo_fill(br) ;

        if ( state.pen_  ) cairo_fill_preserve(cr) ;
        else cairo_fill (cr);
    }

    if ( state.pen_ )  {
        const Pen &pen = *state.pen_ ;
        set_cairo_stroke(pen) ;
        cairo_stroke(cr) ;
    }

}

void Backend::set_cairo_fill(const std::shared_ptr<Brush> &br) {

    if ( br->fillRule() == EvenOddFillRule)
        cairo_set_fill_rule (cr_, CAIRO_FILL_RULE_EVEN_ODD);
    else if ( br->fillRule() == NonZeroFillRule )
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


class CairoFontFaceCache {

public:

    cairo_font_face_t *find(const string &key) {
        std::lock_guard<std::mutex> g(mx_) ;
        map<string, cairo_font_face_t *>::const_iterator it = cache_.find(key) ;
        if ( it != cache_.end() ) return it->second ;
        else return 0 ;
    }

    void save(const string &key, cairo_font_face_t *face) {
        std::lock_guard<std::mutex> g(mx_) ;
        cache_.insert(std::make_pair(key, face)) ;
    }

    static CairoFontFaceCache &instance() {
        static CairoFontFaceCache s_instance ;
        return s_instance ;
    }

    cairo_font_face_t *query_font_face_fc(const std::string &family_name, FontStyle font_style, FontWeight font_weight) ;

private:

    static string font_face_key(const string &family_name, FontStyle font_style, FontWeight font_weight) ;


    std::map<string, cairo_font_face_t *> cache_ ;
    std::mutex mx_ ;
};




#ifdef _WIN32


static cairo_font_face_t *query_font_face(const std::string &familyName, FontStyle fontStyle, FontWeight fontWeight)
{
    LOGFONTW lFont ;

    memset(&lFont, 0, sizeof(LOGFONTW)) ;

    lFont.lfCharSet = DEFAULT_CHARSET ;
    lFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE ;

    switch ( fontWeight )
    {
    case BoldFontWeight:
        lFont.lfWeight = FW_BOLD ; break ;
    default:
        lFont.lfWeight = FW_NORMAL ; break ;
    }

    if ( fontStyle == ObliqueFontStyle ||  fontStyle == ItalicFontStyle )
        lFont.lfItalic = TRUE ;

    int length = ::MultiByteToWideChar(CP_UTF8, 0, familyName.c_str(), familyName.length(), 0, 0);
    wchar_t *output_buffer = new wchar_t [length];
    ::MultiByteToWideChar(CP_UTF8, 0, familyName.c_str(), familyName.length(), output_buffer, length);

    wcsncpy(lFont.lfFaceName, output_buffer, LF_FACESIZE) ;

    cairo_font_face_t *ff ;

    ff = cairo_win32_font_face_create_for_logfontw(&lFont) ;

    delete [] output_buffer ;

    return ff ;
}

#else

string CairoFontFaceCache::font_face_key(const string &family_name, FontStyle font_style, FontWeight font_weight) {
    string key = family_name  + '-' ;

    switch ( font_style )
    {
    case FontStyle::Normal:
        key += "normal-" ;
        break ;
    case FontStyle::Oblique:
        key += "oblique-" ;
        break ;
    case FontStyle::Italic:
        key += "italic-" ;
        break ;
    }

    switch ( font_weight )
    {
    case FontWeight::Normal:
        key += "normal" ;
        break ;
    case FontWeight::Bold:
        key += "bold" ;
        break ;
    }

    return key ;
}

cairo_font_face_t *CairoFontFaceCache::query_font_face_fc(const std::string &family_name, FontStyle font_style, FontWeight font_weight)
{
    string key = font_face_key(family_name, font_style, font_weight) ;

    cairo_font_face_t *face = find(key) ;

    if ( face ) return face ;

    FcPattern* pat = FcPatternCreate() ;

    FcPatternAddString(pat, FC_FAMILY, (const FcChar8*)(family_name.c_str()));

    if ( font_style == FontStyle::Italic )
        FcPatternAddInteger(pat, FC_SLANT, FC_SLANT_ITALIC) ;
    else if ( font_style == FontStyle::Oblique )
        FcPatternAddInteger(pat, FC_SLANT, FC_SLANT_OBLIQUE) ;
    else
        FcPatternAddInteger(pat, FC_SLANT, FC_SLANT_ROMAN) ;

    if ( font_weight == FontWeight::Bold )
        FcPatternAddInteger(pat, FC_WEIGHT, FC_WEIGHT_BOLD) ;
    else
        FcPatternAddInteger(pat, FC_WEIGHT, FC_WEIGHT_NORMAL) ;

    FcPatternAddBool(pat, FC_SCALABLE, FcTrue);


    cairo_font_options_t *font_options =  cairo_font_options_create ();

    // more recent versions of cairo support advanced text rendering options
    cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_DEFAULT) ;

    cairo_ft_font_options_substitute(font_options, pat) ;

    FcConfigSubstitute(0, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    FcChar8* fontConfigFamilyNameAfterConfiguration;
    FcPatternGetString(pat, FC_FAMILY, 0, &fontConfigFamilyNameAfterConfiguration);

    FcResult fontConfigResult;
    FcPattern *resultPattern = FcFontMatch(0, pat, &fontConfigResult);
    if (!resultPattern) // No match.
        return 0;

    FcChar8* fontConfigFamilyNameAfterMatching;
    FcPatternGetString(resultPattern, FC_FAMILY, 0, &fontConfigFamilyNameAfterMatching);


    face = cairo_ft_font_face_create_for_pattern(resultPattern) ;

    cairo_font_options_destroy(font_options) ;

    FcPatternDestroy(pat) ;

    FcPatternDestroy(resultPattern) ;

    save(key, face) ;

    return face ;
}

cairo_scaled_font_t *cairo_setup_font(const string &family_name, FontStyle font_style, FontWeight font_weight, double font_size)
{
    cairo_font_face_t *face = CairoFontFaceCache::instance().query_font_face_fc(family_name, font_style, font_weight) ;

    // create scaled font

    cairo_matrix_t ctm, font_matrix;
    cairo_font_options_t *font_options;

    cairo_matrix_init_identity (&ctm);
    cairo_matrix_init_scale (&font_matrix, font_size, font_size);
    font_options = cairo_font_options_create ();
    cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_NONE);
    cairo_font_options_set_hint_metrics (font_options, CAIRO_HINT_METRICS_OFF);

    cairo_scaled_font_t *scaled_font = cairo_scaled_font_create (face,
                                                                 &font_matrix,
                                                                 &ctm,
                                                                 font_options);

    cairo_font_options_destroy (font_options);

    if ( cairo_scaled_font_status(scaled_font) != CAIRO_STATUS_SUCCESS ) {
        cairo_font_face_destroy (face);
        return nullptr ;
    }

    return scaled_font ;
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


//


class TextLayout {
public:
    TextLayout(const string &text, const Font &font) ;

    bool run() ;

    bool breakLines() ;

private:


    struct TextItem {
        uint start_ ;
        uint end_ ;
        hb_script_t script_ ;
        string lang_ ;
        hb_direction_t dir_ ;
    } ;

    struct GlyphInfo {


    };

    struct TextLine {

        TextLine(int32_t first, int32_t last): first_(first), last_(last) {}

        double line_height_; // Includes line spacing (returned by freetype)
        double max_char_height_; // Max height of any glyphs in line - calculated by shaper
        double width_;
        double glyphs_width_;
        int32_t first_;
        int32_t last_;
        bool first_line_;
        unsigned space_count_;
        vector<GlyphInfo> glyphs_ ;
    } ;


    static hb_direction_t icu_direction_to_hb(UBiDiDirection direction) {
        return (direction == UBIDI_RTL) ? HB_DIRECTION_RTL : HB_DIRECTION_LTR;
    }

    using DirectionRun = std::tuple<hb_direction_t, uint, uint> ;
    using LangScriptRun = std::tuple<hb_script_t, uint, uint> ;

    bool itemize(int32_t start, int32_t end, vector<TextItem> &items) ;
    bool itemizeBiDi(vector<DirectionRun> &d_runs, int32_t start, int32_t end) ;
    bool itemizeScript(vector<LangScriptRun> &runs) ;
    void mergeRuns(const vector<LangScriptRun> &script_runs, const vector<DirectionRun> &dir_runs, vector<TextItem> &items) ;
    void breakLine(int32_t start, int32_t end) ;
    bool shape(TextLine &line) ;

private:
    UnicodeString us_ ;
    const Font &font_ ;
} ;



bool TextLayout::itemizeBiDi(vector<DirectionRun> &d_runs, int32_t s_begin, int32_t s_end) {

    int32_t s_length = s_end - s_begin ;
    // Adapted from https://github.com/asmaAL-Bahanta/icu-BiDi-Example/blob/master/bidiExample.cpp

    std::unique_ptr<UBiDi, decltype(&ubidi_close)> bidi(ubidi_open(), ubidi_close) ;
    UErrorCode error_code = U_ZERO_ERROR;
    UBiDiLevel para_level= UBIDI_DEFAULT_LTR;

    // initialize algorithm with string
    ubidi_setPara(bidi.get(), us_.getBuffer() + s_begin, s_length, para_level, nullptr, &error_code);

    if ( U_SUCCESS(error_code) ) {
        UBiDiDirection direction = ubidi_getDirection(bidi.get());
        // if the string has a unique direction we are done
        if ( direction != UBIDI_MIXED )
            d_runs.emplace_back(icu_direction_to_hb(direction), s_begin, s_end);
        else {
            // enumerate detected directions
            int32_t count = ubidi_countRuns(bidi.get(), &error_code);

            if ( U_SUCCESS(error_code) ) {
                for( int32_t i=0; i<count; i++ ) {
                    int32_t run_start, run_length;

                    UBiDiDirection dir = ubidi_getVisualRun(bidi.get(), i, &run_start, &run_length);

                    run_start += s_begin ;

                    d_runs.emplace_back(icu_direction_to_hb(dir), run_start, run_start + run_length);
                }
            }
            else return false ;
        }
    }
    else
        return false ;

    return true ;
}



bool TextLayout::itemizeScript(vector<LangScriptRun> &runs) {

    ScriptRun script_run(us_.getBuffer(), us_.length());

    while ( script_run.next() ) {
        int32_t run_start = script_run.getScriptStart();
        int32_t run_end = script_run.getScriptEnd();
        UScriptCode run_code = script_run.getScriptCode();

        hb_script_t hb_script ;
        if ( run_code == USCRIPT_INVALID_CODE)
            hb_script = HB_SCRIPT_INVALID;
        else
            hb_script = hb_script_from_string(uscript_getShortName(run_code), -1);

        runs.emplace_back(hb_script, run_start, run_end);
    }

    return true ;
}


void TextLayout::mergeRuns(const vector<LangScriptRun> &script_runs, const vector<DirectionRun> &dir_runs, vector<TextItem> &items)
{
    for (auto &dir_run : dir_runs)
    {
        uint start = std::get<1>(dir_run) ;
        uint end = std::get<2>(dir_run);

        auto rtl_insertion_point = items.end();

        auto ms_it = script_runs.end() ;
        for ( auto it = script_runs.begin(); it != script_runs.end(); ++it ) {
            if (( std::get<1>(*it) <= start ) && ( std::get<2>(*it) > start) ) {
                ms_it = it ;
                break ;
            }
        }

        while (start < end)
        {
            TextItem item;
            item.start_ = start ;
            item.end_ = std::min(std::get<2>(*ms_it), end);
            item.script_ = std::get<0>(*ms_it) ;
            item.lang_ = ScriptRun::detectLanguage(item.script_);
            item.dir_ = std::get<0>(dir_run) ;

            if ( item.dir_ == HB_DIRECTION_LTR )
                items.emplace_back(item);
            else
                rtl_insertion_point = items.insert(rtl_insertion_point, item);

            start = item.end_ ;

            if ( std::get<2>(*ms_it) == start)
                ++ms_it;
        }
    }
}


void TextLayout::breakLine(int32_t start, int32_t end) {
    cout << start << ' ' << end << endl ;

    TextLine line(start, end);

    shape(line);
   #if 0
       double scaled_wrap_width = wrap_width_ * scale_factor_;
       if (scaled_wrap_width <= 0 || line.width() < scaled_wrap_width)
       {
           add_line(std::move(line));
           return;
       }
       if (text_ratio_ > 0)
       {
           double wrap_at;
           double string_width = line.width();
           double string_height = line.line_height();
           for (double i = 1.0;
                ((wrap_at = string_width/i)/(string_height*i)) > text_ratio_ && (string_width/i) > scaled_wrap_width;
                i += 1.0) ;
           scaled_wrap_width = wrap_at;
       }

       mapnik::value_unicode_string const& text = itemizer_.text();
       Locale locale; // TODO: Is the default constructor correct?
       UErrorCode status = U_ZERO_ERROR;
       std::unique_ptr<BreakIterator> breakitr(BreakIterator::createLineInstance(locale, status));

       // Not breaking the text if an error occurs is probably the best thing we can do.
       // https://github.com/mapnik/mapnik/issues/2072
       if (!U_SUCCESS(status))
       {
           add_line(std::move(line));
           MAPNIK_LOG_ERROR(text_layout) << " could not create BreakIterator: " << u_errorName(status);
           return;
       }

       breakitr->setText(text);
       double current_line_length = 0;
       int last_break_position = static_cast<int>(line.first_char());
       for (unsigned i = line.first_char(); i < line.last_char(); ++i)
       {
           // TODO: character_spacing
           std::map<unsigned, double>::const_iterator width_itr = width_map_.find(i);
           if (width_itr != width_map_.end())
           {
               current_line_length += width_itr->second;
           }
           if (current_line_length <= scaled_wrap_width) continue;

           int break_position = wrap_before_ ? breakitr->preceding(i + 1) : breakitr->following(i);
           // following() returns a break position after the last word. So DONE should only be returned
           // when calling preceding.
           if (break_position <= last_break_position || break_position == static_cast<int>(BreakIterator::DONE))
           {
               // A single word is longer than the maximum line width.
               // Violate line width requirement and choose next break position
               break_position = breakitr->following(i);
               if (break_position == static_cast<int>(BreakIterator::DONE))
               {
                   break_position = line.last_char();
                   MAPNIK_LOG_ERROR(text_layout) << "Unexpected result in break_line. Trying to recover...\n";
               }
           }
           // Break iterator operates on the whole string, while we only look at one line. So we need to
           // clamp break values.
           if (break_position < static_cast<int>(line.first_char()))
           {
               break_position = line.first_char();
           }
           if (break_position > static_cast<int>(line.last_char()))
           {
               break_position = line.last_char();
           }
           bool adjust_for_space_character = break_position > 0 && text[break_position - 1] == 0x0020;

           text_line new_line(last_break_position, adjust_for_space_character ? break_position - 1 : break_position);
           clear_cluster_widths(last_break_position, adjust_for_space_character ? break_position - 1 : break_position);
           shape_text(new_line);
           add_line(std::move(new_line));

           last_break_position = break_position;
           i = break_position - 1;
           current_line_length = 0;
       }

       if (last_break_position == static_cast<int>(line.first_char()))
       {
           // No line breaks => no reshaping required
           add_line(std::move(line));
       }
       else if (last_break_position != static_cast<int>(line.last_char()))
       {
           text_line new_line(last_break_position, line.last_char());
           clear_cluster_widths(last_break_position, line.last_char());
           shape_text(new_line);
           add_line(std::move(new_line));
   }
#endif
}

bool TextLayout::shape(TextLine &line)
{
    unsigned start = line.first_ ;
    unsigned end = line.last_ ;
    std::size_t length = end - start;

    if ( !length ) return true ;

    // itemize text span
    vector<TextItem> items ;
    itemize(start, end, items);

    // prepare HarfBuzz shaping engine

    line.glyphs_.reserve(length);

    auto hb_buffer_deleter = [](hb_buffer_t * buffer) { hb_buffer_destroy(buffer);};
    const std::unique_ptr<hb_buffer_t, decltype(hb_buffer_deleter)> buffer(hb_buffer_create(), hb_buffer_deleter);

    hb_buffer_pre_allocate(buffer.get(), length);

    // perform shaping for each item, with unique script, direction

    for ( const auto & text_item : items ) {

        struct glyph_face_info
        {
            cairo_scaled_font_t *face;
            hb_glyph_info_t glyph;
            hb_glyph_position_t position;
        };

        for( const auto &face_name: font_.familyNames() ) {

            // initialize buffer with subtext and corresponding direction and script

            hb_buffer_clear_contents(buffer.get());
            hb_buffer_add_utf16(buffer.get(), us_.getBuffer(), us_.length(), text_item.start_, static_cast<int>(text_item.end_ - text_item.start_));
            hb_buffer_set_direction(buffer.get(), text_item.dir_);

            if ( !text_item.lang_.empty() )
                hb_buffer_set_language(buffer.get(), hb_language_from_string(text_item.lang_.c_str(), -1));

            hb_buffer_set_script(buffer.get(), text_item.script_);

         //   hb_ft_font_set_load_flags(font,FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING);

            // try to load font

            cairo_scaled_font_t *sf = cairo_setup_font(face_name, font_.style(), font_.weight(), font_.size()) ;

            FT_Face ft_face = cairo_ft_scaled_font_lock_face(sf) ;

            if ( ft_face == 0 ) return false ;

            hb_font_t *hb_font = hb_ft_font_create(ft_face, nullptr);

            // run shaper on this segment and font

            hb_shape(hb_font, buffer.get(), 0, 0);

            hb_font_destroy(hb_font);

            cairo_ft_scaled_font_unlock_face(sf) ;

            // get resulting glyphs and find which of the characters were correctly mapped by the current font face

            unsigned num_glyphs = hb_buffer_get_length(buffer.get());
            hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer.get(), &num_glyphs);
            hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer.get(), &num_glyphs);

        }
#if 0
           face_set_ptr face_set = font_manager.get_face_set(text_item.format_->face_name, text_item.format_->fontset);
           double size = text_item.format_->text_size * scale_factor;
           face_set->set_unscaled_character_sizes();
           std::size_t num_faces = face_set->size();

           font_feature_settings const& ff_settings = text_item.format_->ff_settings;
           int ff_count = safe_cast<int>(ff_settings.count());

           // rendering information for a single glyph
           struct glyph_face_info
           {
               face_ptr face;
               hb_glyph_info_t glyph;
               hb_glyph_position_t position;
           };

           // this table is filled with information for rendering each glyph, so that
           // several font faces can be used in a single text_item
           std::size_t pos = 0;
           std::vector<std::vector<glyph_face_info>> glyphinfos;

           glyphinfos.resize(text.length());
           for (auto const& face : *face_set)
           {
               ++pos;
               hb_buffer_clear_contents(buffer.get());
               hb_buffer_add_utf16(buffer.get(), detail::uchar_to_utf16(text.getBuffer()), text.length(), text_item.start, static_cast<int>(text_item.end - text_item.start));
               hb_buffer_set_direction(buffer.get(), (text_item.dir == UBIDI_RTL) ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);

               hb_font_t *font(hb_ft_font_create(face->get_face(), nullptr));
               auto script = detail::_icu_script_to_script(text_item.script);
               auto language = detail::script_to_language(script);
               MAPNIK_LOG_DEBUG(harfbuzz_shaper) << "RUN:[" << text_item.start << "," << text_item.end << "]"
                                                 << " LANGUAGE:" << hb_language_to_string(language)
                                                 << " SCRIPT:" << script << "(" << text_item.script << ") " << uscript_getShortName(text_item.script)
                                                 << " FONT:" << face->family_name();
               if (language != HB_LANGUAGE_INVALID)
               {
                   hb_buffer_set_language(buffer.get(), language); // set most common language for the run based script
               }
               hb_buffer_set_script(buffer.get(), script);

               // https://github.com/mapnik/test-data-visual/pull/25
   #if HB_VERSION_MAJOR > 0
   #if HB_VERSION_ATLEAST(1, 0 , 5)
               hb_ft_font_set_load_flags(font,FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING);
   #endif
   #endif
               hb_shape(font, buffer.get(), ff_settings.get_features(), ff_count);
               hb_font_destroy(font);

               unsigned num_glyphs = hb_buffer_get_length(buffer.get());
               hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer.get(), &num_glyphs);
               hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer.get(), &num_glyphs);

               unsigned cluster = 0;
               bool in_cluster = false;
               std::vector<unsigned> clusters;

               for (unsigned i = 0; i < num_glyphs; ++i)
               {
                   if (i == 0)
                   {
                       cluster = glyphs[0].cluster;
                       clusters.push_back(cluster);
                   }
                   if (cluster != glyphs[i].cluster)
                   {
                       cluster = glyphs[i].cluster;
                       clusters.push_back(cluster);
                       in_cluster = false;
                   }
                   else if (i != 0)
                   {
                       in_cluster = true;
                   }
                   if (glyphinfos.size() <= cluster)
                   {
                       glyphinfos.resize(cluster + 1);
                   }
                   auto & c = glyphinfos[cluster];
                   if (c.empty())
                   {
                       c.push_back({face, glyphs[i], positions[i]});
                   }
                   else if (c.front().glyph.codepoint == 0)
                   {
                       c.front() = { face, glyphs[i], positions[i] };
                   }
                   else if (in_cluster)
                   {
                       c.push_back({ face, glyphs[i], positions[i] });
                   }
               }
               bool all_set = true;
               for (auto c_id : clusters)
               {
                   auto const& c = glyphinfos[c_id];
                   if (c.empty() || c.front().glyph.codepoint == 0)
                   {
                       all_set = false;
                       break;
                   }
               }
               if (!all_set && (pos < num_faces))
               {
                   //Try next font in fontset
                   continue;
               }
               double max_glyph_height = 0;
               for (auto const& c_id : clusters)
               {
                   auto const& c = glyphinfos[c_id];
                   for (auto const& info : c)
                   {
                       auto const& gpos = info.position;
                       auto const& glyph = info.glyph;
                       unsigned char_index = glyph.cluster;
                       glyph_info g(glyph.codepoint,char_index,text_item.format_);
                       if (info.glyph.codepoint != 0) g.face = info.face;
                       else g.face = face;
                       if (g.face->glyph_dimensions(g))
                       {
                           g.scale_multiplier = g.face->get_face()->units_per_EM > 0 ?
                               (size / g.face->get_face()->units_per_EM) : (size / 2048.0) ;
                           //Overwrite default advance with better value provided by HarfBuzz
                           g.unscaled_advance = gpos.x_advance;
                           g.offset.set(gpos.x_offset * g.scale_multiplier, gpos.y_offset * g.scale_multiplier);
                           double tmp_height = g.height();
                           if (g.face->is_color())
                           {
                               tmp_height = g.ymax();
                           }
                           if (tmp_height > max_glyph_height) max_glyph_height = tmp_height;
                           width_map[char_index] += g.advance();
                           line.add_glyph(std::move(g), scale_factor);
                       }
                   }
               }
               line.update_max_char_height(max_glyph_height);
               break; //When we reach this point the current font had all glyphs.
           }
       }
   #endif
   }

}

bool TextLayout::itemize(int32_t start, int32_t end, vector<TextItem> &items) {
    using namespace icu ;

    // itemize directions
    vector<DirectionRun> dir_runs ;
    if ( !itemizeBiDi(dir_runs, start, end) ) return false ;

    // itemize scripts
    vector<LangScriptRun> script_runs ;
    if ( !itemizeScript(script_runs) ) return false ;

    mergeRuns(script_runs, dir_runs, items);

    return true ;
}


TextLayout::TextLayout(const string &text, const Font &font): font_(font) {
    us_ = UnicodeString::fromUTF8(text) ;
}

bool TextLayout::run() {
    int32_t start = 0, end = 0;

    while ( (end = us_.indexOf('\n', end) + 1) > 0 ) {
        breakLine(start, end) ;
        start = end;
    }

    breakLine(start, us_.length()) ;

    return true ;
}

#endif
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

    detail::TextLayout layout(text, f) ;
    layout.run() ;

    cairo_scaled_font_t *scaled_font = detail::cairo_setup_font(f.familyNames()[0], f.style(), f.weight(), f.size()) ;

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
}



void Canvas::drawText(const string &str, double x0, double y0)
{
    const Font &f = state_.top().font_ ;
    cairo_scaled_font_t *scaled_font = detail::cairo_setup_font(f.familyNames()[0], f.style(), f.weight(), f.size()) ;

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


static void cairo_push_transform(cairo_t *cr, const Transform &a)
{
    cairo_matrix_t matrix;
    Eigen::Matrix3d tr = a.matrix() ;
    cairo_matrix_init (&matrix, tr(0, 0), tr(0, 1), tr(0, 2), tr(1, 0), tr(1, 1), tr(1, 2));
    cairo_transform (cr, &matrix);
}

void Canvas::setClipRect(double x0, double y0, double w, double h)
{
    cairo_rectangle(cr_, x0, y0, w, h) ;
    cairo_clip(cr_) ;
}

void Canvas::setClipPath(const Path &p)
{
    path(p) ;
    cairo_clip(cr_) ;
}

void Canvas::drawLine(double x0, double y0, double x1, double y1) {
    line_path(x0, y0, x1, y1) ;
    fill_stroke_shape() ;
}

void Canvas::drawRect(double x0, double y0, double w, double h) {
    rect_path(x0, y0, w, h) ;
    fill_stroke_shape() ;
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

Canvas::Canvas(double width, double height): width_(width), height_(height) {
}

ImageCanvas::ImageCanvas(double w, double h, double dpi): Canvas(w, h), dpi_(dpi) {
    surf_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h) ;
    cr_ = cairo_create(surf_) ;
}

void ImageCanvas::saveToPng(const std::string &fileName) {
    cairo_surface_write_to_png ((cairo_surface_t *)surf_, fileName.c_str());
}
/*

cairo_surface_t *cairo_create_image_surface(const cv::Mat &im)
{
    cairo_surface_t *psurf ;

    int width = im.cols, height = im.rows ;

    int sstride = im.step[0] ;

    psurf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height) ;

    // this is needed to work with transparency
    cairo_t *cr2 = cairo_create (psurf);
    cairo_set_operator (cr2, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba (cr2, 0, 0, 0, 1);
    cairo_paint (cr2);
    cairo_destroy (cr2);

    unsigned char *data = cairo_image_surface_get_data(psurf) ;
    int dstride = cairo_image_surface_get_stride(psurf);

    unsigned char *dp, *drp = data ;
    unsigned char *sp, *srp = (unsigned char *)im.data ;

    if ( im.type() == CV_8UC1  )
    {
        for (int i = 0; i < height; i++)
        {
            dp = drp; sp = srp ;

            for (int j = 0; j < width; j++)
            {
                unsigned char val = *sp ;
                unsigned char red =   val  ;
                unsigned char green = val  ;
                unsigned char blue =  val  ;
                unsigned char alpha = 255 ;
                unsigned int p = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
                *((int32_t *)dp) = p ;

                dp += 4 ;
                sp ++ ;

            }

            drp += dstride ;
            srp += sstride ;
        }


    }
    else if ( im.type() == CV_8UC3 )
    {
        for (int i = 0; i < height; i++)
        {
            dp = drp; sp = srp ;

            for (int j = 0; j < width; j++)
            {
                unsigned char red =   sp[2]  ;
                unsigned char green = sp[1]  ;
                unsigned char blue =  sp[0]  ;
                unsigned char alpha = 255 ;
                unsigned int p = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
                *((int32_t *)dp) = p ;

                dp += 4 ;
                sp += 3 ;
            }

            drp += dstride ;
            srp += sstride ;
        }



    }
    else if ( im.type() == CV_8UC4 )
    {
        for (int i = 0; i < height; i++)
        {
            dp = drp; sp = srp ;

            for (int j = 0; j < width; j++)
            {
                unsigned char alpha = sp[3] ;
                unsigned char red =   ( sp[2] * alpha ) >> 8 ;
                unsigned char green = ( sp[1] * alpha ) >> 8 ;
                unsigned char blue =  ( sp[0] * alpha ) >> 8 ;

                unsigned int p = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);

                *((uint32_t *)dp) = p ;

                dp += 4 ;
                sp += 4 ;
            }

            drp += dstride ;
            srp += sstride ;
        }



    }


    return psurf ;
}


Backend & Backend::drawImage(const cv::Mat &im, double x0, double y0, double w, double h, ViewBoxPolicy policy, ViewBoxAlign align,
                        double opacity )
{
    cairo_t *cr = (cairo_t *)cr_  ;

    //cairo_surface_t *imsurf = cairo_image_surface_create_from_png("D:/Temp/oo.png") ;
    cairo_surface_t *imsurf = cairo_create_image_surface(im) ;

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
void Canvas::setTransform(const Transform &tr) {
    cairo_push_transform(cr_, tr) ;
}


void Canvas::setAntialias(bool anti_alias)
{
    if ( anti_alias )
        cairo_set_antialias (cr_, CAIRO_ANTIALIAS_DEFAULT);
    else
        cairo_set_antialias (cr_, CAIRO_ANTIALIAS_NONE);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////




} // namespace xg
