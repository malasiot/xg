#ifndef __XPLOT_BRUSH_HPP__
#define __XPLOT_BRUSH_HPP__

#include <xg/color.hpp>
#include <xg/xform.hpp>

#include <vector>

namespace xg {

enum FillRule { EvenOddFillRule, NonZeroFillRule } ;

// abstract class for all brush type

class Brush {
public:

    virtual ~Brush() = default;

    void setFillRule(FillRule rule) ;
    void setFillOpacity(double opacity) ;

    double fillOpacity() const { return fill_opacity_ ; }
    FillRule fillRule() const { return fill_rule_ ; }

  //  enum Type { Solid, LinearGradient, RadialGradient, Pattern, None } ;

protected:

    Brush(): fill_rule_(EvenOddFillRule), fill_opacity_(1.0) {}

public:

    FillRule fill_rule_ ;
    double fill_opacity_ ;
} ;


class SolidBrush: public Brush {

public:

    SolidBrush(const Color &clr): clr_(clr) {}

    const Color color() const { return clr_ ; }

private:

    Color clr_ ;
} ;

enum SpreadMethod { PadSpread, RepeatSpread, ReflectSpread } ;
enum BrushUnits { UserSpaceOnUse, ObjectBoundingBox } ;

class GradientBrush: public Brush {

public:

    void setSpread(SpreadMethod method) { sm_ = method ; }
    void setTransform(const Transform &trans) { tr_ = trans ; }
    void setUnits(BrushUnits un) { gu_ = un ; }


    struct Stop {
        Stop(double offset, const Color &clr): offset_(offset), clr_(clr) {}
        double offset_ ;
        Color clr_ ;
    } ;

    GradientBrush &addStop(double offset, const Color &clr) {
        stops_.push_back(Stop(offset, clr)) ;
        return *this ;
    }


    BrushUnits units() const { return gu_ ; }
    SpreadMethod spread() const { return sm_ ; }
    const Transform &transform() const { return tr_ ; }
    const std::vector<Stop> &stops() const { return stops_ ; }

protected:

    GradientBrush(): sm_(PadSpread) {}

private:

    std::vector<Stop> stops_ ;
    SpreadMethod sm_ ;
    Transform tr_ ;
    BrushUnits gu_ ;

} ;


class LinearGradientBrush: public GradientBrush {

public:

    LinearGradientBrush(double x0, double y0, double x1, double y1, BrushUnits units = UserSpaceOnUse):
        x0_(x0), y0_(y0), x1_(x1), y1_(y1) { setUnits(units) ;}

    double x0() const { return x0_ ; }
    double y0() const { return y0_ ; }
    double x1() const { return x1_ ; }
    double y1() const { return y1_ ; }

private:

    double x0_, y0_, x1_, y1_ ;
} ;

class RadialGradientBrush: public GradientBrush {

public:

    RadialGradientBrush(double cx, double cy, double r, double fx = 0, double fy = 0, BrushUnits units = UserSpaceOnUse):
        cx_(cx), cy_(cy), fx_(fx), fy_(fy), r_(r)  { setUnits(units) ; }

    double cx() const { return cx_ ; }
    double cy() const { return cy_ ; }
    double fx() const { return fx_ ; }
    double fy() const { return fy_ ; }
    double radius() const { return r_ ; }

private:

    double cx_, cy_, fx_, fy_, r_ ;
} ;

struct PatternSurface {

} ;

class PatternBrush: public Brush {

public:

    enum Units { UserSpaceOnUse, ObjectBoundingBox } ;

    PatternBrush(PatternSurface &surf, Units units = UserSpaceOnUse):
        pu_(units), surf_(surf)  {}

    void setTransform(const Transform &trans) { tr_ = trans ; }

private:

    PatternSurface &surf_ ;
    Transform tr_ ;
    Units pu_ ;
    double width_, height_ ;

} ;

} // namespace xplot ;

#endif
