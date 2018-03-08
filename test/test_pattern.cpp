#include <xg/canvas.hpp>
#include <xg/image.hpp>
#include <xg/vector.hpp>

using namespace xg ;
using namespace std ;


int main(int argc, char *argv[]) {

    auto pattern = std::make_shared<PatternCanvas>(64, 64) ;

    pattern->setPen(Pen()) ;
    pattern->setBrush(SolidBrush(NamedColor::blue())) ;

    pattern->drawCircle(32, 32, 32) ;

  //  pattern.getImage().saveToPNG("/tmp/pattern.png") ;

    ImageCanvas canvas(512, 512) ;

    PatternBrush brush(pattern) ;
    brush.setSpread(SpreadMethod::Repeat) ;
    brush.setTransform(Matrix2d::rotation(10*M_PI/180)) ;
    canvas.setBrush(brush) ;
    canvas.drawRect(10, 10, 500, 500) ;

    canvas.getImage().saveToPNG("/tmp/canvas.png") ;

}
