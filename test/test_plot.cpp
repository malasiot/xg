#include <xg/canvas.hpp>
#include <xg/image.hpp>

using namespace xg ;
using namespace std ;

int main(int argc, char *argv[]) {

    Image im(256, 256) ;
    string buffer ;
    im.saveToPNGBuffer(buffer) ;
    ImageCanvas canvas(1024, 512) ;

    canvas.setPen(Pen()) ;

    Font f("Amiri,serif", 12) ;
    f.setStyle(FontStyle::Italic) ;
    canvas.setFont(f) ;
    canvas.drawRect(40, 100, 100, 100) ;
    canvas.drawLine(0, 90, 200, 90) ;
    canvas.setBrush(SolidBrush(NamedColor::alice_blue())) ;
    string s("Win is the mother of battles بسم الله الرحمن الرحيم") ;
   // string s("Win is \nthe mother of battles") ;

    double w = 100 ;
    //string s("Σπανακόρυζο λεμονάτο και σπυρωτό. Μια παραδοσιακή κλασική συνταγή. Εύκολο, γρήγορο, οικονομικό, υγιεινό και νηστίσιμο!") ;
    canvas.drawRect(40, 100, w, 400) ;
    canvas.drawText(s, 40, 100, w, 400, TextAlignTop | TextAlignRight) ;

    canvas.getImage().saveToPNG("/tmp/oo.png") ;

}
