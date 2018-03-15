#include <xg/canvas.hpp>
#include <xg/text_layout.hpp>

#include <fstream>
#include <iostream>

using namespace xg ;
using namespace std ;

int main(int argc, char *argv[]) {

    string text("This is a very short string") ;
    Font font("Arial", 64) ;

    TextLayout layout(text, font) ;
    layout.setWrapWidth(500) ;
    layout.setTextDirection(TextDirection::Auto);

    layout.compute() ;


   ImageCanvas canvas(1024, 512) ;

   canvas.setBrush(SolidBrush(Color(NamedColor::white(), 1.0))) ;
   canvas.drawRect(0, 0, 1024, 512) ;


   canvas.getImage().saveToPNG("/tmp/oo.png") ;

}
