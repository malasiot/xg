#ifndef __FONT_MANAGER_HPP__
#define __FONT_MANAGER_HPP__

#include <xg/font.hpp>

#include <cairo/cairo-ft.h>
#include <mutex>
#include <map>

class FontManager {
public:

    cairo_font_face_t *find(const std::string &key) {
        std::lock_guard<std::mutex> g(mx_) ;
        std::map<std::string, cairo_font_face_t *>::const_iterator it = cache_.find(key) ;
        if ( it != cache_.end() ) return it->second ;
        else return 0 ;
    }

    void save(const std::string &key, cairo_font_face_t *face) {
        std::lock_guard<std::mutex> g(mx_) ;
        cache_.insert(std::make_pair(key, face)) ;
    }

    static FontManager &instance() {
        static FontManager s_instance ;
        return s_instance ;
    }

    // Use FreeType library and FontConfig to query system for desired font
    cairo_font_face_t *queryFace(const std::string &family_name, xg::FontStyle font_style, xg::FontWeight font_weight) ;

    // Calls query face and if sucessfull scales the font found
    cairo_scaled_font_t *createFont(const xg::Font &font) ;

private:

    FontManager() = default ;

    static std::string font_face_key(const std::string &family_name, xg::FontStyle font_style, xg::FontWeight font_weight) ;


    std::map<std::string, cairo_font_face_t *> cache_ ;
    std::mutex mx_ ;



} ;


#endif
