#ifndef __XPLOT_SUBPLOT_HPP__
#define __XPLOT_SUBPLOT_HPP__

#include <xplot/drawable.hpp>
#include <xplot/bbox.hpp>

namespace xplot {

class Subplot: public Drawable {
public:

    Subplot() {}

    virtual BoundingBox getDataBounds() = 0;
private:

    friend class Plot ;
} ;


} // namespace xplot ;

#endif
