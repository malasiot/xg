#ifndef __XPLOT_PLOT_HPP__
#define __XPLOT_PLOT_HPP__

#include <vector>
#include <xplot/drawable.hpp>
#include <xplot/line_plot.hpp>
#include <xplot/legend.hpp>
#include <xplot/axis.hpp>
#include <xplot/label.hpp>

namespace xplot {

class LinePlot ;
class Drawable ;

class Label ;
class Subplot ;

class Plot: public Drawable {
public:

    Plot() ;

    LinePlot &plot(const std::vector<double> &x, const std::vector<double> &y) ;

    Legend &legend() ;

    Axis &xAxis() ;
    Axis &yAxis() ;

    Label &title() ;

    void draw(RenderingContext &) override ;
    void updateLayout(RenderingContext &) override ;

private:

    Legend legend_ ;
    XAxis x_axis_ ;
    YAxis y_axis_ ;
    Label title_ ;
    std::vector<std::unique_ptr<Subplot>> elements_ ;

private:

    BoundingBox dataBounds() ;
    Transform dataTransform() ;

} ;


} // namespace xplot ;

#endif
