#include <xg/canvas.hpp>

using namespace xg ;
using namespace std ;

int main(int argc, char *argv[]) {

    ImageCanvas canvas(1024, 512) ;

    canvas.setPen(Pen()) ;

    Font f("Arial,serif", 12) ;
    f.setStyle(FontStyle::Italic) ;
    canvas.setFont(f) ;
    canvas.drawRect(40, 100, 100, 100) ;
    canvas.drawLine(0, 90, 200, 90) ;
    canvas.setBrush(SolidBrush(NamedColor::alice_blue())) ;
   // string s("Win is the mother of battles بسم الله الرحمن الرحيم") ;
   // string s("Win is \nthe mother of battles") ;

    string s("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.") ;
    canvas.drawRect(40, 100, 200, 400) ;
    canvas.drawText(s, 40, 100, 200, 400, TextAlignTop | TextAlignHCenter) ;

    canvas.saveToPng("/tmp/oo.png") ;

}
