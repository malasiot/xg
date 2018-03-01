#ifndef __XG_SVG_LENGTH_HPP__
#define __XG_SVG_LENGTH_HPP__

#include <string>
#include <xg/util/dictionary.hpp>

namespace xg {
namespace svg {

enum class LengthUnitType { Unknown, Number, EMS, EXS, PX, CM, MM, IN, PT, PC, Percentage } ;
enum class LengthDirection { Absolute, Horizontal, Vertical, Viewport } ;

class Length
{
  public:

    Length(): unit_type_(LengthUnitType::Unknown), value_in_specified_units_(0) {}
    Length(float val, LengthUnitType unit = LengthUnitType::Number): unit_type_(unit), value_in_specified_units_(val) {}

    static Length fromString(const std::string &str) {
        Length l ;
        l.parse(str) ;
        return l ;
    }

    void parse(const std::string &str)  ;

    static std::vector<Length> parseList(const std::string &str) ;
 //   double toPixels(RenderingContext *ctx, Direction dir = AbsoluteDir) const ;

    bool unknown() const { return unit_type_ == LengthUnitType::Unknown ; }

    float value() const { return value_in_specified_units_ ; }

    LengthUnitType units() const { return unit_type_ ; }

    LengthUnitType unit_type_ ;
    float value_in_specified_units_ ;
};

inline Length operator "" _cm( long double v ) {
  return Length{(float)v, LengthUnitType::CM};
}

inline Length operator "" _perc( long double v ) {
  return Length{(float)v, LengthUnitType::Percentage};
}

inline Length operator "" _px( long double v ) {
  return Length{(float)v, LengthUnitType::PX };
}

} // namespace svg


} //namespace xg




#endif

