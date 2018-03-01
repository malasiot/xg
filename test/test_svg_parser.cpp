#include <xg/svg_document.hpp>
#include <fstream>
#include <iostream>

#include <xg/util/variant.hpp>

using namespace xg ;
using namespace std ;

int main(int argc, char *argv[]) {

    SVGDocument doc ;

    ifstream strm("/home/malasiot/Downloads/pencil.svg") ;
    try {
        doc.readStream(strm) ;
    }
    catch ( SVGLoadException &e ) {

        cout << e.what() << endl ;
    }

}
