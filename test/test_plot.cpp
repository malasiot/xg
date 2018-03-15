#include <xg/canvas.hpp>
#include <xg/image.hpp>
#include <xg/vector.hpp>

using namespace xg ;
using namespace std ;

int main(int argc, char *argv[]) {

    Image im(256, 256) ;
    string buffer ;
    im.saveToPNGBuffer(buffer) ;
    ImageCanvas canvas(1024, 512) ;

    canvas.setPen(Pen()) ;

    Font f("Amiri", 32) ;
    f.setStyle(FontStyle::Italic) ;
    canvas.setFont(f) ;

    canvas.setBrush(SolidBrush(NamedColor::alice_blue())) ;
    string s("Win is the mother of battles بسم الله الرحمن الرحيم") ;
   // string s("Win is \nthe mother of battles") ;
/*
    Matrix2d tr ;
    tr.translate(Vector2d(-40, -100)) ;
    tr.scale(0.5, 0.5) ;
    tr.rotate(30 * M_PI/180) ;
    tr.translate(Vector2d(40, 100)) ;

    canvas.setTransform(tr);
*/
    double w = 5000 ;
    //string s("Σπανακόρυζο λεμονάτο και σπυρωτό. Μια παραδοσιακή κλασική συνταγή. Εύκολο, γρήγορο, οικονομικό, υγιεινό και νηστίσιμο!") ;
    canvas.drawRect(40, 100, w, 400) ;
  canvas.drawText(s, 40, 100, w, 400, TextAlignBottom | TextAlignLeft) ;
 //   canvas.drawText(s, 40, 100, w, 400, TextAlignTop | TextAlignRight) ;

    canvas.drawText(s, 40, 100) ;

    canvas.getImage().saveToPNG("/tmp/oo.png") ;

}
