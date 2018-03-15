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
#include <xg/image.hpp>
#include <xg/rectangle.hpp>
#include <xg/glyph.hpp>
#include <xg/svg_document.hpp>

#include <xg/backends/cairo/canvas.hpp>

namespace xg {

enum TextAlignFlags {
    TextAlignLeft = 0x01, TextAlignRight = 0x02, TextAlignTop = 0x04, TextAlignBottom = 0x08, TextAlignHCenter = 0x10, TextAlignVCenter = 0x20, TextAlignBaseline = 0x40
}  ;

class Canvas: public detail::Backend {
protected:

    Canvas(double width, double height, double dpix, double dpiy) ;

    Canvas(Canvas &&op) noexcept = default ;
    Canvas& operator=(Canvas&& op) noexcept = default ;

public:

    void save() ;
    void restore() ;

    void setTransform(const Matrix2d &tr) ;

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
    void setClipPath(const Path &p, FillRule frule= FillRule::EvenOdd) ;

    void drawLine(double x0, double y0, double x1, double y1) ;
    void drawLine(const Point2d &p1, const Point2d &p2) ;
    void drawRect(double x0, double y0, double w, double h) ;
    void drawRect(const Rectangle2d &r) ;

    void drawPath(const Path &path) ;

    void drawPolyline(double *pts, int nPts) ;
    void drawPolygon(double *pts, int nPts) ;
    void drawCircle(double cx, double cy, double r) ;
    void drawCircle(const Point2d &center, double r) ;
    void drawEllipse(double xp, double yp, double ax, double ay) ;

    void drawText(const std::string &textStr, double x0, double y0) ;
    void drawText(const std::string &textStr, double x0, double y0, double width, double height, unsigned int flags) ;
    void drawText(const std::string &textStr, const Point2d &p) ;
    void drawText(const std::string &textStr, const Rectangle2d &r, unsigned int flags) ;

    void drawGlyph(const Glyph &g, const Point2d &p) ;
    void drawGlyphs(const std::vector<Glyph> &glyphs, const std::vector<Point2d> &positions) ;

    void drawImage(const Image &im,  double opacity) ;

    void drawSVG(const SVGDocument &doc) ;

    double width() const { return width_ ;  }
    double height() const { return height_ ;  }
    double dpiX() const { return dpi_x_ ; }
    double dpiY() const { return dpi_y_ ; }

protected:
    double width_, height_ ;
    double dpi_x_, dpi_y_ ;
} ;

class ImageCanvas: public Canvas
{
public:

    ImageCanvas(double width, double height, double dpi=300) ;

    Image getImage() ;

    void saveToPng(const std::string &fname) ;
} ;

// frontend for cairo recording surface
class PatternCanvas: public Canvas {
public:
    PatternCanvas(double width, double height) ;
} ;


} // namespace xg ;

#endif
