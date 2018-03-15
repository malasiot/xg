#ifndef __XG_TEXT_LAYOUT_ENGINE_HPP__
#define __XG_TEXT_LAYOUT_ENGINE_HPP__

#include <xg/font.hpp>
#include <xg/text_layout.hpp>
#include <xg/glyph.hpp>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <unicode/unistr.h>
#include <unicode/uscript.h>
#include <unicode/ubidi.h>

#include "scrptrun.h"
#include <cairo/cairo.h>

#include <string>
#include <map>

using xg::Glyph ;
using xg::TextLine ;

class TextLayoutEngine {
public:
    TextLayoutEngine(const std::string &text, const xg::Font &f) ;

    void setWrapWidth(double w) ;
    void setTextDirection(xg::TextDirection dir) { bidi_mode_ = dir ; }
    bool run() ;

    const std::vector<TextLine> &lines() const { return lines_ ; }

    double width() const { return width_ ; }
    double height() const { return height_ ; }

    ~TextLayoutEngine();

private:


    struct TextItem {
        uint start_ ;
        uint end_ ;
        hb_script_t script_ ;
        std::string lang_ ;
        hb_direction_t dir_ ;
    } ;


    static hb_direction_t icu_direction_to_hb(UBiDiDirection direction) {
        return (direction == UBIDI_RTL) ? HB_DIRECTION_RTL : HB_DIRECTION_LTR;
    }

    using DirectionRun = std::tuple<hb_direction_t, uint, uint> ;
    using LangScriptRun = std::tuple<hb_script_t, uint, uint> ;

    bool itemize(int32_t start, int32_t end, std::vector<TextItem> &items) ;
    bool itemizeBiDi(std::vector<DirectionRun> &d_runs, int32_t start, int32_t end) ;
    bool itemizeScript(std::vector<LangScriptRun> &runs) ;
    void mergeRuns(const std::vector<LangScriptRun> &script_runs, const std::vector<DirectionRun> &dir_runs, std::vector<TextItem> &items) ;
    void breakLine(int32_t start, int32_t end) ;
    bool shape(TextLine &line) ;

    struct GlyphInfo {
        hb_glyph_info_t glyph_;
        hb_glyph_position_t position_;
    };

    struct GlyphCollection {
        unsigned num_glyphs_ ;
        std::vector<std::vector<GlyphInfo>> glyphs_ ;
        std::vector<unsigned> clusters_ ;
    } ;

    bool getGlyphsAndClusters(hb_buffer_t *buffer, GlyphCollection &glyphs) ;
    void fillGlyphInfo(GlyphCollection &glyphs, TextLine &line) ;
    void clearWidths(int32_t start, int32_t end) ;
    void addLine(TextLine&& line) ;
    void makeCairoGlyphsAndMetrics(TextLine &line);
    void computeHeight();

private:
    UnicodeString us_ ;
    cairo_scaled_font_t *font_ ;
    std::map<unsigned,double> width_map_ ;
    double wrap_width_ = -1 ;

    std::vector<TextLine> lines_ ;
    double width_ = 0, height_ = 0 ;
    unsigned glyphs_count_ ;
    char wrap_char_ = ' ';
    bool wrap_before_ = true ;
    bool repeat_wrap_char_ = false;
    xg::TextDirection bidi_mode_ = xg::TextDirection::Auto;


} ;


#endif
