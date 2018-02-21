#include <xg/canvas.hpp>

using namespace xg ;
using namespace std ;

int main(int argc, char *argv[]) {

    ImageCanvas canvas(512, 512) ;

    canvas.setPen(Pen()) ;
    canvas.setFont(Font("cursive", 25)) ;
    canvas.drawRect(40, 40, 100, 100) ;
    canvas.drawLine(0, 90, 200, 90) ;
    canvas.setBrush(SolidBrush(NamedColor::alice_blue())) ;
    string s("When the Arabic letter Lam (ل) is followed \nby the letter Alif (ا), \nFriBidi substitutes the two with the ligature Lam+Alif (لا) and a zero-width space is inserted.") ;
    canvas.drawText(s, 40, 40, 100, 100, TextAlignTop | TextAlignHCenter) ;
    canvas.saveToPng("/tmp/oo.png") ;

}
