#ifndef __XG_TEXT_LAYOUT_HPP__
#define __XG_TEXT_LAYOUT_HPP__

#include <string>
#include <memory>

#include <xg/font.hpp>
#include <xg/glyph.hpp>

class TextLayoutEngine ;

namespace xg {

enum class TextDirection { Auto, LeftToRight, RightToLeft } ;
class TextLine ;

class TextLayout {
public:
    TextLayout(const std::string &text, const Font &fd) ;

    void setWrapWidth(double width) ;
    void setLineSpacing(double ls) ;

    void setTextDirection(TextDirection dir) ;

    void compute() ;

    double width() const ;
    double height() const ;

    const std::vector<TextLine> &lines() const ;


private:

    std::shared_ptr<TextLayoutEngine> engine_ ;
};

class TextLine {

public:

    unsigned numGlyphs() const { return glyphs_.size() ; }

    // line height as given by the font metrics
    double height() const { return height_ ; }

    // line width
    double width() const { return width_ ; }

    // distance from base-line to highest point
    double ascent() const { return ascent_ ; }

    // distance from base-line to lowest hanging point
    double descent() const { return descent_ ;  }

    const std::vector<Glyph> &glyphs() const { return glyphs_ ; }

protected:

    friend class ::TextLayoutEngine ;

    TextLine(int32_t first, int32_t last): first_(first), last_(last) {}

    void addGlyph(Glyph && glyph)  {
        double advance = glyph.x_advance_ ;

        width_ += advance ;
        glyphs_.emplace_back(std::move(glyph));
    }

    double height_ = 0 ; // line height
    double width_ = 0; // line width
    double ascent_ ;  // distance from base-line to highest point
    double descent_ ; // distance from base-line to lowest hanging point

    int32_t first_; // index to first unicode code point in logical order
    int32_t last_;

    std::vector<Glyph> glyphs_ ;
} ;


}

#endif
