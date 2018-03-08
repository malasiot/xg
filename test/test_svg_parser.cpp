#include <xg/canvas.hpp>

#include <fstream>
#include <iostream>

#include <xg/util/variant.hpp>

using namespace xg ;
using namespace std ;

int main(int argc, char *argv[]) {

    SVGDocument doc ;

    ifstream strm("/home/malasiot/Downloads/pattern03.svg") ;
    try {
        doc.readStream(strm) ;

        ImageCanvas canvas(1024, 512) ;

        canvas.setBrush(SolidBrush(NamedColor::white())) ;
        canvas.drawRect(0, 0, 1024, 512) ;
        canvas.drawSVG(doc) ;

        canvas.getImage().saveToPNG("/tmp/oo.png") ;

    }
    catch ( SVGLoadException &e ) {

        cout << e.what() << endl ;
    }

}
