#ifndef __XPLOT_RENDERING_CONTEXT_HPP__
#define __XPLOT_RENDERING_CONTEXT_HPP__

#include <string>
#include <xg/pen.hpp>
#include <xg/brush.hpp>
#include <xg/font.hpp>
#include <xg/xform.hpp>
#include <xg/path.hpp>
#include <xg/font.hpp>

namespace xg {

enum TextAlignFlags {
    TextAlignLeft = 0x01, TextAlignRight = 0x02, TextAlignTop = 0x04, TextAlignBottom = 0x08, TextAlignHCenter = 0x10, TextAlignVCenter = 0x20, TextAlignBaseline = 0x40
} ;

class RenderingContext {
public:
    virtual void save() = 0 ;
    virtual void restore() = 0 ;

    virtual void pushTransform(const Transform &tr) = 0 ;
    virtual void popTransform() = 0 ;

    virtual void setPen(const Pen &pen) = 0 ;
    virtual void setBrush(const SolidBrush &brush) = 0 ;
    virtual void setBrush(const LinearGradientBrush &brush) = 0;
    virtual void setBrush(const RadialGradientBrush &brush) = 0;
    virtual void setBrush(const PatternBrush &brush) = 0;
    virtual void setFont(const Font &font) = 0;

    virtual void clearPen() = 0 ;
    virtual void clearBrush() = 0 ;

    virtual void setAntialias(bool antiAlias = true) = 0;

    virtual void setClipRect(double x0, double y0, double w, double h) = 0;
    virtual void setClipPath(const Path &p) = 0;

    virtual void drawLine(double x0, double y0, double x1, double y1) = 0;
    virtual void drawRect(double x0, double y0, double w, double h) = 0;
    virtual void drawPath(const Path &path) = 0;

    virtual void drawPolyline(double *pts, int nPts) = 0;
    virtual void drawPolygon(double *pts, int nPts) = 0;
    virtual void drawCircle(double cx, double cy, double r) = 0;
    virtual void drawEllipse(double xp, double yp, double ax, double ay) = 0;

    virtual void drawText(const std::string &textStr, double x0, double y0) = 0;
    virtual void drawText(const std::string &textStr, double x0, double y0, double width, double height, unsigned int flags) = 0;

} ;

} // namespace xplot ;

#endif
