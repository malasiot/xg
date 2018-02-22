#ifndef __TEXT_LAYOUT_HPP__
#define __TEXT_LAYOUT_HPP__

#include <xg/font.hpp>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <unicode/unistr.h>
#include <unicode/uscript.h>
#include <unicode/ubidi.h>

#include "scrptrun.h"
#include <cairo/cairo.h>

#include <string>
#include <map>

struct Glyph {

    Glyph(unsigned cp, unsigned c_index): glyph_index_(cp), char_index_(c_index) {}

    double line_height_ ;
    unsigned glyph_index_;
    unsigned char_index_;
    cairo_scaled_font_t *face_ ;
    double advance_;
    double x_offset_, y_offset_ ;
};

struct TextLine {

    TextLine(int32_t first, int32_t last): first_(first), last_(last) {}

    void addGlyph(Glyph && glyph)  {
        height_ = std::max(height_, glyph.line_height_) ;
        double advance = glyph.advance_ ;

        if ( glyphs_.empty() )
        {
            width_ = advance;
            glyphs_width_ = advance;
            space_count_ = 0;
        }
        else if ( advance > 0 )
        {
            // Only add character spacing if the character is not a zero-width part of a cluster.
            width_ += advance ;
            glyphs_width_ += advance;
            ++space_count_;
        }
        glyphs_.emplace_back(std::move(glyph));
    }


    double max_char_height_ = 0; // Max height of any glyphs in line - calculated by shaper
    double height_ = 0 ; // font height
    double width_; // line width
    double glyphs_width_;
    int32_t first_;
    int32_t last_;
    bool first_line_ = false ;
    unsigned space_count_ = 0 ;
    std::vector<Glyph> glyphs_ ;
} ;

class TextLayout {
public:
    TextLayout(const std::string &text, cairo_scaled_font_t *sf, double wrap = -1) ;

    bool run() ;

    const std::vector<TextLine> &lines() const { return lines_ ; }

    double width() const { return width_ ; }

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
        std::vector<std::vector<GlyphInfo>> glyphs_ ;
        std::vector<unsigned> clusters_ ;
    } ;

    bool getGlyphsAndClusters(hb_buffer_t *buffer, GlyphCollection &glyphs) ;
    void fillGlyphInfo(GlyphCollection &glyphs, TextLine &line) ;
    void clearWidths(int32_t start, int32_t end) ;
    void addLine(TextLine&& line) ;

private:
    UnicodeString us_ ;
    cairo_scaled_font_t *font_ ;
    std::map<unsigned,double> width_map_ ;
    double wrap_width_ ;
    std::vector<TextLine> lines_ ;
    double width_, height_ ;
    unsigned glyphs_count_ ;
} ;

#endif
