#include <xg/path.hpp>

#include <cmath>
using namespace std ;

namespace xg {

void Path::addCommand(Command cmd, double arg0, double arg1, double arg2, double arg3, double arg4, double arg5)
{
    switch ( cmd )
    {
    case MoveToCmd:
    case LineToCmd:
        cmds_.emplace_back(cmd, arg0, arg1) ;
        break ;
    case CurveToCmd:
        cmds_.emplace_back(cmd, arg0, arg1, arg2, arg3, arg4, arg5) ;
        break ;
    case ClosePathCmd:
        cmds_.emplace_back(cmd) ;
        break ;
    }

    previous_cmd_ = cmd ;
}


static void svg_path_arc_segment (double ctx[6],
double xc, double yc,
double th0, double th1, double rx, double ry, double x_axis_rotation)
{
    double sin_th, cos_th;
    double a00, a01, a10, a11;
    double x1, y1, x2, y2, x3, y3;
    double t;
    double th_half;

    sin_th = sin (x_axis_rotation * (M_PI / 180.0));
    cos_th = cos (x_axis_rotation * (M_PI / 180.0));
    /* inverse transform compared with rsvg_path_arc */
    a00 = cos_th * rx;
    a01 = -sin_th * ry;
    a10 = sin_th * rx;
    a11 = cos_th * ry;

    th_half = 0.5 * (th1 - th0);
    t = (8.0 / 3.0) * sin (th_half * 0.5) * sin (th_half * 0.5) / sin (th_half);
    x1 = xc + cos (th0) - t * sin (th0);
    y1 = yc + sin (th0) + t * cos (th0);
    x3 = xc + cos (th1);
    y3 = yc + sin (th1);
    x2 = x3 + t * sin (th1);
    y2 = y3 - t * cos (th1);

    ctx[0] = a00 * x1 + a01 * y1 ;
    ctx[1] = a10 * x1 + a11 * y1 ;
    ctx[2] = a00 * x2 + a01 * y2 ;
    ctx[3] = a10 * x2 + a11 * y2 ;
    ctx[4] = a00 * x3 + a01 * y3 ;
    ctx[5] = a10 * x3 + a11 * y3;
}

Path & Path::moveTo(double x, double y) {
    addCommand(MoveToCmd, cx_ = x, cy_ = y) ;
    return *this ;
}

Path & Path::moveToRel(double x, double y) {
    cx_ += x ; cy_ += y ;
    addCommand(MoveToCmd, cx_, cy_) ;
    return *this ;
}

Path & Path::closePath() {
    addCommand(ClosePathCmd) ;
    return *this ;
}

Path & Path::lineTo(double x, double y) {
    addCommand(LineToCmd, cx_ = x, cy_ = y) ;
    return *this ;
}

Path & Path::lineToRel(double x, double y) {
    cx_ += x ; cy_ += y ;
    addCommand(LineToCmd, cx_, cy_) ;
    return *this ;
}

Path & Path::lineToHorz(double x) {
    addCommand(LineToCmd, cx_ = x, cy_) ;
    return *this ;
}

Path & Path::lineToHorzRel(double x) {
    addCommand(LineToCmd, cx_ += x, cy_) ;
    return *this ;
}

Path & Path::lineToVert(double y) {
    addCommand(LineToCmd, cx_, cy_ = y) ;
    return *this ;
}

Path & Path::lineToVertRel(double y) {
    addCommand(LineToCmd, cx_, cy_ += y) ;
    return *this ;
}

Path & Path::curveTo(double x, double y, double x1, double y1, double x2, double y2) {
    addCommand(CurveToCmd, x, y, rx_ = x1, ry_ = y1, cx_ = x2, cy_ = y2) ;
    return *this ;
}

Path & Path::curveToRel(double x, double y, double x1, double y1, double x2, double y2) {
    addCommand(CurveToCmd,  cx_ + x, cy_ + y,
                     rx_ = cx_ + x1, ry_ = cy_ + y1,
                     cx_ + x2, cy_ + y2) ;
    cx_ += x2 ; cy_ += y2 ;
    return *this ;
}

Path & Path::quadTo(double arg1, double arg2, double arg3, double arg4) {
    rx_ = arg1 ; ry_ = arg2 ;

    /* raise quadratic bezier to cubic */
    double x1 = (cx_ + 2 * arg1) * (1.0 / 3.0);
    double y1 = (cy_ + 2 * arg2) * (1.0 / 3.0);
    double x3 = arg3 ;
    double y3 = arg4 ;
    double x2 = (x3 + 2 * arg1) * (1.0 / 3.0);
    double y2 = (y3 + 2 * arg2) * (1.0 / 3.0);

    addCommand(CurveToCmd, x1, y1, x2, y2, x3, y3) ;

    cx_ = arg3 ;
    cy_ = arg4 ;

    previous_cmd_ = QuadCurveToCmd ;

    return *this ;
}


Path & Path::quadToRel(double arg1, double arg2, double arg3, double arg4) {
    arg1 += cx_ ; arg3 += cx_ ;
    arg2 += cy_ ; arg4 += cy_ ;

    quadTo(arg1, arg2, arg3, arg4) ;

    return *this ;
}

Path & Path::smoothCurveTo(double arg3, double arg4, double arg5, double arg6)
{
    double arg1, arg2 ;

    if ( previous_cmd_ == CurveToCmd ) {
        arg1 = 2 * cx_ - rx_ ;
        arg2 = 2 * cy_ - ry_ ;
    }
    else {
        arg1 = cx_ ; arg2 = cy_ ;
    }

    addCommand(CurveToCmd,
                     arg1, arg2, rx_ = arg3, ry_ = arg4, cx_ = arg5, cy_ = arg6) ;

    return *this ;

}

Path & Path::smoothCurveToRel(double arg3, double arg4, double arg5, double arg6) {
    double arg1, arg2 ;

    if ( previous_cmd_ == CurveToCmd ) {
        arg1 = 2 * cx_ - rx_ ;
        arg2 = 2 * cy_ - ry_ ;
    }
    else    {
        arg1 = cx_ ; arg2 = cy_ ;
    }

    arg1 -= cx_ ; arg2 -= cy_ ;

    addCommand(CurveToCmd,
                     cx_ + arg1, cy_ + arg2, rx_ = cx_ + arg3, ry_ = cy_ + arg4,
                     arg5 + cx_, arg6 + cy_) ;
    cx_ += arg5 ;
    cy_ += arg6 ;

    return *this ;
}

Path & Path::smoothQuadTo(double arg3, double arg4) {
    double arg1, arg2 ;

    if ( previous_cmd_ == QuadCurveToCmd ) {
        arg1 = 2 * cx_ - rx_ ;
        arg2 = 2 * cy_ - ry_ ;
    }
    else
    {
        arg1 = cx_ ; arg2 = cy_ ;
    }

    /* raise quadratic bezier to cubic */
    double x1 = (cx_ + 2 * arg1) * (1.0 / 3.0);
    double y1 = (cy_ + 2 * arg2) * (1.0 / 3.0);
    double x3 = arg3 ;
    double y3 = arg4 ;
    double x2 = (x3 + 2 * arg1) * (1.0 / 3.0);
    double y2 = (y3 + 2 * arg2) * (1.0 / 3.0);

    addCommand(CurveToCmd, x1, y1, x2, y2, x3, y3) ;

    cx_ = arg3 ;
    cy_ = arg4 ;

    previous_cmd_ = QuadCurveToCmd ;

    return *this ;
}

Path & Path::smoothQuadToRel(double arg3, double arg4) {
    double arg1, arg2 ;

    if ( previous_cmd_ == QuadCurveToCmd ) {
        arg1 = 2 * cx_ - rx_ ;
        arg2 = 2 * cy_ - ry_ ;
    }
    else {
        arg1 = cx_ ; arg2 = cy_ ;
    }

    arg3 += cx_ ; arg4 += cy_ ;

    /* raise quadratic bezier to cubic */
    double x1 = (cx_ + 2 * arg1) * (1.0 / 3.0);
    double y1 = (cy_ + 2 * arg2) * (1.0 / 3.0);
    double x3 = arg3 ;
    double y3 = arg4 ;
    double x2 = (x3 + 2 * arg1) * (1.0 / 3.0);
    double y2 = (y3 + 2 * arg2) * (1.0 / 3.0);

    addCommand(CurveToCmd, x1, y1, x2, y2, x3, y3) ;

    cx_ = arg3 ;
    cy_ = arg4 ;

    previous_cmd_ = QuadCurveToCmd ;

    return *this ;

}

Path & Path::arcTo(double arg1, double arg2, double arg3, bool arg4, bool arg5, double arg6, double arg7) {

    /**
            * rsvg_path_arc: Add an RSVG arc to the path context.
            * @ctx: Path context.
            * @rx: Radius in x direction (before rotation).
            * @ry: Radius in y direction (before rotation).
            * @x_axis_rotation: Rotation angle for axes.
            * @large_arc_flag: 0 for arc length <= 180, 1 for arc >= 180.
            * @sweep: 0 for "negative angle", 1 for "positive angle".
            * @x: New x coordinate.
            * @y: New y coordinate.
            *
            **/

    double rx = arg1 ;
    double ry = arg2 ;
    double x_axis_rotation = arg3 ;
    int large_arc_flag = (arg4 == false ) ? 0 : 1 ;
    int sweep_flag = (arg5 == false ) ? 0 : 1 ;
    double x = arg6 ;
    double y = arg7 ;

    double sin_th, cos_th;
    double a00, a01, a10, a11;
    double x0, y0, x1, y1, xc, yc;
    double d, sfactor, sfactor_sq;
    double th0, th1, th_arc;
    int i, n_segs;

    /* Check that neither radius is zero, since its isn't either
            geometrically or mathematically meaningful and will
            cause divide by zero and subsequent NaNs.  We should
            really do some ranged check ie -0.001 < x < 000.1 rather
            can just a straight check again zero.
            */
    if ((rx == 0.0) || (ry == 0.0)) return *this ;

    double cx = cx_, cy = cy_ ;
    sin_th = sin (x_axis_rotation * (M_PI / 180.0));
    cos_th = cos (x_axis_rotation * (M_PI / 180.0));
    a00 = cos_th / rx;
    a01 = sin_th / rx;
    a10 = -sin_th / ry;
    a11 = cos_th / ry;
    x0 = a00 * cx + a01 * cy;
    y0 = a10 * cx + a11 * cy;
    x1 = a00 * x + a01 * y;
    y1 = a10 * x + a11 * y;
    /* (x0, y0) is current point in transformed coordinate space.
            (x1, y1) is new point in transformed coordinate space.

            The arc fits a unit-radius circle in this space.
            */
    d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    sfactor_sq = 1.0 / d - 0.25;
    if (sfactor_sq < 0) sfactor_sq = 0;
    sfactor = sqrt (sfactor_sq);
    if (sweep_flag == large_arc_flag)  sfactor = -sfactor;
    xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
    yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);
    /* (xc, yc) is center of the circle. */

    th0 = atan2 (y0 - yc, x0 - xc);
    th1 = atan2 (y1 - yc, x1 - xc);

    th_arc = th1 - th0;
    if (th_arc < 0 && sweep_flag) th_arc += 2 * M_PI;
    else if (th_arc > 0 && !sweep_flag) th_arc -= 2 * M_PI;

    n_segs = ceil (fabs (th_arc / (M_PI * 0.5 + 0.001)));

    for (i = 0; i < n_segs; i++)
    {
        double ccc[6] ;
        svg_path_arc_segment (ccc, xc, yc,
                              th0 + i * th_arc / n_segs,
                              th0 + (i + 1) * th_arc / n_segs, rx, ry, x_axis_rotation);

        addCommand(CurveToCmd,	ccc[0], ccc[1], ccc[2], ccc[3], ccc[4], ccc[5]) ;
    }

    cx_ = x ;
    cy_ = y ;

    return *this ;
}

Path & Path::arcToRel(double arg1, double arg2, double arg3, bool arg4, bool arg5, double arg6, double arg7) {

    arg6 += cx_ ; arg7 += cy_ ;

    arcTo(arg1, arg2, arg3, arg4, arg5, arg6, arg7) ;

    return *this ;
}

Path &Path::addEllipse(double x0, double y0, double r1, double r2) {
    moveTo(x0, y0-r2) ;
    arcTo(r1, r2, 0, true, true, x0, y0+r2) ;
    arcTo(r1, r2, 0, true, true, x0, y0-r2) ;
    closePath() ;
    return *this ;
}

Path &Path::addArc(double x0, double y0, double r1, double r2, double startAngle, double sweepAngle) {
    double sx = x0 + r1 * cos(startAngle * M_PI/180) ;
    double sy = y0 + r2 * sin(startAngle * M_PI/180) ;
    double ex = x0 + r1 * cos((startAngle + sweepAngle) * M_PI/180) ;
    double ey = y0 + r2 * sin((startAngle + sweepAngle) * M_PI/180) ;

    moveTo(sx, sy) ;
    arcTo(r1, r2, 0, (sweepAngle > 180 ) , true, ex, ey) ;

    return *this ;
}

Path &Path::addRect(double x0, double y0, double w, double h) {
    moveTo(x0, y0) ;
    lineTo(x0+w, y0) ;
    lineTo(x0+w, y0+h) ;
    lineTo(x0, y0+h) ;
    closePath() ;

    return *this ;
}

Path &Path::addRoundedRect(double xp, double yp, double wp, double hp, double rxp, double ryp) {

    if (rxp > fabs (wp / 2.))
        rxp = fabs (wp / 2.);
    if (ryp > fabs (hp / 2.))
        ryp = fabs (hp / 2.);

    if (rxp == 0) rxp = ryp;
    else if (ryp == 0) ryp = rxp ;

    if ( wp != 0.0 && hp != 0.0 )
    {
        if ( rxp == 0.0 || ryp == 0.0 )
            addRect(xp, yp, wp, hp) ;
        else
        {
            moveTo(xp + rxp, yp) ;
            lineTo(xp + wp - rxp, yp) ;
            arcTo(rxp, ryp, 0, false, true, xp + wp, yp + ryp) ;
            lineTo(xp + wp, yp + hp - ryp) ;
            arcTo(rxp, ryp, 0, false, true, xp + wp -rxp, yp + hp) ;
            lineTo(xp + rxp, yp + hp) ;
            arcTo(rxp, ryp, 0, false, true, xp, yp + hp - ryp) ;
            lineTo(xp, yp + ryp) ;
            arcTo(rxp, ryp, 0, false, true, xp + rxp, yp) ;
            closePath() ;
        }
    }


    return *this ;

}

Path & Path::addPath(const Path &other) {
    std::copy(other.cmds_.begin(), other.cmds_.end(),
              std::back_inserter(cmds_)) ;

    return *this ;
}



/*
extern void cairo_load_font(cairo_font_face_t * &ff, const char *fontFamilyStr,
                            FontStyle style,
                            FontWeight weight,
                            FontStretch stretch, double fontSize) ;

Path & Path::addText(const std::string &str, double x0, double y0, const Font &font)
{
    cairo_surface_t *surface = cairo_recording_surface_create(CAIRO_CONTENT_COLOR, 0) ;
    cairo_t *cr = cairo_create(surface) ;

    cairo_font_face_t *fontFace ;
    cairo_load_font(fontFace, font.family_.c_str(), font.style_, font.variant_,
                    font.weight_, font.stretch_, font.sz_) ;

    cairo_font_options_t *font_options = cairo_font_options_create ();

    cairo_matrix_t tmatrix, ctm ;
    cairo_matrix_init_scale(&tmatrix, font.sz_, font.sz_) ;

    cairo_matrix_init_identity(&ctm) ;

    cairo_scaled_font_t *sf = cairo_scaled_font_create (fontFace, &tmatrix, &ctm, font_options) ;

    cairo_set_scaled_font(cr, sf) ;

    cairo_move_to(cr, x0, y0) ;
    cairo_text_path(cr, str.c_str()) ;
    cairo_path_t *native = cairo_copy_path(cr) ;

    boost::shared_ptr<PathData> data = fromNative(native) ;

    cairo_path_destroy(native) ;

    cairo_destroy(cr) ;
    cairo_surface_destroy(surface) ;
    cairo_font_options_destroy(font_options) ;

    setData(data) ;

    return *this ;
}
*/




}
