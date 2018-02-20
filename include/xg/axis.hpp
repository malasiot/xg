#ifndef __XPLOT_AXIS_HPP__
#define __XPLOT_AXIS_HPP__

#include <xplot/drawable.hpp>
#include <xplot/label.hpp>
#include <xplot/tics.hpp>

namespace xplot {

class Axis: public Drawable {
public:

    Axis() {}

    void setDataRange(double minx, double maxx) ;
    double minValue() const { return display_range_min_ ; }
    double maxValue() const { return display_range_max_ ; }

private:

    float label_padding_ ;
    Label label_ ;
    Tics major_, minor_ ;
    double data_range_min_, data_range_max_ ;
    double display_range_min_, display_range_max_ ;
} ;

class XAxis: public Axis {
public:
    XAxis() {}
    void draw(RenderingContext &) override {}
    void updateLayout(RenderingContext &ctx) override;
};

class YAxis: public Axis {
public:
    YAxis() {}
    void draw(RenderingContext &) override {}
    void updateLayout(RenderingContext &ctx) override;
};


} // namespace xplot ;

#endif
