#ifndef __XG_GLYPH_HPP__
#define __XG_GLYPH_HPP__

#include <string>
#include <vector>

namespace xg {

struct Glyph {

    Glyph(unsigned cp): index_(cp) {}

    unsigned index_;  // glyph code point
    double x_advance_, y_advance_;  // amount to advance cursor
    double x_offset_, y_offset_ ;   // glyphs offset
 };


} // namespace xg ;

#endif
