/*
 *******************************************************************************
 *
 *   Copyright (C) 1999-2016, International Business Machines
 *   Corporation and others.  All Rights Reserved.
 *
 *******************************************************************************
 *   file name:  scrptrun.cpp
 *
 *   created on: 10/17/2001
 *   created by: Eric R. Mader
 */

#include "unicode/utypes.h"
#include "unicode/uscript.h"

#include "scrptrun.h"
#include <map>
#include <set>

using namespace std ;

const char ScriptRun::fgClassID=0;

UChar32 ScriptRun::pairedChars[] = {
    0x0028, 0x0029, // ascii paired punctuation
    0x003c, 0x003e,
    0x005b, 0x005d,
    0x007b, 0x007d,
    0x00ab, 0x00bb, // guillemets
    0x2018, 0x2019, // general punctuation
    0x201c, 0x201d,
    0x2039, 0x203a,
    0x3008, 0x3009, // chinese paired punctuation
    0x300a, 0x300b,
    0x300c, 0x300d,
    0x300e, 0x300f,
    0x3010, 0x3011,
    0x3014, 0x3015,
    0x3016, 0x3017,
    0x3018, 0x3019,
    0x301a, 0x301b
};

#define UPRV_LENGTHOF(array) (int32_t)(sizeof(array)/sizeof((array)[0]))

const int32_t ScriptRun::pairedCharCount = UPRV_LENGTHOF(pairedChars);
const int32_t ScriptRun::pairedCharPower = 1 << highBit(pairedCharCount);
const int32_t ScriptRun::pairedCharExtra = pairedCharCount - pairedCharPower;

int8_t ScriptRun::highBit(int32_t value)
{
    if (value <= 0) {
        return -32;
    }

    int8_t bit = 0;

    if (value >= 1 << 16) {
        value >>= 16;
        bit += 16;
    }

    if (value >= 1 << 8) {
        value >>= 8;
        bit += 8;
    }

    if (value >= 1 << 4) {
        value >>= 4;
        bit += 4;
    }

    if (value >= 1 << 2) {
        value >>= 2;
        bit += 2;
    }

    if (value >= 1 << 1) {
        value >>= 1;
        bit += 1;
    }

    return bit;
}

int32_t ScriptRun::getPairIndex(UChar32 ch)
{
    int32_t probe = pairedCharPower;
    int32_t index = 0;

    if (ch >= pairedChars[pairedCharExtra]) {
        index = pairedCharExtra;
    }

    while (probe > (1 << 0)) {
        probe >>= 1;

        if (ch >= pairedChars[index + probe]) {
            index += probe;
        }
    }

    if (pairedChars[index] != ch) {
        index = -1;
    }

    return index;
}

UBool ScriptRun::sameScript(int32_t scriptOne, int32_t scriptTwo)
{
    return scriptOne <= USCRIPT_INHERITED || scriptTwo <= USCRIPT_INHERITED || scriptOne == scriptTwo;
}

UBool ScriptRun::next()
{
    int32_t startSP  = parenSP;  // used to find the first new open character
    UErrorCode error = U_ZERO_ERROR;

    // if we've fallen off the end of the text, we're done
    if (scriptEnd >= charLimit) {
        return false;
    }
    
    scriptCode = USCRIPT_COMMON;

    for (scriptStart = scriptEnd; scriptEnd < charLimit; scriptEnd += 1) {
        UChar   high = charArray[scriptEnd];
        UChar32 ch   = high;

        // if the character is a high surrogate and it's not the last one
        // in the text, see if it's followed by a low surrogate
        if (high >= 0xD800 && high <= 0xDBFF && scriptEnd < charLimit - 1)
        {
            UChar low = charArray[scriptEnd + 1];

            // if it is followed by a low surrogate,
            // consume it and form the full character
            if (low >= 0xDC00 && low <= 0xDFFF) {
                ch = (high - 0xD800) * 0x0400 + low - 0xDC00 + 0x10000;
                scriptEnd += 1;
            }
        }

        UScriptCode sc = uscript_getScript(ch, &error);
        int32_t pairIndex = getPairIndex(ch);

        // Paired character handling:
        //
        // if it's an open character, push it onto the stack.
        // if it's a close character, find the matching open on the
        // stack, and use that script code. Any non-matching open
        // characters above it on the stack will be poped.
        if (pairIndex >= 0) {
            if ((pairIndex & 1) == 0) {
                parenStack[++parenSP].pairIndex = pairIndex;
                parenStack[parenSP].scriptCode  = scriptCode;
            } else if (parenSP >= 0) {
                int32_t pi = pairIndex & ~1;

                while (parenSP >= 0 && parenStack[parenSP].pairIndex != pi) {
                    parenSP -= 1;
                }

                if (parenSP < startSP) {
                    startSP = parenSP;
                }

                if (parenSP >= 0) {
                    sc = parenStack[parenSP].scriptCode;
                }
            }
        }

        if (sameScript(scriptCode, sc)) {
            if (scriptCode <= USCRIPT_INHERITED && sc > USCRIPT_INHERITED) {
                scriptCode = sc;

                // now that we have a final script code, fix any open
                // characters we pushed before we knew the script code.
                while (startSP < parenSP) {
                    parenStack[++startSP].scriptCode = scriptCode;
                }
            }

            // if this character is a close paired character,
            // pop it from the stack
            if (pairIndex >= 0 && (pairIndex & 1) != 0 && parenSP >= 0) {
                parenSP -= 1;
                startSP -= 1;
            }
        } else {
            // if the run broke on a surrogate pair,
            // end it before the high surrogate
            if (ch >= 0x10000) {
                scriptEnd -= 1;
            }

            break;
        }
    }

    return true;
}



using HBScriptForLang = std::map<std::string, std::array<hb_script_t, 3>> ;

/*
 * DATA FROM pango-script-lang-table.h
 */
static const HBScriptForLang g_scripts_for_lang =
{
    { "aa",     { HB_SCRIPT_LATIN/*62*/ } },
    { "ab",     { HB_SCRIPT_CYRILLIC/*90*/ } },
    { "af",     { HB_SCRIPT_LATIN/*69*/ } },
    { "ak",     { HB_SCRIPT_LATIN/*70*/ } },
    { "am",     { HB_SCRIPT_ETHIOPIC/*264*/ } },
    { "an",     { HB_SCRIPT_LATIN/*66*/ } },
    { "ar",     { HB_SCRIPT_ARABIC/*125*/ } },
    { "as",     { HB_SCRIPT_BENGALI/*64*/ } },
    { "ast",    { HB_SCRIPT_LATIN/*66*/ } },
    { "av",     { HB_SCRIPT_CYRILLIC/*67*/ } },
    { "ay",     { HB_SCRIPT_LATIN/*60*/ } },
    { "az-az",  { HB_SCRIPT_LATIN/*66*/ } },
    { "az-ir",  { HB_SCRIPT_ARABIC/*129*/ } },
    { "ba",     { HB_SCRIPT_CYRILLIC/*82*/ } },
    { "be",     { HB_SCRIPT_CYRILLIC/*68*/ } },
    { "ber-dz", { HB_SCRIPT_LATIN/*70*/ } },
    { "ber-ma", { HB_SCRIPT_TIFINAGH/*32*/ } },
    { "bg",     { HB_SCRIPT_CYRILLIC/*60*/ } },
    { "bh",     { HB_SCRIPT_DEVANAGARI/*68*/ } },
    { "bho",    { HB_SCRIPT_DEVANAGARI/*68*/ } },
    { "bi",     { HB_SCRIPT_LATIN/*58*/ } },
    { "bin",    { HB_SCRIPT_LATIN/*76*/ } },
    { "bm",     { HB_SCRIPT_LATIN/*60*/ } },
    { "bn",     { HB_SCRIPT_BENGALI/*63*/ } },
    { "bo",     { HB_SCRIPT_TIBETAN/*95*/ } },
    { "br",     { HB_SCRIPT_LATIN/*64*/ } },
    { "bs",     { HB_SCRIPT_LATIN/*62*/ } },
    { "bua",    { HB_SCRIPT_CYRILLIC/*70*/ } },
    { "byn",    { HB_SCRIPT_ETHIOPIC/*255*/ } },
    { "ca",     { HB_SCRIPT_LATIN/*74*/ } },
    { "ce",     { HB_SCRIPT_CYRILLIC/*67*/ } },
    { "ch",     { HB_SCRIPT_LATIN/*58*/ } },
    { "chm",    { HB_SCRIPT_CYRILLIC/*76*/ } },
    { "chr",    { HB_SCRIPT_CHEROKEE/*85*/ } },
    { "co",     { HB_SCRIPT_LATIN/*84*/ } },
    { "crh",    { HB_SCRIPT_LATIN/*68*/ } },
    { "cs",     { HB_SCRIPT_LATIN/*82*/ } },
    { "csb",    { HB_SCRIPT_LATIN/*74*/ } },
    { "cu",     { HB_SCRIPT_CYRILLIC/*103*/ } },
    { "cv",     { HB_SCRIPT_CYRILLIC/*72*/, HB_SCRIPT_LATIN/*2*/ } },
    { "cy",     { HB_SCRIPT_LATIN/*78*/ } },
    { "da",     { HB_SCRIPT_LATIN/*70*/ } },
    { "de",     { HB_SCRIPT_LATIN/*59*/ } },
    { "dv",     { HB_SCRIPT_THAANA/*49*/ } },
    { "dz",     { HB_SCRIPT_TIBETAN/*95*/ } },
    { "ee",     { HB_SCRIPT_LATIN/*96*/ } },
    { "el",     { HB_SCRIPT_GREEK/*69*/ } },
    { "en",     { HB_SCRIPT_LATIN/*72*/ } },
    { "eo",     { HB_SCRIPT_LATIN/*64*/ } },
    { "es",     { HB_SCRIPT_LATIN/*66*/ } },
    { "et",     { HB_SCRIPT_LATIN/*64*/ } },
    { "eu",     { HB_SCRIPT_LATIN/*56*/ } },
    { "fa",     { HB_SCRIPT_ARABIC/*129*/ } },
    { "fat",    { HB_SCRIPT_LATIN/*70*/ } },
    { "ff",     { HB_SCRIPT_LATIN/*62*/ } },
    { "fi",     { HB_SCRIPT_LATIN/*62*/ } },
    { "fil",    { HB_SCRIPT_LATIN/*84*/ } },
    { "fj",     { HB_SCRIPT_LATIN/*52*/ } },
    { "fo",     { HB_SCRIPT_LATIN/*68*/ } },
    { "fr",     { HB_SCRIPT_LATIN/*84*/ } },
    { "fur",    { HB_SCRIPT_LATIN/*66*/ } },
    { "fy",     { HB_SCRIPT_LATIN/*75*/ } },
    { "ga",     { HB_SCRIPT_LATIN/*80*/ } },
    { "gd",     { HB_SCRIPT_LATIN/*70*/ } },
    { "gez",    { HB_SCRIPT_ETHIOPIC/*218*/ } },
    { "gl",     { HB_SCRIPT_LATIN/*66*/ } },
    { "gn",     { HB_SCRIPT_LATIN/*70*/ } },
    { "gu",     { HB_SCRIPT_GUJARATI/*68*/ } },
    { "gv",     { HB_SCRIPT_LATIN/*54*/ } },
    { "ha",     { HB_SCRIPT_LATIN/*60*/ } },
    { "haw",    { HB_SCRIPT_LATIN/*62*/ } },
    { "he",     { HB_SCRIPT_HEBREW/*27*/ } },
    { "hi",     { HB_SCRIPT_DEVANAGARI/*68*/ } },
    { "hne",    { HB_SCRIPT_DEVANAGARI/*68*/ } },
    { "ho",     { HB_SCRIPT_LATIN/*52*/ } },
    { "hr",     { HB_SCRIPT_LATIN/*62*/ } },
    { "hsb",    { HB_SCRIPT_LATIN/*72*/ } },
    { "ht",     { HB_SCRIPT_LATIN/*56*/ } },
    { "hu",     { HB_SCRIPT_LATIN/*70*/ } },
    { "hy",     { HB_SCRIPT_ARMENIAN/*77*/ } },
    { "hz",     { HB_SCRIPT_LATIN/*56*/ } },
    { "ia",     { HB_SCRIPT_LATIN/*52*/ } },
    { "id",     { HB_SCRIPT_LATIN/*54*/ } },
    { "ie",     { HB_SCRIPT_LATIN/*52*/ } },
    { "ig",     { HB_SCRIPT_LATIN/*58*/ } },
    { "ii",     { HB_SCRIPT_YI/*1165*/ } },
    { "ik",     { HB_SCRIPT_CYRILLIC/*68*/ } },
    { "io",     { HB_SCRIPT_LATIN/*52*/ } },
    { "is",     { HB_SCRIPT_LATIN/*70*/ } },
    { "it",     { HB_SCRIPT_LATIN/*72*/ } },
    { "iu",     { HB_SCRIPT_CANADIAN_ABORIGINAL/*161*/ } },
    { "ja",     { HB_SCRIPT_HAN/*6356*/, HB_SCRIPT_KATAKANA/*88*/, HB_SCRIPT_HIRAGANA/*85*/ } },
    { "jv",     { HB_SCRIPT_LATIN/*56*/ } },
    { "ka",     { HB_SCRIPT_GEORGIAN/*33*/ } },
    { "kaa",    { HB_SCRIPT_CYRILLIC/*78*/ } },
    { "kab",    { HB_SCRIPT_LATIN/*70*/ } },
    { "ki",     { HB_SCRIPT_LATIN/*56*/ } },
    { "kj",     { HB_SCRIPT_LATIN/*52*/ } },
    { "kk",     { HB_SCRIPT_CYRILLIC/*77*/ } },
    { "kl",     { HB_SCRIPT_LATIN/*81*/ } },
    { "km",     { HB_SCRIPT_KHMER/*63*/ } },
    { "kn",     { HB_SCRIPT_KANNADA/*70*/ } },
    { "ko",     { HB_SCRIPT_HANGUL/*2443*/ } },
    { "kok",    { HB_SCRIPT_DEVANAGARI/*68*/ } },
    { "kr",     { HB_SCRIPT_LATIN/*56*/ } },
    { "ks",     { HB_SCRIPT_ARABIC/*145*/ } },
    { "ku-am",  { HB_SCRIPT_CYRILLIC/*64*/ } },
    { "ku-iq",  { HB_SCRIPT_ARABIC/*32*/ } },
    { "ku-ir",  { HB_SCRIPT_ARABIC/*32*/ } },
    { "ku-tr",  { HB_SCRIPT_LATIN/*62*/ } },
    { "kum",    { HB_SCRIPT_CYRILLIC/*66*/ } },
    { "kv",     { HB_SCRIPT_CYRILLIC/*70*/ } },
    { "kw",     { HB_SCRIPT_LATIN/*64*/ } },
    { "kwm",    { HB_SCRIPT_LATIN/*52*/ } },
    { "ky",     { HB_SCRIPT_CYRILLIC/*70*/ } },
    { "la",     { HB_SCRIPT_LATIN/*68*/ } },
    { "lb",     { HB_SCRIPT_LATIN/*75*/ } },
    { "lez",    { HB_SCRIPT_CYRILLIC/*67*/ } },
    { "lg",     { HB_SCRIPT_LATIN/*54*/ } },
    { "li",     { HB_SCRIPT_LATIN/*62*/ } },
    { "ln",     { HB_SCRIPT_LATIN/*78*/ } },
    { "lo",     { HB_SCRIPT_LAO/*55*/ } },
    { "lt",     { HB_SCRIPT_LATIN/*70*/ } },
    { "lv",     { HB_SCRIPT_LATIN/*78*/ } },
    { "mai",    { HB_SCRIPT_DEVANAGARI/*68*/ } },
    { "mg",     { HB_SCRIPT_LATIN/*56*/ } },
    { "mh",     { HB_SCRIPT_LATIN/*62*/ } },
    { "mi",     { HB_SCRIPT_LATIN/*64*/ } },
    { "mk",     { HB_SCRIPT_CYRILLIC/*42*/ } },
    { "ml",     { HB_SCRIPT_MALAYALAM/*68*/ } },
    { "mn-cn",  { HB_SCRIPT_MONGOLIAN/*130*/ } },
    { "mn-mn",  { HB_SCRIPT_CYRILLIC/*70*/ } },
    { "mo",     { HB_SCRIPT_CYRILLIC/*66*/, HB_SCRIPT_LATIN/*62*/ } },
    { "mr",     { HB_SCRIPT_DEVANAGARI/*68*/ } },
    { "ms",     { HB_SCRIPT_LATIN/*52*/ } },
    { "mt",     { HB_SCRIPT_LATIN/*72*/ } },
    { "my",     { HB_SCRIPT_MYANMAR/*48*/ } },
    { "na",     { HB_SCRIPT_LATIN/*60*/ } },
    { "nb",     { HB_SCRIPT_LATIN/*70*/ } },
    { "nds",    { HB_SCRIPT_LATIN/*59*/ } },
    { "ne",     { HB_SCRIPT_DEVANAGARI/*68*/ } },
    { "ng",     { HB_SCRIPT_LATIN/*52*/ } },
    { "nl",     { HB_SCRIPT_LATIN/*82*/ } },
    { "nn",     { HB_SCRIPT_LATIN/*76*/ } },
    { "no",     { HB_SCRIPT_LATIN/*70*/ } },
    { "nr",     { HB_SCRIPT_LATIN/*52*/ } },
    { "nso",    { HB_SCRIPT_LATIN/*58*/ } },
    { "nv",     { HB_SCRIPT_LATIN/*70*/ } },
    { "ny",     { HB_SCRIPT_LATIN/*54*/ } },
    { "oc",     { HB_SCRIPT_LATIN/*70*/ } },
    { "om",     { HB_SCRIPT_LATIN/*52*/ } },
    { "or",     { HB_SCRIPT_ORIYA/*68*/ } },
    { "os",     { HB_SCRIPT_CYRILLIC/*66*/ } },
    { "ota",    { HB_SCRIPT_ARABIC/*37*/ } },
    { "pa-in",  { HB_SCRIPT_GURMUKHI/*63*/ } },
    { "pa-pk",  { HB_SCRIPT_ARABIC/*145*/ } },
    { "pap-an", { HB_SCRIPT_LATIN/*72*/ } },
    { "pap-aw", { HB_SCRIPT_LATIN/*54*/ } },
    { "pl",     { HB_SCRIPT_LATIN/*70*/ } },
    { "ps-af",  { HB_SCRIPT_ARABIC/*49*/ } },
    { "ps-pk",  { HB_SCRIPT_ARABIC/*49*/ } },
    { "pt",     { HB_SCRIPT_LATIN/*82*/ } },
    { "qu",     { HB_SCRIPT_LATIN/*54*/ } },
    { "rm",     { HB_SCRIPT_LATIN/*66*/ } },
    { "rn",     { HB_SCRIPT_LATIN/*52*/ } },
    { "ro",     { HB_SCRIPT_LATIN/*62*/ } },
    { "ru",     { HB_SCRIPT_CYRILLIC/*66*/ } },
    { "rw",     { HB_SCRIPT_LATIN/*52*/ } },
    { "sa",     { HB_SCRIPT_DEVANAGARI/*68*/ } },
    { "sah",    { HB_SCRIPT_CYRILLIC/*76*/ } },
    { "sc",     { HB_SCRIPT_LATIN/*62*/ } },
    { "sco",    { HB_SCRIPT_LATIN/*56*/ } },
    { "sd",     { HB_SCRIPT_ARABIC/*54*/ } },
    { "se",     { HB_SCRIPT_LATIN/*66*/ } },
    { "sel",    { HB_SCRIPT_CYRILLIC/*66*/ } },
    { "sg",     { HB_SCRIPT_LATIN/*72*/ } },
    { "sh",     { HB_SCRIPT_CYRILLIC/*94*/, HB_SCRIPT_LATIN/*62*/ } },
    { "shs",    { HB_SCRIPT_LATIN/*46*/ } },
    { "si",     { HB_SCRIPT_SINHALA/*73*/ } },
    { "sid",    { HB_SCRIPT_ETHIOPIC/*281*/ } },
    { "sk",     { HB_SCRIPT_LATIN/*86*/ } },
    { "sl",     { HB_SCRIPT_LATIN/*62*/ } },
    { "sm",     { HB_SCRIPT_LATIN/*52*/ } },
    { "sma",    { HB_SCRIPT_LATIN/*60*/ } },
    { "smj",    { HB_SCRIPT_LATIN/*60*/ } },
    { "smn",    { HB_SCRIPT_LATIN/*68*/ } },
    { "sms",    { HB_SCRIPT_LATIN/*80*/ } },
    { "sn",     { HB_SCRIPT_LATIN/*52*/ } },
    { "so",     { HB_SCRIPT_LATIN/*52*/ } },
    { "sq",     { HB_SCRIPT_LATIN/*56*/ } },
    { "sr",     { HB_SCRIPT_CYRILLIC/*60*/ } },
    { "ss",     { HB_SCRIPT_LATIN/*52*/ } },
    { "st",     { HB_SCRIPT_LATIN/*52*/ } },
    { "su",     { HB_SCRIPT_LATIN/*54*/ } },
    { "sv",     { HB_SCRIPT_LATIN/*68*/ } },
    { "sw",     { HB_SCRIPT_LATIN/*52*/ } },
    { "syr",    { HB_SCRIPT_SYRIAC/*45*/ } },
    { "ta",     { HB_SCRIPT_TAMIL/*48*/ } },
    { "te",     { HB_SCRIPT_TELUGU/*70*/ } },
    { "tg",     { HB_SCRIPT_CYRILLIC/*78*/ } },
    { "th",     { HB_SCRIPT_THAI/*73*/ } },
    { "ti-er",  { HB_SCRIPT_ETHIOPIC/*255*/ } },
    { "ti-et",  { HB_SCRIPT_ETHIOPIC/*281*/ } },
    { "tig",    { HB_SCRIPT_ETHIOPIC/*221*/ } },
    { "tk",     { HB_SCRIPT_LATIN/*68*/ } },
    { "tl",     { HB_SCRIPT_LATIN/*84*/ } },
    { "tn",     { HB_SCRIPT_LATIN/*58*/ } },
    { "to",     { HB_SCRIPT_LATIN/*52*/ } },
    { "tr",     { HB_SCRIPT_LATIN/*70*/ } },
    { "ts",     { HB_SCRIPT_LATIN/*52*/ } },
    { "tt",     { HB_SCRIPT_CYRILLIC/*76*/ } },
    { "tw",     { HB_SCRIPT_LATIN/*70*/ } },
    { "ty",     { HB_SCRIPT_LATIN/*64*/ } },
    { "tyv",    { HB_SCRIPT_CYRILLIC/*70*/ } },
    { "ug",     { HB_SCRIPT_ARABIC/*125*/ } },
    { "uk",     { HB_SCRIPT_CYRILLIC/*72*/ } },
    { "ur",     { HB_SCRIPT_ARABIC/*145*/ } },
    { "uz",     { HB_SCRIPT_LATIN/*52*/ } },
    { "ve",     { HB_SCRIPT_LATIN/*62*/ } },
    { "vi",     { HB_SCRIPT_LATIN/*186*/ } },
    { "vo",     { HB_SCRIPT_LATIN/*54*/ } },
    { "vot",    { HB_SCRIPT_LATIN/*62*/ } },
    { "wa",     { HB_SCRIPT_LATIN/*70*/ } },
    { "wal",    { HB_SCRIPT_ETHIOPIC/*281*/ } },
    { "wen",    { HB_SCRIPT_LATIN/*76*/ } },
    { "wo",     { HB_SCRIPT_LATIN/*66*/ } },
    { "xh",     { HB_SCRIPT_LATIN/*52*/ } },
    { "yap",    { HB_SCRIPT_LATIN/*58*/ } },
    { "yi",     { HB_SCRIPT_HEBREW/*27*/ } },
    { "yo",     { HB_SCRIPT_LATIN/*114*/ } },
    { "za",     { HB_SCRIPT_LATIN/*52*/ } },
    { "zh-cn",  { HB_SCRIPT_HAN/*6763*/ } },
    { "zh-hk",  { HB_SCRIPT_HAN/*2213*/ } },
    { "zh-mo",  { HB_SCRIPT_HAN/*2213*/ } },
    { "zh-sg",  { HB_SCRIPT_HAN/*6763*/ } },
    { "zh-tw",  { HB_SCRIPT_HAN/*13063*/ } },
    { "zu",     { HB_SCRIPT_LATIN/*52*/ } }
};

static std::set<std::string> g_default_languages = { "en" } ;


static std::map<hb_script_t, std::string> g_sample_language_map =
{
    {HB_SCRIPT_ARABIC,"ar"},
    {HB_SCRIPT_ARMENIAN,"hy"},
    {HB_SCRIPT_BENGALI,"bn"},
    {HB_SCRIPT_CHEROKEE,"chr"},
    {HB_SCRIPT_COPTIC,"cop"},
    {HB_SCRIPT_CYRILLIC,"ru"},
    {HB_SCRIPT_DEVANAGARI,"hi"},
    {HB_SCRIPT_ETHIOPIC,"am"},
    {HB_SCRIPT_GEORGIAN,"ka"},
    {HB_SCRIPT_GREEK,"el"},
    {HB_SCRIPT_GUJARATI,"gu"},
    {HB_SCRIPT_GURMUKHI,"pa"},
    {HB_SCRIPT_HANGUL,"ko"},
    {HB_SCRIPT_HEBREW,"he"},
    {HB_SCRIPT_HIRAGANA,"ja"},
    {HB_SCRIPT_KANNADA,"kn"},
    {HB_SCRIPT_KATAKANA,"ja"},
    {HB_SCRIPT_KHMER,"km"},
    {HB_SCRIPT_LAO,"lo"},
    {HB_SCRIPT_LATIN,"en"},
    {HB_SCRIPT_MALAYALAM,"ml"},
    {HB_SCRIPT_MONGOLIAN,"mn"},
    {HB_SCRIPT_MYANMAR,"my"},
    {HB_SCRIPT_ORIYA,"or"},
    {HB_SCRIPT_SINHALA,"si"},
    {HB_SCRIPT_SYRIAC,"syr"},
    {HB_SCRIPT_TAMIL,"ta"},
    {HB_SCRIPT_TELUGU,"te"},
    {HB_SCRIPT_THAANA,"dv"},
    {HB_SCRIPT_THAI,"th"},
    {HB_SCRIPT_TIBETAN,"bo"},
    {HB_SCRIPT_CANADIAN_ABORIGINAL,"iu"},
    {HB_SCRIPT_TAGALOG,"tl"},
    {HB_SCRIPT_HANUNOO,"hnn"},
    {HB_SCRIPT_BUHID,"bku"},
    {HB_SCRIPT_TAGBANWA,"tbw"},
    {HB_SCRIPT_UGARITIC,"uga"},
    {HB_SCRIPT_BUGINESE,"bug"},
    {HB_SCRIPT_SYLOTI_NAGRI,"syl"},
    {HB_SCRIPT_OLD_PERSIAN,"peo"},
    {HB_SCRIPT_NKO,"nqo"},

    /*
     * DEFAULT-VALUE
     */
    {HB_SCRIPT_INVALID, ""}
};

string ScriptRun::detectLanguage(hb_script_t code) {

    for (auto &lang : g_default_languages ) {
        auto it = g_scripts_for_lang.find(lang) ;
        if ( it != g_scripts_for_lang.end() ) {
            for( auto &script: it->second ) {
                if ( script == code ) return lang ;
            }
        }
    }

    auto it = g_sample_language_map.find(code);

    if ( it == g_sample_language_map.end() ) {
        it = g_sample_language_map.find(HB_SCRIPT_INVALID);
    }
    return it->second ;
}
