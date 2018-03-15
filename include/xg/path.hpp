#ifndef __XPLOT_PATH_HPP__
#define __XPLOT_PATH_HPP__

#include <string>
#include <memory>
#include <vector>

#include <xg/xform.hpp>
#include <xg/rectangle.hpp>
#include <xg/glyph.hpp>

namespace xg {

class PathData ;
class Font ;

class Path
{
public:

    Path() = default;
    ~Path() = default;

    // See SVG 1.1 Path specification (http://www.w3.org/TR/SVG/paths.html)

    Path & moveTo(double x, double y) ;
    Path & moveToRel(double x, double y) ;

    Path & lineTo(double x, double y) ;
    Path & lineToRel(double x, double y) ;

    Path & lineToHorz(double x) ;
    Path & lineToHorzRel(double x) ;

    Path & lineToVert(double y) ;
    Path & lineToVertRel(double y) ;

    Path & curveTo(double x, double y, double x1, double y1, double x2, double y2) ;
    Path & curveToRel(double x, double y, double x1, double y1, double x2, double y2) ;

    Path & quadTo(double x, double y, double x1, double y1) ;
    Path & quadToRel(double x, double y, double x1, double y1) ;

    Path & smoothCurveTo(double x, double y, double x1, double y1) ;
    Path & smoothCurveToRel(double x, double y, double x1, double y1) ;

    Path & smoothQuadTo(double x, double y) ;
    Path & smoothQuadToRel(double x, double y) ;

    Path & arcTo(double rx, double ry, double angle, bool largeArcFlag, bool sweepFlag, double x, double y) ;
    Path & arcToRel(double rx, double ry, double angle, bool largeArcFlag, bool sweepFlag, double x, double y) ;

    Path & closePath() ;

    Path & addEllipse(double x0, double y0, double r1, double r2) ;
    // The arc is traced along the perimeter of the ellipse bounded by the specified rectangle.
    // The starting point of the arc is determined by measuring clockwise from the x-axis of the
    // ellipse (at the 0-degree angle) by the number of degrees in the start angle.
    // The endpoint is similarly located by measuring clockwise from the starting point by the
    // number of degrees in the sweep angle.
    Path & addArc(double x0, double y0, double r1, double r2, double startAngle, double sweepAngle) ;
    Path & addRect(double x0, double y0, double w, double h) ;
    Path & addRoundedRect(double x0, double y0, double w, double h, double xrad, double yrad) ;
    Path & addPath(const Path &other) ;
    Path & addText(const std::string &str, double x0, double y0, const Font &font) ;
    Path & addGlyphs(const std::vector<Glyph> &glyphs, const std::vector<Point2d> &pos, const Font &font) ;


    Path &addPolygon(const std::vector<Point2d> &pts) ;
    Path &addPolyline(const std::vector<Point2d> &pts) ;

    Path transformed(const Matrix2d &m) const;

    // path bounding box
    Rectangle2d extents() const ;

    // return a flattened version of the path
    Path flattened() const ;

    enum Command { MoveToCmd, ClosePathCmd, LineToCmd, CurveToCmd, QuadCurveToCmd } ;

    struct CommandBlock {
        CommandBlock(Command cmd, double arg0=0, double arg1=0, double arg2=0, double arg3=0, double arg4=0, double arg5=0):
            cmd_(cmd), arg0_(arg0), arg1_(arg1), arg2_(arg2), arg3_(arg3), arg4_(arg4), arg5_(arg5) {}

        double arg0_, arg1_, arg2_, arg3_, arg4_, arg5_ ;

        Command cmd_ ;
    } ;

    const std::vector<CommandBlock> &commands() const { return cmds_ ; }

private:


    std::vector<CommandBlock> cmds_ ;
    double cx_, cy_, rx_, ry_ ;
    Command previous_cmd_ ;

    void addCommand(Command cmd, double arg0=0, double arg1=0, double arg2=0, double arg3=0, double arg4=0, double arg5=0) ;
} ;



} // namespace xplot ;

#endif
