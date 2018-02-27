#include <xg/canvas.hpp>
#include <xg/image.hpp>
#include <xg/vector.hpp>

using namespace xg ;
using namespace std ;

void drawShape(Canvas &canvas, const Rectangle2d &rect) {

    canvas.drawRect(rect) ;
    canvas.drawLine(rect.topLeft(), rect.bottomRight()) ;
    canvas.drawLine(rect.bottomLeft(), rect.topRight()) ;
}

int main(int argc, char *argv[]) {

    ImageCanvas canvas(1024, 512) ;

    canvas.setPen(Pen()) ;
    canvas.setBrush(SolidBrush(NamedColor::blue())) ;

    Rectangle2d rect(0, 0, 100, 100) ;

    canvas.setTransform(Matrix2d::translation({50, 50})) ;

    double angle = 0 ;
    for( double x = 0 ; x < 1024 ; x += 200 )
        for( double y = 0 ; y<512 ; y += 200 ) {

            angle += 10 * M_PI/180 ;

            Matrix2d tr ;
            tr.rotate(angle, rect.center(), 2 ) ;
            tr.translate({x, y}) ;

            canvas.save() ;

            canvas.setTransform(tr) ;

            canvas.setBrush(SolidBrush({0.1, 0.3, 0.5})) ;

            drawShape(canvas, rect) ;

            canvas.restore() ;
        }

    canvas.getImage().saveToPNG("/tmp/oo.png") ;

}
