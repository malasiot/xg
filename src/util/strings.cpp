#include <xg/util/strings.hpp>
#include <iostream>

using namespace std ;

namespace xg {

std::vector<string> split(const string &s, const char *delimeters)
{
    vector<string> tokens;
    string token;

    for( char c: s ) {
        if ( strchr(delimeters, c) == nullptr )
            token += c ;
        else  {
            if ( !token.empty() )
                tokens.push_back(token);
            token.clear();
        }
    }
    if ( !token.empty() ) tokens.push_back(token);

    return tokens ;
}

std::vector<string> split(const string &s, const regex &re)
{
    sregex_token_iterator first{s.begin(), s.end(), re, -1}, last;
    return {first, last};
}

string rtrimCopy(const string &str, const char *delim) {
    size_t endpos = str.find_last_not_of(delim);
    if( string::npos != endpos )
        return str.substr( 0, endpos+1 );
    else
        return str ;
}

void rtrim(string &str, const char *delim) {
    size_t endpos = str.find_last_not_of(delim);
    if( string::npos != endpos )
       str.substr( 0, endpos+1 ).swap(str);
}

void ltrim(string &str, const char *delim) {
    size_t startpos = str.find_first_not_of(delim);
    if( string::npos != startpos )
        str.substr( startpos ).swap(str);
}

string ltrimCopy(const string &str, const char *delim) {
    size_t startpos = str.find_first_not_of(delim);
    if( string::npos != startpos )
        return str.substr( startpos ) ;
    else
        return str ;
}

string trimCopy(const string &str, const char *delim) {
    return ltrimCopy(rtrimCopy(str, delim), delim) ;
}

void trim(string &str, const char *delim) {
    ltrim(str, delim) ;
    rtrim(str, delim) ;
}


bool startsWith(const string &src, const string &prefix)
{
    if ( src.length() >= prefix.length() )
        return src.compare(0, prefix.size(), prefix) == 0;
    else
        return false ;
}

bool endsWith(const string &src, const string &suffix)
{
    if ( src.length() >= suffix.length() )
        return src.compare ( src.length() - suffix.length(), suffix.length(), suffix) == 0;
    else
        return false ;
}


void replaceAll(string &subject, const string &search, const string &replace)
{
    size_t pos = 0;
    while ( (pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

string replaceAllCopy(const string &subject, const string &search, const string &replace)
{
    string res ;
    size_t pos = 0, start_pos = 0;
    while ( (pos = subject.find(search, pos)) != std::string::npos) {
        res.append(subject.substr(start_pos, pos - start_pos)) ;
        res.append(replace) ;
        pos += search.length();
        start_pos = pos;
    }
    return res ;
}

string toUpperCopy(const std::string &source) {
    string res ;
    std::transform(source.begin(), source.end(), std::back_inserter(res), ::toupper);
    return res ;
}

string replace(string &subject, const regex &search, const string &replace) {
    return regex_replace(subject, search, replace) ;
}

string replace(string &src, const regex &re, std::function<string (const smatch &)> callback)
{
    string res ;
    size_t pos = 0, start_pos = 0;

    sregex_iterator  begin(src.begin(), src.end(), re),  end;
    std::for_each(begin, end, [&](const smatch &match) {
        pos = match.position((size_t)0);
        res.append(src.substr(start_pos, pos - start_pos)) ;
        res.append(callback(match)) ;
        pos += match.length(0) ;
        start_pos = pos ;
    });

    return res;
}

string join(const std::vector<string> &parts, const char *delimeter) {
    string s ;
    for( auto it = parts.begin() ; it != parts.end() ; ++it ) {
        if ( it != parts.begin() ) s += delimeter ;
        s += *it ;
    }
    return s ;
}

} // namespace xg

