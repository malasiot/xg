#ifndef __XPLOT_LINE_PLOT_HPP__
#define __XPLOT_LINE_PLOT_HPP__

#include <vector>
#include <xplot/subplot.hpp>
#include <xplot/color.hpp>
#include <xplot/bbox.hpp>

namespace xplot {

class LinePlot: public Subplot
{
public:

    LinePlot(const std::vector<double> &x, const std::vector<double> &y): data_pts_x_(x), data_pts_y_(y) {
        compute_bounds() ;
    }

    enum LineStyle { Solid, SolidSegments, Dashed, Dotted, DashDot, Empty } ;
    enum MarkerStyle { Circle, Square, XMark, Point, Plus, Star, Diamond, TriangleDown,
                       TriangleUp, TriangleLeft, TriangleRight, None } ;

    // Set the title of this line to appear on the graph legend

    void setLabel(const std::string &title) ;

    // Set line and marker appearance

    void setLineWidth(double width) ;
    void setLineStyle(LineStyle style) ;
    void setMarkerStyle(MarkerStyle ms) ;
    void setLineColor(const Color &) ;
    void setMarkerEdgeColor(const Color &) ;
    void setMarkerFaceColor(const Color &) ;
    void setMarkerSize(double sz) ;
    void setMarkerLineWidth(double mw) ;

    void draw(RenderingContext &) override {}

    BoundingBox getDataBounds() override ;

private:

    std::vector<double> data_pts_x_, data_pts_y_ ;
    BoundingBox bounds_ ;
    std::string title_ ;
    Color lc_, mec_, mfc_ ;
    MarkerStyle mstyle_ ;
    LineStyle lstyle_ ;
    double lw_, mlw_, msz_ ;

    void parseParamString(const char *wstr) ;

    void compute_bounds()  ;


} ;



} // namespace xplot ;

#endif
