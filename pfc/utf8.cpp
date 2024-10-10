#include "pfc-lite.h"
#include "string_base.h"

namespace pfc {
//utf8 stuff
#include "pocket_char_ops.h"

#ifdef _MSC_VER
    t_size utf16_decode_char(const wchar_t * p_source,unsigned * p_out,t_size p_source_length) throw() {
        PFC_STATIC_ASSERT( sizeof(wchar_t) == sizeof(char16_t) );
        return wide_decode_char( p_source, p_out, p_source_length );
    }
    t_size utf16_encode_char(unsigned c,wchar_t * out) throw() {
        PFC_STATIC_ASSERT( sizeof(wchar_t) == sizeof(char16_t) );
        return wide_encode_char( c, out );
    }
#endif

    t_size wide_decode_char(const wchar_t * p_source,unsigned * p_out,t_size p_source_length) throw() {
        PFC_STATIC_ASSERT( sizeof( wchar_t ) == sizeof( char16_t ) || sizeof( wchar_t ) == sizeof( unsigned ) );
        if constexpr (sizeof( wchar_t ) == sizeof( char16_t ) ) {
            return utf16_decode_char( reinterpret_cast< const char16_t *>(p_source), p_out, p_source_length );
        } else {
            if (p_source_length == 0) { * p_out = 0; return 0; }
            * p_out = p_source [ 0 ];
            return 1;
        }
    }
	t_size wide_encode_char(unsigned c,wchar_t * out) throw() {
        PFC_STATIC_ASSERT( sizeof( wchar_t ) == sizeof( char16_t ) || sizeof( wchar_t ) == sizeof( unsigned ) );
        if constexpr (sizeof( wchar_t ) == sizeof( char16_t ) ) {
            return utf16_encode_char( c, reinterpret_cast< char16_t * >(out) );
        } else {
            * out = (wchar_t) c;
            return 1;
        }
    }

    size_t uni_decode_char(const char16_t * p_source, unsigned & p_out, size_t p_source_length) noexcept {
        return utf16_decode_char(p_source, &p_out, p_source_length);
    }
    size_t uni_decode_char(const char * p_source, unsigned & p_out, size_t p_source_length) noexcept {
        return utf8_decode_char(p_source, p_out, p_source_length);
    }
    size_t uni_decode_char(const wchar_t * p_source, unsigned & p_out, size_t p_source_length) noexcept {
        if constexpr ( sizeof(wchar_t) == sizeof(char16_t)) {
            return utf16_decode_char( reinterpret_cast<const char16_t*>(p_source), &p_out, p_source_length);
        } else {
            if (p_source_length > 0) {
                unsigned c = (unsigned)*p_source;
                if (c != 0) {
                    p_out = c; return 1;
                }
            }
            p_out = 0; return 0;
        }
    }

    size_t uni_char_length(const char * arg) {
        return utf8_char_len(arg);
    }
    size_t uni_char_length(const char16_t * arg) {
        unsigned dontcare;
        return utf16_decode_char(arg, &dontcare);
    }
    size_t uni_char_length(const wchar_t * arg) {
        if constexpr ( sizeof(wchar_t) == sizeof(char16_t) ) {
            unsigned dontcare;
            return utf16_decode_char(reinterpret_cast<const char16_t*>(arg), &dontcare);
        } else {
            return *arg == 0 ? 0 : 1;
        }
    }
    
    size_t uni_encode_char(unsigned c, char* out) noexcept {
        PFC_ASSERT(c != 0);
        return utf8_encode_char(c, out);
    }
    size_t uni_encode_char(unsigned c, char16_t* out) noexcept {
        PFC_ASSERT(c != 0);
        return utf16_encode_char(c, out);
    }
    size_t uni_encode_char(unsigned c, wchar_t* out) noexcept {
        PFC_ASSERT(c != 0);
        if constexpr ( sizeof(wchar_t) == sizeof(char16_t)) {
            return utf16_encode_char(c, reinterpret_cast<char16_t*>(out));
        } else {
            *out = c; return 1;
        }
    }


bool is_lower_ascii(const char * param)
{
	while(*param)
	{
		if (*param<0) return false;
		param++;
	}
	return true;
}

static bool check_end_of_string(const char * ptr)
{
	return !*ptr;
}

size_t strcpy_utf8_truncate(const char * src,char * out,size_t maxbytes)
{
	size_t rv = 0 , ptr = 0;
	if (maxbytes>0)
	{	
		maxbytes--;//for null
		while(!check_end_of_string(src) && maxbytes>0)
		{
            size_t delta = utf8_char_len(src);
            if (delta>maxbytes || delta==0) break;
			maxbytes -= delta;
            do 
            {
                out[ptr++] = *(src++);
            } while(--delta);
			rv = ptr;
		}
		out[rv]=0;
	}
	return rv;
}

t_size strlen_utf8(const char * p,t_size num) noexcept
{
	unsigned w;
	t_size d;
	t_size ret = 0;
	for(;num;)
	{
		d = utf8_decode_char(p,w);
		if (w==0 || d<=0) break;
		ret++;
		p+=d;
		num-=d;
	}
	return ret;
}

t_size utf8_chars_to_bytes(const char * string,t_size count) noexcept
{
	t_size bytes = 0;
	while(count)
	{
		unsigned dummy;
		t_size delta = utf8_decode_char(string+bytes,dummy);
		if (delta==0) break;
		bytes += delta;
		count--;
	}
	return bytes;
}

}