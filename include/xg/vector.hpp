#ifndef __XG_VECTOR_HPP__
#define __XG_VECTOR_HPP__

#include <limits>
#include <cassert>
#include <cmath>

namespace xg {

class Vector2d {
public:
    Vector2d(double x, double y): x_(x), y_(y) {}

    double x() const { return x_ ; }
    double y() const { return y_ ; }

    double &x() { return x_ ; }
    double &y() { return y_ ; }

    double length() const { return sqrt(lengthSquared()) ; }
    double lengthSquared() const { return x_ * x_ + y_ * y_ ; }
    double dot(const Vector2d& other) const { return x_ * other.x_ + y_ * other.y_ ; }

    void normalize() {
        double l = length() ;
        if ( fabs(l) > std::numeric_limits<double>::epsilon() ) {
            x_ /= l ; y_ /= l ;
        }
    }

    Vector2d normalized() const {
        double l = length() ;
        if ( fabs(l) > std::numeric_limits<double>::epsilon() ) {
            return { x_ / l, y_ / l } ;
        }
        else return *this ;
    }

    Vector2d & operator +=(const Vector2d& rhs) {
        x_ += rhs.x_ ;
        y_ += rhs.y_ ;
        return *this ;
    }

    Vector2d & operator -=(const Vector2d& rhs) {
        x_ -= rhs.x_ ;
        y_ -= rhs.y_ ;
        return *this ;
    }

    Vector2d & operator *=(double rhs) noexcept {
        x_ *= rhs ;
        y_ *= rhs ;
        return *this ;
    }

    Vector2d & operator /=(double rhs) noexcept {
        x_ /= rhs ;
        y_ /= rhs ;
        return *this ;
    }

    friend Vector2d operator -(const Vector2d &v) {
        return {-v.x_, -v.y_} ;
    }

    friend Vector2d operator +(const Vector2d &v1, const Vector2d &v2) {
        return {v1.x_ + v2.x_, v1.y_ + v2.y_} ;
    }

    friend Vector2d operator -(const Vector2d &v1, const Vector2d &v2) {
        return {v1.x_ - v2.x_, v1.y_ - v2.y_} ;
    }

    friend Vector2d operator *(const Vector2d &v1, double scalar) {
        return {v1.x_ * scalar, v1.y_ * scalar} ;
    }

    friend Vector2d operator *(double scalar, const Vector2d &v1) {
        return v1 * scalar ;
    }

    friend Vector2d operator /(const Vector2d &v1, double scalar) {
        return {v1.x_ / scalar, v1.y_ /scalar} ;
    }


    double &operator[] (unsigned int i) {
        assert(i < 2) ;
        return ( i == 0 ) ? x_ : y_ ;
    }

    const double &operator[] (int i) const {
        assert(i < 2) ;
        return ( i == 0 ) ? x_ : y_ ;
    }


private:

    double x_ = 0.0, y_ = 0.0 ;
} ;

typedef Vector2d Point2d ;

}
#endif
