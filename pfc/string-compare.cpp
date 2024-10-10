#include "pfc-lite.h"

#include "string-compare.h"
#include "string_base.h"
#include "debug.h"
#include "bsearch_inline.h"
#include "sortstring.h"

namespace pfc {
    unsigned charToANSI(unsigned GotChar, unsigned fallback) {
        if (GotChar < 128) return GotChar;

        static constexpr uint16_t from[] = {L'\u00C0', L'\u00C1', L'\u00C2', L'\u00C3', L'\u00C4', L'\u00C5', L'\u00C7', L'\u00C8', L'\u00C9', L'\u00CA', L'\u00CB', L'\u00CC', L'\u00CD', L'\u00CE', L'\u00CF', L'\u00D1', L'\u00D2', L'\u00D3', L'\u00D4', L'\u00D5', L'\u00D6', L'\u00D8', L'\u00D9', L'\u00DA', L'\u00DB', L'\u00DC', L'\u00DD', L'\u00E0', L'\u00E1', L'\u00E2', L'\u00E3', L'\u00E4', L'\u00E5', L'\u00E7', L'\u00E8', L'\u00E9', L'\u00EA', L'\u00EB', L'\u00EC', L'\u00ED', L'\u00EE', L'\u00EF', L'\u00F0', L'\u00F1', L'\u00F2', L'\u00F3', L'\u00F4', L'\u00F5', L'\u00F6', L'\u00F8', L'\u00F9', L'\u00FA', L'\u00FB', L'\u00FC', L'\u00FD', L'\u0100', L'\u0101', L'\u0102', L'\u0103', L'\u0104', L'\u0105', L'\u0106', L'\u0107', L'\u0108', L'\u0109', L'\u010A', L'\u010B', L'\u010C', L'\u010D', L'\u010E', L'\u010F', L'\u0110', L'\u0111', L'\u0112', L'\u0113', L'\u0114', L'\u0115', L'\u0116', L'\u0117', L'\u0118', L'\u0119', L'\u011A', L'\u011B', L'\u011C', L'\u011D', L'\u011E', L'\u011F', L'\u0120', L'\u0121', L'\u0122', L'\u0123', L'\u0128', L'\u0129', L'\u012A', L'\u012B', L'\u012C', L'\u012D', L'\u012E', L'\u012F', L'\u0130', L'\u0131', L'\u0134', L'\u0135', L'\u0136', L'\u0137', L'\u0139', L'\u013A', L'\u013B', L'\u013C', L'\u013D', L'\u013E', L'\u013F', L'\u0140', L'\u0141', L'\u0142', L'\u0143', L'\u0144', L'\u0145', L'\u0146', L'\u0147', L'\u0148', L'\u0149', L'\u014A', L'\u014B', L'\u014C', L'\u014D', L'\u014E', L'\u014F', L'\u0150', L'\u0151', L'\u0154', L'\u0155', L'\u0156', L'\u0157', L'\u0158', L'\u0159', L'\u015A', L'\u015B', L'\u015C', L'\u015D', L'\u015E', L'\u015F', L'\u0160', L'\u0161', L'\u0162', L'\u0163', L'\u0164', L'\u0165', L'\u0166', L'\u0167', L'\u0168', L'\u0169', L'\u016A', L'\u016B', L'\u016C', L'\u016D', L'\u016E', L'\u016F', L'\u0170', L'\u0171', L'\u0172', L'\u0173', L'\u0174', L'\u0175', L'\u0176', L'\u0177', L'\u0178', L'\u0179', L'\u017A', L'\u017B', L'\u017C', L'\u017D', L'\u017E'};
        static constexpr uint16_t to[] = {L'\u0041', L'\u0041', L'\u0041', L'\u0041', L'\u0041', L'\u0041', L'\u0043', L'\u0045', L'\u0045', L'\u0045', L'\u0045', L'\u0049', L'\u0049', L'\u0049', L'\u0049', L'\u004E', L'\u004F', L'\u004F', L'\u004F', L'\u004F', L'\u004F', L'\u004F', L'\u0055', L'\u0055', L'\u0055', L'\u0055', L'\u0059', L'\u0061', L'\u0061', L'\u0061', L'\u0061', L'\u0061', L'\u0061', L'\u0063', L'\u0065', L'\u0065', L'\u0065', L'\u0065', L'\u0069', L'\u0069', L'\u0069', L'\u0069', L'\u006F', L'\u006E', L'\u006F', L'\u006F', L'\u006F', L'\u006F', L'\u006F', L'\u006F', L'\u0075', L'\u0075', L'\u0075', L'\u0075', L'\u0079', L'\u0041', L'\u0061', L'\u0041', L'\u0061', L'\u0041', L'\u0061', L'\u0043', L'\u0063', L'\u0043', L'\u0063', L'\u0043', L'\u0063', L'\u0043', L'\u0063', L'\u0044', L'\u0064', L'\u0044', L'\u0064', L'\u0045', L'\u0065', L'\u0045', L'\u0065', L'\u0045', L'\u0065', L'\u0045', L'\u0065', L'\u0045', L'\u0065', L'\u0047', L'\u0067', L'\u0047', L'\u0067', L'\u0047', L'\u0067', L'\u0047', L'\u0067', L'\u0049', L'\u0069', L'\u0049', L'\u0069', L'\u0049', L'\u0069', L'\u0049', L'\u0069', L'\u0049', L'\u0069', L'\u004A', L'\u006A', L'\u004B', L'\u006B', L'\u004C', L'\u006C', L'\u004C', L'\u006C', L'\u004C', L'\u006C', L'\u004C', L'\u006C', L'\u004C', L'\u006C', L'\u004E', L'\u006E', L'\u004E', L'\u006E', L'\u004E', L'\u006E', L'\u006E', L'\u004E', L'\u006E', L'\u004F', L'\u006F', L'\u004F', L'\u006F', L'\u004F', L'\u006F', L'\u0052', L'\u0072', L'\u0052', L'\u0072', L'\u0052', L'\u0072', L'\u0053', L'\u0073', L'\u0053', L'\u0073', L'\u0053', L'\u0073', L'\u0053', L'\u0073', L'\u0054', L'\u0074', L'\u0054', L'\u0074', L'\u0054', L'\u0074', L'\u0055', L'\u0075', L'\u0055', L'\u0075', L'\u0055', L'\u0075', L'\u0055', L'\u0075', L'\u0055', L'\u0075', L'\u0055', L'\u0075', L'\u0057', L'\u0077', L'\u0059', L'\u0079', L'\u0059', L'\u005A', L'\u007A', L'\u005A', L'\u007A', L'\u005A', L'\u007A'};
        static_assert(std::size(from) == std::size(to));

        size_t idx;
        if (bsearch_simple_inline_t(from, std::size(from), GotChar, idx)) {
            return to[idx];
        }

        return fallback;
    }

    int stricmp_ascii_partial(const char* str, const char* substr) throw() {
        size_t walk = 0;
        for (;;) {
            char c1 = str[walk];
            char c2 = substr[walk];
            c1 = ascii_tolower(c1); c2 = ascii_tolower(c2);
            if (c2 == 0) return 0; // substr terminated = ret0 regardless of str content
            if (c1 < c2) return -1; // ret -1 early
            else if (c1 > c2) return 1; // ret 1 early
            // else c1 == c2 and c2 != 0 so c1 != 0 either
            ++walk; // go on
        }
    }

    bool stringEqualsI_ascii_ex(const char* s1, size_t len1, const char* s2, size_t len2) throw() {
        t_size walk1 = 0, walk2 = 0;
        for (;;) {
            char c1 = (walk1 < len1) ? s1[walk1] : 0;
            char c2 = (walk2 < len2) ? s2[walk2] : 0;
            c1 = ascii_tolower(c1); c2 = ascii_tolower(c2);
            if (c1 != c2) return false;
            if (c1 == 0) return true;
            walk1++;
            walk2++;
        }
    }

    int stricmp_ascii_ex(const char* const s1, t_size const len1, const char* const s2, t_size const len2) throw() {
        t_size walk1 = 0, walk2 = 0;
        for (;;) {
            char c1 = (walk1 < len1) ? s1[walk1] : 0;
            char c2 = (walk2 < len2) ? s2[walk2] : 0;
            c1 = ascii_tolower(c1); c2 = ascii_tolower(c2);
            if (c1 < c2) return -1;
            else if (c1 > c2) return 1;
            else if (c1 == 0) return 0;
            walk1++;
            walk2++;
        }
    }

    int wstricmp_ascii(const wchar_t* s1, const wchar_t* s2) throw() {
        for (;;) {
            wchar_t c1 = *s1, c2 = *s2;

            if (c1 > 0 && c2 > 0 && c1 < 128 && c2 < 128) {
                c1 = ascii_tolower_lookup((char)c1);
                c2 = ascii_tolower_lookup((char)c2);
            } else {
                if (c1 == 0 && c2 == 0) return 0;
            }
            if (c1 < c2) return -1;
            else if (c1 > c2) return 1;
            else if (c1 == 0) return 0;

            s1++;
            s2++;
        }
    }

    int stricmp_ascii(const char* s1, const char* s2) throw() {
        for (;;) {
            char c1 = *s1, c2 = *s2;

            if (c1 > 0 && c2 > 0) {
                c1 = ascii_tolower_lookup(c1);
                c2 = ascii_tolower_lookup(c2);
            } else {
                if (c1 == 0 && c2 == 0) return 0;
            }
            if (c1 < c2) return -1;
            else if (c1 > c2) return 1;
            else if (c1 == 0) return 0;

            s1++;
            s2++;
        }
    }

    static int naturalSortCompareInternal(const char* s1, const char* s2, bool insensitive) throw() {
        for (;; ) {
            unsigned c1, c2;
            size_t d1 = utf8_decode_char(s1, c1);
            size_t d2 = utf8_decode_char(s2, c2);
            if (d1 == 0 && d2 == 0) {
                return 0;
            }
            if (char_is_numeric(c1) && char_is_numeric(c2)) {
                // Numeric block in both strings, do natural sort magic here
                size_t l1 = 1, l2 = 1;
                while (char_is_numeric(s1[l1])) ++l1;
                while (char_is_numeric(s2[l2])) ++l2;

                size_t l = max_t(l1, l2);
                for (int pass = 0; pass < 2; ++pass) {
                    const char filler = pass ? 'z' : '0';
                    for (size_t w = 0; w < l; ++w) {
                        char digit1 = filler, digit2 = filler;

                        t_ssize off;

                        off = w + l1 - l;
                        if (off >= 0) {
                            digit1 = s1[w - l + l1];
                        }
                        off = w + l2 - l;
                        if (off >= 0) {
                            digit2 = s2[w - l + l2];
                        }
                        if (digit1 < digit2) return -1;
                        if (digit1 > digit2) return 1;
                    }
                }
                s1 += l1; s2 += l2;
                continue;
            }

            unsigned alt1 = charToANSI(c1, c1), alt2 = charToANSI(c2, c2);
            if (alt1 != c1 || alt2 != c2) {
                if (insensitive) {
                    alt1 = charLower(alt1);
                    alt2 = charLower(alt2);
                }
                if (alt1 < alt2) return -1;
                if (alt1 > alt2) return 1;
            }

            if (insensitive) {
                c1 = charLower(c1);
                c2 = charLower(c2);
            }
            if (c1 < c2) return -1;
            if (c1 > c2) return 1;

            s1 += d1; s2 += d2;
        }
    }
    int naturalSortCompare(const char* s1, const char* s2) throw() {
        int v = naturalSortCompareInternal(s1, s2, true);
        if (v) return v;
        v = naturalSortCompareInternal(s1, s2, false);
        if (v) return v;
        return strcmp(s1, s2);
    }

    int naturalSortCompareI(const char* s1, const char* s2) throw() {
        return naturalSortCompareInternal(s1, s2, true);
    }
#ifdef _WIN32
    int winNaturalSortCompare(const char* s1, const char* s2);
    int winNaturalSortCompareI(const char* s1, const char* s2);
#endif
#ifdef __APPLE__
    int appleNaturalSortCompare(const char* s1, const char* s2);
    int appleNaturalSortCompareI(const char* s1, const char* s2);
#endif
    int sysNaturalSortCompare(const char* s1, const char* s2) {
#ifdef _WIN32
        return winNaturalSortCompare(s1, s2);
#elif defined(__APPLE__)
        return appleNaturalSortCompare(s1, s2);
#else
        return naturalSortCompare(s1, s2);
#endif
    }
    int sysNaturalSortCompareI(const char* s1, const char* s2) {
#ifdef _WIN32
        return winNaturalSortCompareI(s1, s2);
#elif defined(__APPLE__)
        return appleNaturalSortCompareI(s1, s2);
#else
        return naturalSortCompareI(s1, s2);
#endif
    }
    const char* _stringComparatorCommon::myStringToPtr(string_part_ref) {
        pfc::crash(); return nullptr;
    }

    int stringCompareCaseInsensitiveEx(string_part_ref s1, string_part_ref s2) {
        t_size w1 = 0, w2 = 0;
        for (;;) {
            unsigned c1, c2; t_size d1, d2;
            d1 = utf8_decode_char(s1.m_ptr + w1, c1, s1.m_len - w1);
            d2 = utf8_decode_char(s2.m_ptr + w2, c2, s2.m_len - w2);
            if (d1 == 0 && d2 == 0) return 0;
            else if (d1 == 0) return -1;
            else if (d2 == 0) return 1;
            else {
                c1 = charLower(c1); c2 = charLower(c2);
                if (c1 < c2) return -1;
                else if (c1 > c2) return 1;
            }
            w1 += d1; w2 += d2;
        }
    }
    int stringCompareCaseInsensitive(const char* s1, const char* s2) {
        for (;;) {
            unsigned c1, c2; t_size d1, d2;
            d1 = utf8_decode_char(s1, c1);
            d2 = utf8_decode_char(s2, c2);
            if (d1 == 0 && d2 == 0) return 0;
            else if (d1 == 0) return -1;
            else if (d2 == 0) return 1;
            else {
                c1 = charLower(c1); c2 = charLower(c2);
                if (c1 < c2) return -1;
                else if (c1 > c2) return 1;
            }
            s1 += d1; s2 += d2;
        }
    }
#ifdef PFC_SORTSTRING_GENERIC
    int sortStringCompare(const char* str1, const char* str2) {
        return naturalSortCompare(str1, str2);
    }
    int sortStringCompareI(const char* str1, const char* str2) {
        return naturalSortCompareI(str1, str2);
    }
#endif
}
