#include "svg_parse_util.hpp"

#include <cmath>

#include <xg/util/strings.hpp>

using namespace std ;

namespace xg {

bool parse_number(const std::string &s, float &val) {
    const char *c_start = s.c_str() ;

    char *c_end ;
    val = strtof(c_start, &c_end) ;

    if ( c_end == c_start )  return false ;
    if ((val == -HUGE_VAL || val == HUGE_VAL) && (ERANGE == errno)) return false ;

    return true ;
}
/*
bool parse_coordinate_list(const std::string &p, vector<float> &args)
{
    auto tokens = split(p, ", ") ;

    for ( const string &s: tokens ) {

        float val ;
        if ( !parse_number(s, val) ) return false ;
        else args.emplace_back(val) ;
    }

    return true ;
}
*/

bool parse_coordinate_list(const std::string &s, vector<float> &args)
{
    const char *c_start = s.c_str() ;
    char *c_end ;

    while (1) {
        float val = strtof(c_start, &c_end) ;

        if ( c_end == c_start )  return false ;
        if ((val == -HUGE_VAL || val == HUGE_VAL) && (ERANGE == errno)) return false ;

        args.push_back(val) ;

        c_start = c_end ;

        while ( *c_start == ' ' || *c_start == ',' ) ++c_start ;

        if ( *c_start == 0 ) break ;
    }

    return true ;
}


bool parse_coordinate_list(const std::string &p, float &arg1, float &arg2)
{
    vector<float> args ;
    if ( !parse_coordinate_list(p, args) || args.size() != 2 ) return false ;
    else {
        arg1 = args[0] ; arg2 = args[1] ;
        return true ;
    }
}

bool parse_coordinate_list(const std::string &p, float &arg1, float &arg2, float &arg3, float &arg4)
{
    vector<float> args ;
    if ( !parse_coordinate_list(p, args) || args.size() != 4 ) return false ;
    else {
        arg1 = args[0] ; arg2 = args[1] ; arg3 = args[2] ; arg4 = args[3] ;
        return true ;
    }
}


bool parse_coordinate_list(const string &p, float &arg1, float &arg2, float &arg3, float &arg4, float &arg5, float &arg6)
{
    vector<float> args ;
    if ( !parse_coordinate_list(p, args) || args.size() != 6 ) return false ;
    else {
        arg1 = args[0] ; arg2 = args[1] ; arg3 = args[2] ; arg4 = args[3] ;
        arg5 = args[4] ; arg6 = args[5] ;
        return true ;
    }
}

bool parse_coordinate_list(const std::string &p, float &arg1, float &arg2, float &arg3, float &arg4,
                           float &arg5, float &arg6, float &arg7)
{
    vector<float> args ;
    if ( !parse_coordinate_list(p, args) || args.size() != 7 ) return false ;
    else {
        arg1 = args[0] ; arg2 = args[1] ; arg3 = args[2] ; arg4 = args[3] ;
        arg5 = args[4] ; arg6 = args[5] ; arg7 = args[6] ;
        return true ;
    }
}

string parse_uri(const string &src)
{
    static regex rx_url("[\\s]*url[\\s]*\\([\\s]*([^)]+)[\\s]*\\)[\\s]*") ;
    smatch what ;

    if ( regex_match(src, what, rx_url) )
        return what.str(1) ;
    else return string() ;

}


void eat_white(const char *&p)
{
    while (*p != 0 && isspace(*p) ) ++p ;
}

void eat_white_comma(const char *&p)
{
    while (*p != 0 && ( isspace(*p) || *p == ',' ) ) ++p ;
}


}
