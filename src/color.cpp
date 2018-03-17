#include <xg/color.hpp>
#include <regex>
#include <cmath>

using namespace std ;

namespace xg {

struct ColorPair {
    const char *name;
    unsigned int r, g, b;
};

class ColorComp {
public:
    ColorComp() {}

    bool operator () ( const ColorPair &c, const std::string &name) const {
        return strcasecmp(c.name, name.c_str() ) < 0;
    }

    bool operator () ( const std::string &name, const ColorPair &c) const {
        return strcasecmp(name.c_str(), c.name ) < 0;
    }
};

bool operator < (const ColorPair &c1, const ColorPair &c2)
{
      return strcasecmp(c1.name, c2.name);
}

static int css_clip_rgb_percent (double in_percent)
{
    /* spec says to clip these values */
    if (in_percent > 100.) return 255;
    else if (in_percent <= 0.) return 0;
    return (int) floor (255. * in_percent / 100. + 0.5);
}

static int css_clip_rgb (int rgb)
{
    /* spec says to clip these values */
    if (rgb > 255) return 255;
    else if (rgb < 0) return 0;
    return rgb;
}

static bool parse_css_color(const string &str, unsigned char &r, unsigned char &g, unsigned char &b) {
    static regex clr_regex_1("#([0-9a-fA-F])([0-9a-fA-F])([0-9a-fA-F])(?:([0-9a-fA-F])([0-9a-fA-F])([0-9a-fA-F]))?", std::regex::icase) ;
    static regex clr_regex_2("rgb\\([\\s]*([0-9]+)[\\s]*,[\\s]*([0-9]+)[\\s]*,[\\s]*([0-9]+)[\\s]*\\)[\\s]*", std::regex::icase) ;
    static regex clr_regex_3("rgb\\([\\s]*([0-9]+)\\%[\\s]*,[\\s]*([0-9]+)\\%[\\s]*,[\\s]*([0-9]+)[\\s]*\\)[\\s]*", std::regex::icase) ;

    smatch what;

    unsigned int val = 0 ;

    if ( regex_match(str, what, clr_regex_1) ) {
        unsigned int hex, i ;

        for( i=1 ; i<what.size() ; i++ )
        {
            string s = what[i] ;

            if ( s.empty() ) break ;

            char c = s.at(0) ;

            if ( c >= '0' && c <= '9' ) hex = c - '0';
            else if ( c >= 'A' && c <= 'F' ) hex = c - 'A' + 10 ;
            else if ( c >= 'a' && c <= 'f' ) hex = c - 'a' + 10 ;

            val = (val << 4) + hex;
        }

        if ( i == 4 ) {
            r = static_cast<uint8_t>(((val & 0xf00) >> 4) | ((val & 0xf00) >> 8)) ;
            g = static_cast<uint8_t>(( val & 0xf0 ) | ( ( val & 0xf0 ) >> 4))  ;
            b = static_cast<uint8_t>((val & 0xf) | ((val & 0xf) << 4)) ;
        } else {
            r = static_cast<uint8_t>((val & 0xff0000) >> 16) ;
            g = static_cast<uint8_t>((val & 0xff00) >> 8)  ;
            b = static_cast<uint8_t>(val & 0xff) ;
        }
        return true ;
    }
    else if ( regex_match(str, what, clr_regex_2) ) {
        r = css_clip_rgb(atoi(what.str(1).c_str())) ;
        g = css_clip_rgb(atoi(what.str(2).c_str())) ;
        b = css_clip_rgb(atoi(what.str(3).c_str())) ;

        return true ;
    }
    else if ( regex_match(str, what, clr_regex_3) )  {
        r = css_clip_rgb_percent(atoi(what.str(1).c_str())) ;
        g = css_clip_rgb_percent(atoi(what.str(2).c_str())) ;
        b = css_clip_rgb_percent(atoi(what.str(3).c_str())) ;

        return true ;
    }
    else { // try named colors

        static const ColorPair color_list[] = {
            {"aliceblue", 240, 248, 255},
            {"antiquewhite", 250, 235, 215},
            {"aqua", 0, 255, 255},
            {"aquamarine", 127, 255, 212},
            {"azure", 240, 255, 255},
            {"beige", 245, 245, 220},
            {"bisque", 255, 228, 196},
            {"black", 0, 0, 0},
            {"blanchedalmond", 255, 235, 205},
            {"blue", 0, 0, 255},
            {"blueviolet", 138, 43, 226},
            {"brown", 165, 42, 42},
            {"burlywood", 222, 184, 135},
            {"cadetblue", 95, 158, 160},
            {"chartreuse", 127, 255, 0},
            {"chocolate", 210, 105, 30},
            {"coral", 255, 127, 80},
            {"cornflowerblue", 100, 149, 237},
            {"cornsilk", 255, 248, 220},
            {"crimson", 220, 20, 60},
            {"cyan", 0, 255, 255},
            {"darkblue", 0, 0, 139},
            {"darkcyan", 0, 139, 139},
            {"darkgoldenrod", 184, 132, 11},
            {"darkgray", 169, 169, 169},
            {"darkgreen", 0, 100, 0},
            {"darkgrey", 169, 169, 169},
            {"darkkhaki", 189, 183, 107},
            {"darkmagenta", 139, 0, 139},
            {"darkolivegreen", 85, 107, 47},
            {"darkorange", 255, 140, 0},
            {"darkorchid", 153, 50, 204},
            {"darkred", 139, 0, 0},
            {"darksalmon", 233, 150, 122},
            {"darkseagreen", 143, 188, 143},
            {"darkslateblue", 72, 61, 139},
            {"darkslategray", 47, 79, 79},
            {"darkslategrey", 47, 79, 79},
            {"darkturquoise", 0, 206, 209},
            {"darkviolet", 148, 0, 211},
            {"deeppink", 255, 20, 147},
            {"deepskyblue", 0, 191, 255},
            {"dimgray", 105, 105, 105},
            {"dimgrey", 105, 105, 105},
            {"dodgerblue", 30, 144, 255},
            {"firebrick", 178, 34, 34},
            {"floralwhite", 255, 255, 240},
            {"forestgreen", 34, 139, 34},
            {"fuchsia", 255, 0, 255},
            {"gainsboro", 220, 220, 220},
            {"ghostwhite", 248, 248, 255},
            {"gold", 255, 215, 0},
            {"goldenrod", 218, 165, 32},
            {"gray", 128, 128, 128},
            {"green", 0, 128, 0},
            {"greenyellow", 173, 255, 47},
            {"grey", 128, 128, 128},
            {"honeydew", 240, 255, 240},
            {"hotpink", 255, 105, 180},
            {"indianred", 205, 92, 92},
            {"indigo", 75, 0, 130},
            {"ivory", 255, 255, 240},
            {"khaki", 240, 230, 140},
            {"lavender", 230, 230, 250},
            {"lavenderblush", 255, 240, 245},
            {"lawngreen", 124, 252, 0},
            {"lemonchiffon", 255, 250, 205},
            {"lightblue", 173, 216, 230},
            {"lightcoral", 240, 128, 128},
            {"lightcyan", 224, 255, 255},
            {"lightgoldenrodyellow", 250, 250, 210},
            {"lightgray", 211, 211, 211},
            {"lightgreen", 144, 238, 144},
            {"lightgrey", 211, 211, 211},
            {"lightpink", 255, 182, 193},
            {"lightsalmon", 255, 160, 122},
            {"lightseagreen", 32, 178, 170},
            {"lightskyblue", 135, 206, 250},
            {"lightslategray", 119, 136, 153},
            {"lightslategrey", 119, 136, 153},
            {"lightsteelblue", 176, 196, 222},
            {"lightyellow", 255, 255, 224},
            {"lime", 0, 255, 0},
            {"limegreen", 50, 205, 50},
            {"linen", 250, 240, 230},
            {"magenta", 255, 0, 255},
            {"maroon", 128, 0, 0},
            {"mediumaquamarine", 102, 205, 170},
            {"mediumblue", 0, 0, 205},
            {"mediumorchid", 186, 85, 211},
            {"mediumpurple", 147, 112, 219},
            {"mediumseagreen", 60, 179, 113},
            {"mediumslateblue", 123, 104, 238},
            {"mediumspringgreen", 0, 250, 154},
            {"mediumturquoise", 72, 209, 204},
            {"mediumvioletred", 199, 21, 133},
            {"midnightblue", 25, 25, 112},
            {"mintcream", 245, 255, 250},
            {"mistyrose", 255, 228, 225},
            {"moccasin", 255, 228, 181},
            {"navajowhite", 255, 222, 173},
            {"navy", 0, 0, 128},
            {"oldlace", 253, 245, 230},
            {"olive", 128, 128, 0},
            {"olivedrab", 107, 142, 35},
            {"orange", 255, 165, 0},
            {"orangered", 255, 69, 0},
            {"orchid", 218, 112, 214},
            {"palegoldenrod", 238, 232, 170},
            {"palegreen", 152, 251, 152},
            {"paleturquoise", 175, 238, 238},
            {"palevioletred", 219, 112, 147},
            {"papayawhip", 255, 239, 213},
            {"peachpuff", 255, 218, 185},
            {"peru", 205, 133, 63},
            {"pink", 255, 192, 203},
            {"plum", 221, 160, 203},
            {"powderblue", 176, 224, 230},
            {"purple", 128, 0, 128},
            {"red", 255, 0, 0},
            {"rosybrown", 188, 143, 143},
            {"royalblue", 65, 105, 225},
            {"saddlebrown", 139, 69, 19},
            {"salmon", 250, 128, 114},
            {"sandybrown", 244, 164, 96},
            {"seagreen", 46, 139, 87},
            {"seashell", 255, 245, 238},
            {"sienna", 160, 82, 45},
            {"silver", 192, 192, 192},
            {"skyblue", 135, 206, 235},
            {"slateblue", 106, 90, 205},
            {"slategray", 119, 128, 144},
            {"slategrey", 119, 128, 144},
            {"snow", 255, 255, 250},
            {"springgreen", 0, 255, 127},
            {"steelblue", 70, 130, 180},
            {"tan", 210, 180, 140},
            {"teal", 0, 128, 128},
            {"thistle", 216, 191, 216},
            {"tomato", 255, 99, 71},
            {"turquoise", 64, 224, 208},
            {"violet", 238, 130, 238},
            {"wheat", 245, 222, 179},
            {"white", 255, 255, 255},
            {"whitesmoke", 245, 245, 245},
            {"yellow", 255, 255, 0},
            {"yellowgreen", 154, 205, 50}
        };

        if ( const ColorPair *res = std::lower_bound(color_list, color_list + sizeof (color_list) / sizeof (color_list[0]), str, ColorComp() ) ) {
            r = res->r ; g = res->g ; b = res->b ;
            return true ;
        }
        else return false ;
    }

    return false ;
}


CSSColor::CSSColor(const string &css_color_spec)
{
    if ( !parse_css_color(css_color_spec, r_, g_, b_) ) {
        stringstream strm ;
        strm << "invalid CSS color: " << css_color_spec ;
        throw CSSColorParseException(strm.str()) ;
    }
}

Color::Color(const CSSColor &clr, double alpha): r_(clr.r_/255.0), g_(clr.g_/255.0), b_(clr.b_/255.0), a_(clamp(alpha)) {
}

NamedColor NamedColor::alice_blue() noexcept { return {240, 248, 255} ; }
NamedColor NamedColor::antique_white() noexcept { return {250, 235, 215} ; }
NamedColor NamedColor::aqua() noexcept { return {0, 255, 255} ; }
NamedColor NamedColor::aqua_marine() noexcept { return {127, 255, 212} ; }
NamedColor NamedColor::azure() noexcept { return {240, 255, 255} ; }
NamedColor NamedColor::beige() noexcept { return {245, 245, 220} ; }
NamedColor NamedColor::bisque() noexcept { return {255, 228, 196} ; }
NamedColor NamedColor::black() noexcept { return {0, 0, 0} ; }
NamedColor NamedColor::blanched_almond() noexcept { return {255, 235, 205} ; }
NamedColor NamedColor::blue() noexcept { return {0, 0, 255} ; }
NamedColor NamedColor::blue_violet() noexcept { return {138, 43, 226} ; }
NamedColor NamedColor::brown() noexcept { return {165, 42, 42} ; }
NamedColor NamedColor::burlywood() noexcept { return {222, 184, 135} ; }
NamedColor NamedColor::cadet_blue() noexcept { return {95, 158, 160} ; }
NamedColor NamedColor::chartreuse() noexcept { return {127, 255, 0} ; }
NamedColor NamedColor::chocolate() noexcept { return {210, 105, 30} ; }
NamedColor NamedColor::coral() noexcept { return {255, 127, 80} ; }
NamedColor NamedColor::cornflower_blue() noexcept { return {100, 149, 237} ; }
NamedColor NamedColor::corn_silk() noexcept { return {255, 248, 220} ; }
NamedColor NamedColor::crimson() noexcept { return {220, 20, 60} ; }
NamedColor NamedColor::cyan() noexcept { return {0, 255, 255} ; }
NamedColor NamedColor::dark_blue() noexcept { return {0, 0, 139} ; }
NamedColor NamedColor::dark_cyan() noexcept { return {0, 139, 139} ; }
NamedColor NamedColor::dark_golden_rod() noexcept { return {184, 132, 11} ; }
NamedColor NamedColor::dark_gray() noexcept { return {169, 169, 169} ; }
NamedColor NamedColor::dark_green() noexcept { return {0, 100, 0} ; }
NamedColor NamedColor::dark_grey() noexcept { return {169, 169, 169} ; }
NamedColor NamedColor::dark_khaki() noexcept { return {189, 183, 107} ; }
NamedColor NamedColor::dark_magenta() noexcept { return {139, 0, 139} ; }
NamedColor NamedColor::dark_olive_green() noexcept { return {85, 107, 47} ; }
NamedColor NamedColor::dark_orange() noexcept { return {255, 140, 0} ; }
NamedColor NamedColor::dark_orchid() noexcept { return {153, 50, 204} ; }
NamedColor NamedColor::dark_red() noexcept { return {139, 0, 0} ; }
NamedColor NamedColor::dark_salmon() noexcept { return {233, 150, 122} ; }
NamedColor NamedColor::dark_sea_green() noexcept { return {143, 188, 143} ; }
NamedColor NamedColor::dark_slate_blue() noexcept { return {72, 61, 139} ; }
NamedColor NamedColor::dark_slate_gray() noexcept { return {47, 79, 79} ; }
NamedColor NamedColor::dark_slate_grey() noexcept { return {47, 79, 79} ; }
NamedColor NamedColor::dark_turquoise() noexcept { return {0, 206, 209} ; }
NamedColor NamedColor::dark_violet() noexcept { return {148, 0, 211} ; }
NamedColor NamedColor::deep_pink() noexcept { return {255, 20, 147} ; }
NamedColor NamedColor::deep_sky_blue() noexcept { return {0, 191, 255} ; }
NamedColor NamedColor::dim_gray() noexcept { return {105, 105, 105} ; }
NamedColor NamedColor::dim_grey() noexcept { return {105, 105, 105} ; }
NamedColor NamedColor::dodger_blue() noexcept { return {30, 144, 255} ; }
NamedColor NamedColor::fire_brick() noexcept { return {178, 34, 34} ; }
NamedColor NamedColor::floral_white() noexcept { return {255, 255, 240} ; }
NamedColor NamedColor::forest_green() noexcept { return {34, 139, 34} ; }
NamedColor NamedColor::fuchsia() noexcept { return {255, 0, 255} ; }
NamedColor NamedColor::gainsboro() noexcept { return {220, 220, 220} ; }
NamedColor NamedColor::ghostwhite() noexcept { return {248, 248, 255} ; }
NamedColor NamedColor::gold() noexcept { return {255, 215, 0} ; }
NamedColor NamedColor::goldenrod() noexcept { return {218, 165, 32} ; }
NamedColor NamedColor::gray() noexcept { return {128, 128, 128} ; }
NamedColor NamedColor::green() noexcept { return {0, 128, 0} ; }
NamedColor NamedColor::green_yellow() noexcept { return {173, 255, 47} ; }
NamedColor NamedColor::grey() noexcept { return {128, 128, 128} ; }
NamedColor NamedColor::honey_dew() noexcept { return {240, 255, 240} ; }
NamedColor NamedColor::hot_pink() noexcept { return {255, 105, 180} ; }
NamedColor NamedColor::indian_red() noexcept { return {205, 92, 92} ; }
NamedColor NamedColor::indigo() noexcept { return {75, 0, 130} ; }
NamedColor NamedColor::ivory() noexcept { return {255, 255, 240} ; }
NamedColor NamedColor::khaki() noexcept { return {240, 230, 140} ; }
NamedColor NamedColor::lavender() noexcept { return {230, 230, 250} ; }
NamedColor NamedColor::lavender_blush() noexcept { return {255, 240, 245} ; }
NamedColor NamedColor::lawn_green() noexcept { return {124, 252, 0} ; }
NamedColor NamedColor::lemon_chiffon() noexcept { return {255, 250, 205} ; }
NamedColor NamedColor::light_blue() noexcept { return {173, 216, 230} ; }
NamedColor NamedColor::light_coral() noexcept { return {240, 128, 128} ; }
NamedColor NamedColor::light_cyan() noexcept { return {224, 255, 255} ; }
NamedColor NamedColor::light_golden_rod_yellow() noexcept { return {250, 250, 210} ; }
NamedColor NamedColor::light_gray() noexcept { return {211, 211, 211} ; }
NamedColor NamedColor::light_green() noexcept { return {144, 238, 144} ; }
NamedColor NamedColor::light_grey() noexcept { return {211, 211, 211} ; }
NamedColor NamedColor::light_pink() noexcept { return {255, 182, 193} ; }
NamedColor NamedColor::light_salmon() noexcept { return {255, 160, 122} ; }
NamedColor NamedColor::light_sea_green() noexcept { return {32, 178, 170} ; }
NamedColor NamedColor::light_sky_blue() noexcept { return {135, 206, 250} ; }
NamedColor NamedColor::light_slate_gray() noexcept { return {119, 136, 153} ; }
NamedColor NamedColor::light_slate_grey() noexcept { return {119, 136, 153} ; }
NamedColor NamedColor::light_steel_blue() noexcept { return {176, 196, 222} ; }
NamedColor NamedColor::light_yellow() noexcept { return {255, 255, 224} ; }
NamedColor NamedColor::lime() noexcept { return {0, 255, 0} ; }
NamedColor NamedColor::limegreen() noexcept { return {50, 205, 50} ; }
NamedColor NamedColor::linen() noexcept { return {250, 240, 230} ; }
NamedColor NamedColor::magenta() noexcept { return {255, 0, 255} ; }
NamedColor NamedColor::maroon() noexcept { return {128, 0, 0} ; }
NamedColor NamedColor::medium_aqua_marine() noexcept { return {102, 205, 170} ; }
NamedColor NamedColor::medium_blue() noexcept { return {0, 0, 205} ; }
NamedColor NamedColor::medium_orchid() noexcept { return {186, 85, 211} ; }
NamedColor NamedColor::medium_purple() noexcept { return {147, 112, 219} ; }
NamedColor NamedColor::medium_sea_green() noexcept { return {60, 179, 113} ; }
NamedColor NamedColor::medium_slate_blue() noexcept { return {123, 104, 238} ; }
NamedColor NamedColor::medium_spring_green() noexcept { return {0, 250, 154} ; }
NamedColor NamedColor::medium_turquoise() noexcept { return {72, 209, 204} ; }
NamedColor NamedColor::medium_violet_red() noexcept { return {199, 21, 133} ; }
NamedColor NamedColor::midnight_blue() noexcept { return {25, 25, 112} ; }
NamedColor NamedColor::mint_cream() noexcept { return {245, 255, 250} ; }
NamedColor NamedColor::misty_rose() noexcept { return {255, 228, 225} ; }
NamedColor NamedColor::moccasin() noexcept { return {255, 228, 181} ; }
NamedColor NamedColor::navajo_white() noexcept { return {255, 222, 173} ; }
NamedColor NamedColor::navy() noexcept { return {0, 0, 128} ; }
NamedColor NamedColor::old_lace() noexcept { return {253, 245, 230} ; }
NamedColor NamedColor::olive() noexcept { return {128, 128, 0} ; }
NamedColor NamedColor::olive_drab() noexcept { return {107, 142, 35} ; }
NamedColor NamedColor::orange() noexcept { return {255, 165, 0} ; }
NamedColor NamedColor::orangered() noexcept { return {255, 69, 0} ; }
NamedColor NamedColor::orchid() noexcept { return {218, 112, 214} ; }
NamedColor NamedColor::pale_golden_rod() noexcept { return {238, 232, 170} ; }
NamedColor NamedColor::pale_green() noexcept { return {152, 251, 152} ; }
NamedColor NamedColor::pale_turquoise() noexcept { return {175, 238, 238} ; }
NamedColor NamedColor::pale_violet_red() noexcept { return {219, 112, 147} ; }
NamedColor NamedColor::papaya_whip() noexcept { return {255, 239, 213} ; }
NamedColor NamedColor::peach_puff() noexcept { return {255, 218, 185} ; }
NamedColor NamedColor::peru() noexcept { return {205, 133, 63} ; }
NamedColor NamedColor::pink() noexcept { return {255, 192, 203} ; }
NamedColor NamedColor::plum() noexcept { return {221, 160, 203} ; }
NamedColor NamedColor::powderblue() noexcept { return {176, 224, 230} ; }
NamedColor NamedColor::purple() noexcept { return {128, 0, 128} ; }
NamedColor NamedColor::red() noexcept { return {255, 0, 0} ; }
NamedColor NamedColor::rosy_brown() noexcept { return {188, 143, 143} ; }
NamedColor NamedColor::royal_blue() noexcept { return {65, 105, 225} ; }
NamedColor NamedColor::saddle_brown() noexcept { return {139, 69, 19} ; }
NamedColor NamedColor::salmon() noexcept { return {250, 128, 114} ; }
NamedColor NamedColor::sandybrown() noexcept { return {244, 164, 96} ; }
NamedColor NamedColor::seagreen() noexcept { return {46, 139, 87} ; }
NamedColor NamedColor::seashell() noexcept { return {255, 245, 238} ; }
NamedColor NamedColor::sienna() noexcept { return {160, 82, 45} ; }
NamedColor NamedColor::silver() noexcept { return {192, 192, 192} ; }
NamedColor NamedColor::sky_blue() noexcept { return {135, 206, 235} ; }
NamedColor NamedColor::slate_blue() noexcept { return {106, 90, 205} ; }
NamedColor NamedColor::slate_gray() noexcept { return {119, 128, 144} ; }
NamedColor NamedColor::slate_grey() noexcept { return {119, 128, 144} ; }
NamedColor NamedColor::snow() noexcept { return {255, 255, 250} ; }
NamedColor NamedColor::spring_green() noexcept { return {0, 255, 127} ; }
NamedColor NamedColor::steel_blue() noexcept { return {70, 130, 180} ; }
NamedColor NamedColor::tan() noexcept { return {210, 180, 140} ; }
NamedColor NamedColor::teal() noexcept { return {0, 128, 128} ; }
NamedColor NamedColor::thistle() noexcept { return {216, 191, 216} ; }
NamedColor NamedColor::tomato() noexcept { return {255, 99, 71} ; }
NamedColor NamedColor::turquoise() noexcept { return {64, 224, 208} ; }
NamedColor NamedColor::violet() noexcept { return {238, 130, 238} ; }
NamedColor NamedColor::wheat() noexcept { return {245, 222, 179} ; }
NamedColor NamedColor::white() noexcept { return {255, 255, 255} ; }
NamedColor NamedColor::white_smoke() noexcept { return {245, 245, 245} ; }
NamedColor NamedColor::yellow() noexcept { return {255, 255, 0} ; }
NamedColor NamedColor::yellow_green() noexcept { return {154, 205, 50} ; }

double Color::clamp(double value) {
    return std::max(0.0, std::min(1.0, value));
}

}











