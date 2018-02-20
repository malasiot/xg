#include <xg/canvas.hpp>

using namespace xg ;

int main(int argc, char *argv[]) {

    ImageCanvas canvas(512, 512) ;

    canvas.setPen(Pen()) ;
    canvas.setFont(Font("DejaVuSerif", 25)) ;
    canvas.drawRect(40, 40, 100, 100) ;
    canvas.drawLine(0, 90, 200, 90) ;
    canvas.setBrush(SolidBrush(NamedColor::alice_blue())) ;
    canvas.drawText("-0.1y", 40, 40, 100, 100, TextAlignTop | TextAlignHCenter) ;
    canvas.saveToPng("/tmp/oo.png") ;

}
