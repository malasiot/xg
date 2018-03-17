#include "svg_length.hpp"
#include "svg_dom_exceptions.hpp"
using namespace std ;

namespace xg {
namespace svg {

static bool parse_units(char *&c, LengthUnitType &units) {
    switch ( *c ) {
    case 0:
        units = LengthUnitType::Number ;
        return true ;
    case 'e':
        c ++ ;
        switch ( *c ) {
        case 'm':
            units = LengthUnitType::EMS ;
            break ;
        case 'x':
            units = LengthUnitType::EXS ;
            break ;
        default:
            return false ;
        }
        break ;
    case 'p':
        c ++ ;
        switch ( *c ) {
        case 't':
            units = LengthUnitType::PT ;
            break ;
        case 'x':
            units = LengthUnitType::PX ;

            break ;
        case 'c':
            units = LengthUnitType::PC ;
            break ;
        default:
            return false ;
        }
        break ;
    case 'c':
        c ++ ;
        switch ( *c ) {
        case 'm':
            units = LengthUnitType::CM ;
            break ;
        default:
            return false ;
        }
        break ;
    case 'i':
        c ++ ;
        switch ( *c ) {
        case 'n':
            units = LengthUnitType::IN ;
            break ;
        default:
            return false ;
        }
        break ;
    case 'm':
        c ++ ;
        switch ( *c ) {
        case 'm':
            units = LengthUnitType::MM ;
            break ;
        default:
            return false ;
        }
        break ;
    case '%':
        units = LengthUnitType::Percentage ;
        break ;
    }

    ++c ;

    return true ;
}

bool Length::parse(const std::string &str)  {

    if ( str.empty() ) return false ;

    const char *c_start = str.c_str() ;

    char *c_end ;
    value_in_specified_units_ = strtof(c_start, &c_end) ;

    // failed to parse number
    if ( c_end == c_start )  {
        return false ;
    }
    while ( isspace(*c_end) ) ++c_end ;

    unit_type_ = LengthUnitType::Unknown ;

    if ( ! parse_units(c_end, unit_type_) ) return false ;

    return true ;
}

bool LengthList::parse(const string &str)
{
    if ( str.empty() ) return true;

    const char *c_start = str.c_str() ;

    char *c_end ;

    do {
        float value = strtof(c_start, &c_end) ;

        // failed to parse number
        if ( c_end == c_start )  return false ;

        LengthUnitType unit_type = LengthUnitType::Number ;

        if ( ! parse_units(c_end, unit_type) )  return false ;

        values_.emplace_back(value, unit_type) ;

        while ( *c_end && ( isspace(*c_end) || *c_end == ',' ) ) ++c_end ;

        c_start = c_end ;
    } while ( *c_end != 0 ) ;

    return true ;
}


}
}
