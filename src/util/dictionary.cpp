#include <xg/util/dictionary.hpp>

#include <iostream>

using namespace std ;

namespace xg {

Dictionary::Dictionary() {}

void Dictionary::add(const string &key, const string &val)
{
    insert(std::pair<string, string>(key, val)) ;
}

void Dictionary::replace(const string &key, const string &val)
{
    auto it = find(key) ;
    if ( it != end() ) it->second = val ;
    else insert({key, val}) ;
}

void Dictionary::remove(const string &key)
{
    iterator it = find(key) ;

    if ( it != end() ) erase(it) ;
}

void Dictionary::removeSome(const regex &rx)
{
    iterator it = begin() ;

    for( ; it != end() ;  )
	{
        if ( regex_match((*it).first, rx) )
            erase(it++) ;
        else ++it ;
	}
}

void Dictionary::clear() 
{
    clear() ;
}
		

string Dictionary::get(const string &key, const string &defaultVal) const
{
    const_iterator it = find(key) ;

    if ( it != end() ) return (*it).second ;
    else return defaultVal ;
}

void Dictionary::visit(const string &key, std::function<void (const string &)> cb) const
{
    const_iterator it = find(key) ;

    if ( it != end() ) cb(it->second) ;
}
	
bool Dictionary::contains(const string &key) const
{
    const_iterator it = find(key) ;

    return ( it != end() ) ;
}

// get a list of the keys in the dictionary

std::vector<string> Dictionary::keys() const
{
    std::vector<string> res ;

    const_iterator it = begin() ;

    for( ; it != end() ; ++it )
		res.push_back((*it).first) ;

	return res ;
}

std::vector<string> Dictionary::keys(const regex &rx) const
{
    std::vector<string> res ;

    const_iterator it = begin() ;

    for( ; it != end() ; ++it )
        if ( regex_match((*it).first, rx) ) res.push_back((*it).first) ;

	return res ;
}

std::vector<string> Dictionary::values() const
{
    std::vector<string> res ;

    const_iterator it = begin() ;

    for( ; it != end() ; ++it )
		res.push_back((*it).second) ;

	return res ;
}

std::vector<string> Dictionary::values(const regex &rx) const
{
    std::vector<string> res ;

    const_iterator it = begin() ;

    for( ; it != end() ; ++it )
        if ( regex_match((*it).first, rx) ) res.push_back((*it).second) ;

	return res ;

}

void Dictionary::dump() const
{
    const_iterator it = begin() ;

    for( ; it != end() ; ++it )
        cout << (*it).first << ':' << (*it).second << endl ;
}

string Dictionary::serialize(const char *sep)
{
    stringstream strm ;
    const_iterator it = begin() ;

    for( ; it != end() ; ++it )
        strm << (*it).first << '=' << (*it).second << sep ;

    return strm.str() ;
}

int Dictionary::count() const { return size() ; }

int Dictionary::count(const regex &rx) const
{
	int cc = 0 ;

    const_iterator it = begin() ;

    for( ; it != end() ; ++it )
        if ( regex_match((*it).first, rx ) ) cc++ ;

	return cc ;
}

int Dictionary::count(const string &str) const
{
    int cc = 0 ;

    const_iterator it = begin() ;

    for( ; it != end() ; ++it )
        if ( (*it).first == str ) cc++ ;

    return cc ;
}

uint64_t Dictionary::capacity() const
{
    uint64_t bytes = 0;

    const_iterator it = begin() ;

    for( ; it != end() ; ++it )
        bytes += it->first.capacity() + it->second.capacity() ;

    return bytes ;
}

} // namespce xg
