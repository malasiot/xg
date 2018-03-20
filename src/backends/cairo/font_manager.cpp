#include "font_manager.hpp"

using namespace std ;
using namespace xg ;

string FontManager::font_face_key(const string &family_name, FontStyle font_style, FontWeight font_weight) {
    string key = family_name  + '-' ;

    switch ( font_style )
    {
    case FontStyle::Normal:
        key += "normal-" ;
        break ;
    case FontStyle::Oblique:
        key += "oblique-" ;
        break ;
    case FontStyle::Italic:
        key += "italic-" ;
        break ;
    }

    switch ( font_weight )
    {
    case FontWeight::Normal:
        key += "normal" ;
        break ;
    case FontWeight::Bold:
        key += "bold" ;
        break ;
    }

    return key ;
}

// https://stackoverflow.com/a/4119881

static bool iequals(const string& a, const string& b) {
    unsigned int sz = a.size();
    if (b.size() != sz) return false;

    for (unsigned int i = 0; i < sz; ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
}

cairo_font_face_t *FontManager::queryFace(const std::string &family_name, FontStyle font_style, FontWeight font_weight)
{
    string key = font_face_key(family_name, font_style, font_weight) ;

    cairo_font_face_t *face = find(key) ;

    if ( face ) return face ;

    FcPattern* pat = FcPatternCreate() ;

    FcPatternAddString(pat, FC_FAMILY, (const FcChar8*)(family_name.c_str()));

    if ( font_style == FontStyle::Italic )
        FcPatternAddInteger(pat, FC_SLANT, FC_SLANT_ITALIC) ;
    else if ( font_style == FontStyle::Oblique )
        FcPatternAddInteger(pat, FC_SLANT, FC_SLANT_OBLIQUE) ;
    else
        FcPatternAddInteger(pat, FC_SLANT, FC_SLANT_ROMAN) ;

    if ( font_weight == FontWeight::Bold )
        FcPatternAddInteger(pat, FC_WEIGHT, FC_WEIGHT_BOLD) ;
    else
        FcPatternAddInteger(pat, FC_WEIGHT, FC_WEIGHT_NORMAL) ;

    FcPatternAddBool(pat, FC_SCALABLE, FcTrue);


    cairo_font_options_t *font_options =  cairo_font_options_create ();


    //cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_DEFAULT) ;


    cairo_font_options_set_antialias (font_options, CAIRO_ANTIALIAS_SUBPIXEL);
    cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_FULL);
    cairo_font_options_set_hint_metrics (font_options, CAIRO_HINT_METRICS_ON);
    cairo_font_options_set_subpixel_order (font_options, CAIRO_SUBPIXEL_ORDER_RGB);

    cairo_font_options_destroy (font_options);

    cairo_ft_font_options_substitute(font_options, pat) ;

    FcConfigSubstitute(0, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    FcChar8* fontConfigFamilyNameAfterConfiguration;
    FcPatternGetString(pat, FC_FAMILY, 0, &fontConfigFamilyNameAfterConfiguration);

    FcResult fontConfigResult;
    FcPattern *resultPattern = FcFontMatch(0, pat, &fontConfigResult);
    if (!resultPattern) // No match.
        return 0;

    FcChar8* fc_family_name_str;
    FcPatternGetString(resultPattern, FC_FAMILY, 0, &fc_family_name_str);
    string fc_family_name((char *)fc_family_name_str) ;

    if (!iequals(family_name, fc_family_name)
            && !(iequals(family_name, "sans") || iequals(family_name, "sans-serif")
              || iequals(family_name, "serif") || iequals(family_name, "monospace")
              || iequals(family_name, "fantasy") || iequals(family_name, "cursive")))
        face = nullptr ;
    else
        face = cairo_ft_font_face_create_for_pattern(resultPattern) ;

    cairo_font_options_destroy(font_options) ;

    FcPatternDestroy(pat) ;

    FcPatternDestroy(resultPattern) ;

    save(key, face) ;

    return face ;
}


cairo_scaled_font_t *FontManager::createFont(const Font &font)
{
    vector<string> q_family_names(font.familyNames()) ;
    q_family_names.emplace_back("serif") ; // fallback family (OS dependent)

    for( const auto &family: q_family_names ) {
        cairo_font_face_t *face = FontManager::instance().queryFace(family, font.style(), font.weight()) ;
        if ( face ) {
            // create scaled font

            cairo_matrix_t ctm, font_matrix;
            cairo_font_options_t *font_options;

            double font_size = font.size()  ;

            cairo_matrix_init_identity (&ctm);
            cairo_matrix_init_scale (&font_matrix, font_size, font_size);
            font_options = cairo_font_options_create ();
            cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_MEDIUM);
            cairo_font_options_set_hint_metrics (font_options, CAIRO_HINT_METRICS_OFF);

            cairo_scaled_font_t *scaled_font = cairo_scaled_font_create (face,
                                                                 &font_matrix,
                                                                 &ctm,
                                                                 font_options);

            cairo_font_options_destroy (font_options);

            if ( cairo_scaled_font_status(scaled_font) != CAIRO_STATUS_SUCCESS ) {
                cairo_font_face_destroy (face);
                return nullptr ;
            }

            return scaled_font ;
        }
    }

    return nullptr ;
}
