#ifndef __XG_SVG_PARSE_UTIL_HPP__
#define __XG_SVG_PARSE_UTIL_HPP__

#include <string>
#include <vector>

#include <xg/xform.hpp>

namespace xg {

// parse number in string
bool parse_number(const std::string &s, float &v) ;

bool parse_coordinate_list(const std::string &p, std::vector<float> &args) ;
bool parse_coordinate_list(const std::string &p, float &arg1, float &arg2) ;
bool parse_coordinate_list(const std::string &p, float &arg1, float &arg2, float &arg3, float &arg4) ;
bool parse_coordinate_list(const std::string &p, float &arg1, float &arg2, float &arg3, float &arg4, float &arg5, float &arg6) ;
bool parse_coordinate_list(const std::string &p, float &arg1, float &arg2, float &arg3, float &arg4,
                                                                    float &arg5, float &arg6, float &arg7) ;

std::string parse_uri(const std::string &src) ;

void eat_white(const char *&p) ;
void eat_white_comma(const char *&p) ;

bool parse_transform(const std::string &str, Matrix2d &res) ;

}
#endif
