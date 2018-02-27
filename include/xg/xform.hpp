#ifndef __XG_XFORM_HPP__
#define __XG_XFORM_HPP__

#include <xg/vector.hpp>

namespace xg {

class Matrix2d {

    // non-zero elements of 3x3 affine matrix transpose
    double m1_ = 1.0, m2_ = 0.0, m3_ = 0.0, m4_ = 1.0, m5_ = 0.0, m6_ = 0.0 ;
    /*
    [ m1 m2 0 ]
    [ m3 m4 0 ]
    [ m5 m6 1  ]
    */
public:

    Matrix2d() = default ;

    Matrix2d(double m1, double m2, double m3, double m4, double m5, double m6):
        m1_(m1), m2_(m2), m3_(m3), m4_(m4), m5_(m5), m6_(m6) {
    }

    static Matrix2d identity() {
        return { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 } ;
    }

    static Matrix2d translation(const Vector2d &v) {
        return { 1.0, 0.0, 0.0, 1.0, v.x(), v.y() } ;
    }

    static Matrix2d rotation(double radians) {
        double s = sin(radians) ;
        double c = cos(radians) ;
        return { c, s, -s, c, 0.0, 0.0 } ;
    }

    static Matrix2d scaling(double sx, double sy) {
        return { sx, 0, 0, sy, 0.0, 0.0 } ;
    }

    Matrix2d& translate(const Vector2d& value) {
        m5_ += value.x() ; m6_ += value.y() ;
        return *this ;
    }

    Matrix2d &rotate(double angle)
    {
        double c, s, r1, r2, r3, r4, r5, r6;

        c = cos(angle); s = sin(angle);

        r1 = m1_*c; r2 = m2_*s; r3 = m3_*c;
        r4 = m4_*s; r5 = m5_*c; r6 = m6_*s;

        m2_ = m1_*s + m2_*c;
        m4_ = m3_*s + m4_*c;
        m6_ = m5_*s + m6_*c;
        m1_ = r1 - r2;
        m3_ = r3 - r4;
        m5_ = r5 - r6;

        return *this ;
    }

    Matrix2d &scale(double sx, double sy) {
        m1_ *= sx; m2_ *= sy; m3_ *= sx;
        m4_ *= sy; m5_ *= sx; m6_ *= sy;
        return *this ;
    }

    Matrix2d &skew(double sx, double sy) {
        m2_ += m1_*sy;
        m3_ += m4_*sx;
        return *this ;
    }

    Matrix2d &premult(const Matrix2d &other)
    {
        double tmp1 = m1_, tmp2 = m3_ ;

        m1_  = other.m1_ * tmp1 + other.m2_ * tmp2 ;
        m3_  = other.m3_ * tmp1 + other.m4_ * tmp2 ;
        m5_ += other.m5_ * tmp1 + other.m6_ * tmp2 ;

        tmp1 = m2_ ; tmp2 = m4_ ;

        m2_  = other.m1_ * tmp1 + other.m2_ * tmp2 ;
        m4_  = other.m3_ * tmp1 + other.m4_ * tmp2 ;
        m6_ += other.m5_ * tmp1 + other.m6_ * tmp2 ;

        return *this ;
    }

    Matrix2d &postmult(const Matrix2d &other)
    {
        double tmp ;

        tmp = m1_*other.m2_ + m2_*other.m4_;
        m1_  = m1_*other.m1_ + m2_*other.m3_;
        m2_  = tmp;

        tmp = m3_*other.m2_ + m4_*other.m4_;
        m3_  = m3_*other.m1_ + m4_*other.m3_;
        m4_  = tmp;

        tmp = m5_*other.m2_ + m6_*other.m4_;
        m5_  = m5_*other.m1_ + m6_*other.m3_;
        m6_  = tmp;

        m5_ += other.m5_;
        m6_ += other.m6_;

        return *this ;
    }

    Matrix2d& invert() {
        double d = determinant();
        double t1 = m1_, t5 = m5_ ;

        assert( d != 0.0 ) ;

        m5_ = (m3_*m6_ - m4_*m5_)/d;
        m6_ = (m2_*t5 - m1_*m6_)/d;
        m1_ = m4_/d;
        m4_ = t1/d;
        m3_ = -m3_/d;
        m2_ = -m2_/d;
    }

    double m1() const { return m1_ ; }
    double m2() const { return m2_ ; }
    double m3() const { return m3_ ; }
    double m4() const { return m4_ ; }
    double m5() const { return m5_ ; }
    double m6() const { return m6_ ; }

    double &m1() { return m1_ ; }
    double &m2() { return m2_ ; }
    double &m3() { return m3_ ; }
    double &m4() { return m4_ ; }
    double &m5() { return m5_ ; }
    double &m6() { return m6_ ; }

    bool is_invertible() const { return determinant() != 0.0 ; }
    double determinant() const { return m1_*m4_ - m2_*m3_ ; }

    Vector2d transform(const Vector2d &v) {
        return { v.x()*m1_ + v.y()*m3_ + m5_, v.x()*m2_ + v.y()*m4_ + m6_ } ;
    }


    Matrix2d& operator*=(const Matrix2d& rhs) {
        return premult(rhs) ;
    }
};

Matrix2d operator*(const Matrix2d& lhs, const Matrix2d& rhs) noexcept;
bool operator==(const Matrix2d& lhs, const Matrix2d& rhs) noexcept;
bool operator!=(const Matrix2d& lhs, const Matrix2d& rhs) noexcept;


} // namespace xg

#endif
