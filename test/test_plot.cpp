#include <xg/canvas.hpp>

using namespace xg ;
using namespace std ;

int main(int argc, char *argv[]) {

    ImageCanvas canvas(1024, 512) ;

    canvas.setPen(Pen()) ;
    canvas.setFont(Font("Arial,serif", 32)) ;
    canvas.drawRect(40, 100, 100, 100) ;
    canvas.drawLine(0, 90, 200, 90) ;
    canvas.setBrush(SolidBrush(NamedColor::alice_blue())) ;
    string s("Win بسم الله الرحمن الرحيم") ;
    canvas.drawText(s, 40, 100, 100, 100, TextAlignTop | TextAlignHCenter) ;
    canvas.saveToPng("/tmp/oo.png") ;

}
