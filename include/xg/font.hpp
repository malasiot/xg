#ifndef __XPLOT_FONT_HPP__
#define __XPLOT_FONT_HPP__

#include <string>

namespace xg {

enum FontStyle { NormalFontStyle, ObliqueFontStyle, ItalicFontStyle } ;
enum FontWeight { NormalFontWeight, BoldFontWeight } ;

class Font {
public:

    Font(const std::string &family_name, double pts): family_(family_name), sz_(pts),
        style_(NormalFontStyle),  weight_(NormalFontWeight) {}

    Font & setStyle(FontStyle style) { style_ = style  ; return *this ; }
    Font & setWeight(FontWeight weight) { weight_ = weight ; return *this ;}
    Font & setSize(double pts) { sz_ = pts ; return *this ; }
    Font & setFamily(const std::string &family_name ) { family_ = family_name ; return *this ; }

    FontStyle style() const { return style_ ; }
    FontWeight weight() const { return weight_ ; }
    double size() const { return sz_ ; }
    const std::string family() const { return family_ ; }

private:


    FontStyle style_ ;
    FontWeight weight_ ;
    double sz_ ;
    std::string family_ ;
} ;


} // namespace xplot ;

#endif
