#ifndef __XPLOT_LABEL_HPP__
#define __XPLOT_LABEL_HPP__

#include <string>
#include <xplot/drawable.hpp>

namespace xplot {

class Label: public Drawable {
public:

    Label() {}

    void draw(RenderingContext &) override {}


private:
    std::string name_ ;
} ;


} // namespace xplot ;

#endif
