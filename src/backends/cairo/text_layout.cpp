#include "text_layout.hpp"
#include "font_manager.hpp"

#include <memory>
#include <iostream>

using namespace std ;
using xg::Font ;

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

void TextLayout::addLine(TextLine&& line)
{
    if (lines_.empty())
        line.first_line_ = true;

    height_ += line.height_ ;
    glyphs_count_ += line.glyphs_.size();
    width_ = std::max(width_, line.width_);
    lines_.emplace_back(std::move(line));
}



void TextLayout::breakLine(int32_t start, int32_t end) {
    cout << start << ' ' << end << endl ;

    TextLine line(start, end);

    shape(line);

    if ( wrap_width_ < 0 || line.width_ < wrap_width_ ) {
        addLine(std::move(line)) ;
    }

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

bool TextLayout::getGlyphsAndClusters(hb_buffer_t *buffer,  GlyphCollection &glyphs) {

    unsigned num_glyphs = hb_buffer_get_length(buffer);
    if ( num_glyphs == 0 ) return false ;

    hb_glyph_info_t *hb_glyphs = hb_buffer_get_glyph_infos(buffer, &num_glyphs);
    hb_glyph_position_t *hb_positions = hb_buffer_get_glyph_positions(buffer, &num_glyphs);

    unsigned cluster = hb_glyphs[0].cluster ;
    bool in_cluster = false;

    // collect clusters and associated glyphs

    glyphs.clusters_.push_back(cluster) ;

    for ( unsigned i = 0; i < num_glyphs; ++i ) {
        if (cluster != hb_glyphs[i].cluster) {
            cluster = hb_glyphs[i].cluster;
            glyphs.clusters_.push_back(cluster) ;
            in_cluster = false;
        }
        else in_cluster = true ;

        if ( glyphs.glyphs_.size() <= cluster )
               glyphs.glyphs_.resize(cluster + 1);

        auto &c = glyphs.glyphs_[cluster];
        if (c.empty())
            c.push_back({hb_glyphs[i], hb_positions[i]});
        else if (c.front().glyph_.codepoint == 0)
            c.front() = { hb_glyphs[i], hb_positions[i] };
        else if (in_cluster)
            c.push_back({ hb_glyphs[i], hb_positions[i] });
    }

    // check if all clusters/characters have associated glyphs

    bool all_set = true;

    for (const auto &c : glyphs.glyphs_)
    {
        if ( c.empty() || c.front().glyph_.codepoint == 0 ) {
            all_set = false;
            break;
        }
    }

    return all_set ;

}

void TextLayout::clearWidths(int32_t start, int32_t end) {
    for ( int32_t i = start; i<end; ++i )
        width_map_[i] = 0 ;
}

void TextLayout::fillGlyphInfo(GlyphCollection &glyphs, TextLine &line)
{
    cairo_font_extents_t f_extents ;
    cairo_scaled_font_extents (font_, &f_extents);

    double max_glyph_height = 0;

    // iterate all clusters

    for (auto c_id : glyphs.clusters_)
    {
        // iterate over all glyphs in this cluster

        auto &c = glyphs.glyphs_[c_id] ;

        for ( auto const& info : c ) {

            auto const& gpos = info.position_ ;
            auto const& glyph = info.glyph_ ;

            unsigned char_index = glyph.cluster;
            Glyph g(glyph.codepoint, char_index);

            cairo_glyph_t c_glyph ;
            c_glyph.index = info.glyph_.codepoint ;

            cairo_text_extents_t extents ;
            cairo_scaled_font_glyph_extents (font_, &c_glyph, 1, &extents) ;

            g.advance_ = gpos.x_advance/64.0 ;
            g.x_offset_ = gpos.x_offset/64.0;
            g.y_offset_ = gpos.y_offset/64.0 ;
            g.line_height_ = f_extents.height ;

            double height = extents.height ;
            max_glyph_height = std::max(height, max_glyph_height) ;
            width_map_[char_index] += g.advance_  ;

            line.addGlyph(std::move(g)) ;
        }
    }
    line.max_char_height_ = max_glyph_height ;
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

        GlyphCollection glyphs ;

        // initialize buffer with subtext and corresponding direction and script

        hb_buffer_clear_contents(buffer.get());
        hb_buffer_add_utf16(buffer.get(), us_.getBuffer(), us_.length(), text_item.start_, static_cast<int>(text_item.end_ - text_item.start_));
        hb_buffer_set_direction(buffer.get(), text_item.dir_);

        if ( !text_item.lang_.empty() )
             hb_buffer_set_language(buffer.get(), hb_language_from_string(text_item.lang_.c_str(), -1));

        hb_buffer_set_script(buffer.get(), text_item.script_);

         //   hb_ft_font_set_load_flags(font,FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING);

        FT_Face ft_face = cairo_ft_scaled_font_lock_face(font_) ;

        if ( ft_face == 0 ) continue ;

        hb_font_t *hb_font = hb_ft_font_create(ft_face, nullptr);

            // run shaper on this segment and font

        hb_shape(hb_font, buffer.get(), 0, 0);

        hb_font_destroy(hb_font);

        cairo_ft_scaled_font_unlock_face(font_) ;

        // get resulting glyphs and find which of the characters were correctly mapped by the current font face

        getGlyphsAndClusters(buffer.get(), glyphs) ;

        fillGlyphInfo(glyphs, line);
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


TextLayout::TextLayout(const string &text, cairo_scaled_font_t *font, double width): font_(font), wrap_width_(width) {
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
