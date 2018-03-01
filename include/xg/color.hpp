#ifndef __XPLOT_COLOR_HPP__
#define __XPLOT_COLOR_HPP__

#include <string>
#include <stdexcept>

namespace xg {

struct NamedColor ;
struct CSSColor ;

// RGBA color container
// Components are always clamped in the range [0, 1]

class Color
{
public:
    Color() = default ;

    Color(double r, double g, double b, double alpha = 1.0):
        r_(clamp(r)), g_(clamp(g)), b_(clamp(b)), a_(clamp(alpha)) {}

    Color(const CSSColor &clr, double alpha = 1.0) ;

    double r() const noexcept { return r_ ; }
    double g() const noexcept { return g_ ; }
    double b() const noexcept { return b_ ; }
    double a() const noexcept { return a_ ; }

    void setAlpha(double alpha) noexcept { a_ = clamp(alpha) ; }
    void setRed(double red) noexcept { r_ = clamp(red) ; }
    void setGreen(double green) noexcept { g_ = clamp(green) ; }
    void setBlue(double blue) noexcept { b_ = clamp(blue) ; }

private:

    static double clamp(double value) ;

    double r_= 0, g_=0, b_=0, a_=1.0 ;
} ;

struct CSSColor {
public:

    // parse css color, throws ColorParseException
    CSSColor(const std::string &css_color_spec) ;

    CSSColor(unsigned char r, unsigned char g, unsigned char b):
        r_(r), g_(g), b_(b) {}

    unsigned char r_, g_, b_ ;
};

class CSSColorParseException: public std::runtime_error {
public:
    CSSColorParseException(const std::string &msg): std::runtime_error(msg) {}
} ;

// list of named colors

struct NamedColor: public CSSColor {
public:

    NamedColor(unsigned char r, unsigned char g, unsigned char b): CSSColor(r, g, b) {}

    static NamedColor alice_blue() noexcept;
    static NamedColor antique_white() noexcept;
    static NamedColor aqua() noexcept;
    static NamedColor aqua_marine() noexcept;
    static NamedColor azure() noexcept;
    static NamedColor beige() noexcept;
    static NamedColor bisque() noexcept;
    static NamedColor black() noexcept;
    static NamedColor blanched_almond() noexcept;
    static NamedColor blue() noexcept;
    static NamedColor blue_violet() noexcept;
    static NamedColor brown() noexcept;
    static NamedColor burlywood() noexcept;
    static NamedColor cadet_blue() noexcept;
    static NamedColor chartreuse() noexcept;
    static NamedColor chocolate() noexcept;
    static NamedColor coral() noexcept;
    static NamedColor cornflower_blue() noexcept;
    static NamedColor corn_silk() noexcept;
    static NamedColor crimson() noexcept;
    static NamedColor cyan() noexcept;
    static NamedColor dark_blue() noexcept;
    static NamedColor dark_cyan() noexcept;
    static NamedColor dark_golden_rod() noexcept;
    static NamedColor dark_gray() noexcept;
    static NamedColor dark_green() noexcept;
    static NamedColor dark_grey() noexcept;
    static NamedColor dark_khaki() noexcept;
    static NamedColor dark_magenta() noexcept;
    static NamedColor dark_olive_green() noexcept;
    static NamedColor dark_orange() noexcept;
    static NamedColor dark_orchid() noexcept;
    static NamedColor dark_red() noexcept;
    static NamedColor dark_salmon() noexcept;
    static NamedColor dark_sea_green() noexcept;
    static NamedColor dark_slate_blue() noexcept;
    static NamedColor dark_slate_gray() noexcept;
    static NamedColor dark_slate_grey() noexcept;
    static NamedColor dark_turquoise() noexcept;
    static NamedColor dark_violet() noexcept;
    static NamedColor deep_pink() noexcept;
    static NamedColor deep_sky_blue() noexcept;
    static NamedColor dim_gray() noexcept;
    static NamedColor dim_grey() noexcept;
    static NamedColor dodger_blue() noexcept;
    static NamedColor fire_brick() noexcept;
    static NamedColor floral_white() noexcept;
    static NamedColor forest_green() noexcept;
    static NamedColor fuchsia() noexcept;
    static NamedColor gainsboro() noexcept;
    static NamedColor ghostwhite() noexcept;
    static NamedColor gold() noexcept;
    static NamedColor goldenrod() noexcept;
    static NamedColor gray() noexcept;
    static NamedColor green() noexcept;
    static NamedColor green_yellow() noexcept;
    static NamedColor grey() noexcept;
    static NamedColor honey_dew() noexcept;
    static NamedColor hot_pink() noexcept;
    static NamedColor indian_red() noexcept;
    static NamedColor indigo() noexcept;
    static NamedColor ivory() noexcept;
    static NamedColor khaki() noexcept;
    static NamedColor lavender() noexcept;
    static NamedColor lavender_blush() noexcept;
    static NamedColor lawn_green() noexcept;
    static NamedColor lemon_chiffon() noexcept;
    static NamedColor light_blue() noexcept;
    static NamedColor light_coral() noexcept;
    static NamedColor light_cyan() noexcept;
    static NamedColor light_golden_rod_yellow() noexcept;
    static NamedColor light_gray() noexcept;
    static NamedColor light_green() noexcept;
    static NamedColor light_grey() noexcept;
    static NamedColor light_pink() noexcept;
    static NamedColor light_salmon() noexcept;
    static NamedColor light_sea_green() noexcept;
    static NamedColor light_sky_blue() noexcept;
    static NamedColor light_slate_gray() noexcept;
    static NamedColor light_slate_grey() noexcept;
    static NamedColor light_steel_blue() noexcept;
    static NamedColor light_yellow() noexcept;
    static NamedColor lime() noexcept;
    static NamedColor limegreen() noexcept;
    static NamedColor linen() noexcept;
    static NamedColor magenta() noexcept;
    static NamedColor maroon() noexcept;
    static NamedColor medium_aqua_marine() noexcept;
    static NamedColor medium_blue() noexcept;
    static NamedColor medium_orchid() noexcept;
    static NamedColor medium_purple() noexcept;
    static NamedColor medium_sea_green() noexcept;
    static NamedColor medium_slate_blue() noexcept;
    static NamedColor medium_spring_green() noexcept;
    static NamedColor medium_turquoise() noexcept;
    static NamedColor medium_violet_red() noexcept;
    static NamedColor midnight_blue() noexcept;
    static NamedColor mint_cream() noexcept;
    static NamedColor misty_rose() noexcept;
    static NamedColor moccasin() noexcept;
    static NamedColor navajo_white() noexcept;
    static NamedColor navy() noexcept;
    static NamedColor old_lace() noexcept;
    static NamedColor olive() noexcept;
    static NamedColor olive_drab() noexcept;
    static NamedColor orange() noexcept;
    static NamedColor orangered() noexcept;
    static NamedColor orchid() noexcept;
    static NamedColor pale_golden_rod() noexcept;
    static NamedColor pale_green() noexcept;
    static NamedColor pale_turquoise() noexcept;
    static NamedColor pale_violet_red() noexcept;
    static NamedColor papaya_whip() noexcept;
    static NamedColor peach_puff() noexcept;
    static NamedColor peru() noexcept;
    static NamedColor pink() noexcept;
    static NamedColor plum() noexcept;
    static NamedColor powderblue() noexcept;
    static NamedColor purple() noexcept;
    static NamedColor red() noexcept;
    static NamedColor rosy_brown() noexcept;
    static NamedColor royal_blue() noexcept;
    static NamedColor saddle_brown() noexcept;
    static NamedColor salmon() noexcept;
    static NamedColor sandybrown() noexcept;
    static NamedColor seagreen() noexcept;
    static NamedColor seashell() noexcept;
    static NamedColor sienna() noexcept;
    static NamedColor silver() noexcept;
    static NamedColor sky_blue() noexcept;
    static NamedColor slate_blue() noexcept;
    static NamedColor slate_gray() noexcept;
    static NamedColor slate_grey() noexcept;
    static NamedColor snow() noexcept;
    static NamedColor spring_green() noexcept;
    static NamedColor steel_blue() noexcept;
    static NamedColor tan() noexcept;
    static NamedColor teal() noexcept;
    static NamedColor thistle() noexcept;
    static NamedColor tomato() noexcept;
    static NamedColor turquoise() noexcept;
    static NamedColor violet() noexcept;
    static NamedColor wheat() noexcept;
    static NamedColor white() noexcept;
    static NamedColor white_smoke() noexcept;
    static NamedColor yellow() noexcept;
    static NamedColor yellow_green() noexcept;

    NamedColor() = delete ;

} ;
} // namespace xplot ;

#endif
