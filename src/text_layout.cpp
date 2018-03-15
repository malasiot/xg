#include <xg/text_layout.hpp>

#include "backends/cairo/text_layout_engine.hpp"
#include "backends/cairo/font_manager.hpp"

namespace xg {

TextLayout::TextLayout(const std::string &text, const Font &fd) {
    engine_.reset(new TextLayoutEngine(text, fd)) ;
}

void TextLayout::setWrapWidth(double width) {
    engine_->setWrapWidth(width) ;
}

void TextLayout::setTextDirection(TextDirection dir) {
     engine_->setTextDirection(dir) ;
}

void TextLayout::compute() {
    engine_->run() ;
}

double TextLayout::width() const {
    return engine_->width() ;
}

double TextLayout::height() const {
    return engine_->height() ;
}

const std::vector<TextLine> &TextLayout::lines() const
{
    return engine_->lines() ;
}

}
