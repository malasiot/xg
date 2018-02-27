#ifndef __XG_RECTANGLE_HPP__
#define __XG_RECTANGLE_HPP__

#include <xg/vector.hpp>

namespace xg {

class Rectangle2d
{
protected:

    double x_, y_, width_, height_ ;

public:

    Rectangle2d(double x, double y, double w, double h): x_(x), y_(y), width_(w), height_(h) {
        normalize() ;
    }

    Rectangle2d(const Vector2d &tl, const Vector2d &br) {
        x_ = tl.x() ; y_ = tl.y() ;
        width_ = br.x() - x_ ;
        height_ = br.y() - y_ ;
    }

    Point2d topLeft() const { return { x_, y_ } ; }
    Point2d bottomRight() const { return { x_ + width_, y_ + height_ } ; }
    Point2d bottomLeft() const { return { x_, y_ + height_ } ; }
    Point2d topRight() const { return { x_ + width_, y_ } ; }
    Point2d center() const { return { x_ + width_/2, y_ + height_/2} ; }

    double width() const { return width_ ; }
    double height() const { return height_ ; }
    double &width() { return width_ ; }
    double &height() { return height_ ; }

    double x() const { return x_ ; }
    double y() const { return y_ ; }
    double &x() { return x_ ; }
    double &y() { return y_ ; }



private:

    void normalize() {
        double tlx = std::min(x_, x_ + width_) ;
        double tly = std::min(y_, y_ + height_) ;
        double brx = std::max(x_, x_ + width_) ;
        double bry = std::max(y_, y_ + height_) ;

        x_ = tlx ; y_ = tly ;
        width_ = brx - tlx ; height_ = bry - tly ;
    }

};











}
#endif
