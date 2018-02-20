#ifndef __XG_CANVAS_HPP__
#define __XG_CANVAS_HPP__

#include <memory>

#include <string>
#include <xg/pen.hpp>
#include <xg/brush.hpp>
#include <xg/font.hpp>
#include <xg/xform.hpp>
#include <xg/path.hpp>
#include <xg/font.hpp>

#include <xg/backends/cairo/canvas.hpp>

namespace xg {

enum TextAlignFlags {
    TextAlignLeft = 0x01, TextAlignRight = 0x02, TextAlignTop = 0x04, TextAlignBottom = 0x08, TextAlignHCenter = 0x10, TextAlignVCenter = 0x20, TextAlignBaseline = 0x40
}  ;

class Canvas: public detail::Backend {
public:

    Canvas(double width, double height) ;

    Canvas(Canvas &&op) noexcept = default ;
    Canvas& operator=(Canvas&& op) noexcept = default ;

    void save() ;
    void restore() ;

    void setTransform(const Transform &tr) ;

    void setPen(const Pen &pen) ;
    void setBrush(const SolidBrush &brush) ;
    void setBrush(const LinearGradientBrush &brush) ;
    void setBrush(const RadialGradientBrush &brush) ;
    void setBrush(const PatternBrush &brush) ;
    void setFont(const Font &font) ;

    void clearBrush()  ;
    void clearPen() ;

    void setAntialias(bool antiAlias = true) ;

    void setClipRect(double x0, double y0, double w, double h) ;
    void setClipPath(const Path &p) ;

    void drawLine(double x0, double y0, double x1, double y1) ;
    void drawRect(double x0, double y0, double w, double h) ;
    void drawPath(const Path &path) ;

    void drawPolyline(double *pts, int nPts) ;
    void drawPolygon(double *pts, int nPts) ;
    void drawCircle(double cx, double cy, double r) ;
    void drawEllipse(double xp, double yp, double ax, double ay) ;

    void drawText(const std::string &textStr, double x0, double y0) ;
    void drawText(const std::string &textStr, double x0, double y0, double width, double height, unsigned int flags) ;

protected:
    double width_, height_ ;
} ;

class ImageCanvas: public Canvas
{
public:

    ImageCanvas(double width, double height, double dpi=300) ;

    void saveToPng(const std::string &fname) ;

private:
    double dpi_ ;
} ;


} // namespace xg ;

#endif
