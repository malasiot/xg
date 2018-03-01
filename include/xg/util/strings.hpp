#ifndef __XG_STRINGS_HPP__
#define __XG_STRINGS_HPP__

#include <string>
#include <sstream>
#include <vector>
#include <regex>

// various string manipulation functions

namespace xg {

// tokenize string by spliting with one of delimeters
std::vector<std::string> split(const std::string &s, const char *delimeters = " ") ;

// tokenize string by spliting with regex
std::vector<std::string> split(const std::string &s, const std::regex &space) ;

// join string list with delimeter
std::string join(const std::vector<std::string> &parts, const char *delimeter) ;

// trimming
std::string rtrimCopy(const std::string &str, const char *delim = " \t\n\r") ;
void rtrim(std::string &str, const char *delim = " \t\n\r") ;
std::string ltrimCopy(const std::string &str, const char *delim = " \t\n\r") ;
void ltrim(std::string &str, const char *delim = " \t\n\r") ;
std::string trimCopy(const std::string &str, const char *delim = " \t\n\r") ;
void trim(std::string &str, const char *delim = " \t\n\r") ;

// prefix
bool startsWith(const std::string &src, const std::string &prefix) ;
// suffix
bool endsWith(const std::string &src, const std::string &suffix) ;

// search & replace

void replaceAll(std::string &src, const std::string &subject, const std::string &replacement) ;
std::string replaceAllCopy(const std::string &src, const std::string &subject, const std::string &replacement) ;

// convertion
std::string toUpperCopy(const std::string &source) ;

// regex replace, uses std::regex_replace
std::string replace(std::string &src, const std::regex &subject, const std::string &replacement) ;
// regex replace with a callback function
std::string replace(std::string &src, const std::regex &subject, std::function<std::string (const std::smatch &)> callback) ;

} // namespace xg

#endif
