#include "svg_parse_util.hpp"

#include <cmath>

#include <xg/util/strings.hpp>
#include "svg_dom_exceptions.hpp"

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

static bool parse_number_list(const char *&p, vector<float> &numbers)
{
    eat_white(p) ;
    if ( *p++ != '(' )
        return false ;

    const char *start = p ;
    while ( *p && *p != ')' ) ++p ;

    string s(start, p) ;
    if ( !parse_coordinate_list(s, numbers) )
        return false ;

    ++p ;

    return true ;
}

bool parse_transform(const string &str, Matrix2d &res)
{

    const char *p = str.c_str() ;
    eat_white(p) ;

    while ( *p ) {
        vector<float> nums ;
         if ( strncmp(p, "matrix", 6) == 0 ) {
             p += 6 ;

             if ( !parse_number_list(p, nums) ) return false ;

             if ( nums.size() >= 6 ) {
                 Matrix2d m(nums[0], nums[1], nums[2], nums[3], nums[4], nums[5]) ;
                 res.premult(m) ;
             }
         }
         else if ( strncmp(p, "translate", 9) == 0 ) {
             p += 9 ;

             if ( !parse_number_list(p, nums) ) return false ;

             if ( nums.size() >= 2 )
                 res.translate(nums[0], nums[1]) ;
             else if ( nums.size() == 1 )
                 res.translate(nums[0], 0.0) ;
         }
         else if ( strncmp(p, "rotate", 6) == 0 )  {
             p += 6 ;

             if ( !parse_number_list(p, nums) ) return false ;

             if ( nums.size() == 1 )
                 res.rotate(nums[0]) ;
             else if ( nums.size() == 3 )
                 res.rotate(nums[0], Point2d(nums[1], nums[2])) ;

         }
         else if ( strncmp(p, "scale", 5) == 0 )  {
             p += 5 ;

             if ( !parse_number_list(p, nums) ) return false ;

             if ( nums.size() == 1 )
                 res.scale(nums[0], nums[0]) ;
             else if ( nums.size() >= 2 )
                 res.scale(nums[0], nums[1]) ;
         }
         else if ( strncmp(p, "skewX", 5) == 0 ) {
             p += 5 ;

             if ( !parse_number_list(p, nums) ) return false ;

             if ( nums.size() >= 1 )
                 res.skew(nums[0], 0.0) ;
         }
         else if ( strncmp(p, "skewY", 5) == 0 ) {
             p += 5 ;

             if ( !parse_number_list(p, nums) ) return false ;

             if ( nums.size() >= 1 )
                 res.skew(0.0, nums[0]) ;
         }

         eat_white_comma(p) ;
     }
    return true ;
}

}
