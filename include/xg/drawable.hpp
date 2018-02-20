#ifndef __XPLOT_DRAWABLE_HPP__
#define __XPLOT_DRAWABLE_HPP__

#include <memory>
#include <xplot/context.hpp>

namespace xplot {

class Drawable {
public:

    Drawable() {}

    virtual void draw(RenderingContext &) = 0 ;
    virtual void updateLayout(RenderingContext &) {}
} ;

typedef std::unique_ptr<Drawable> DrawablePtr ;

} // namespace xplot ;

#endif
