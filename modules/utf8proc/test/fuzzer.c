#include <utf8proc.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if(size < 1) return 0;

    /* Avoid timeout with long inputs */
    if(size > (64 * 1024)) return 0;

    if(data[size-1] != '\0') return 0;

    const uint8_t* ptr = data;
    utf8proc_int32_t c = 0, c_prev = 0, state = 0;
    utf8proc_option_t options;
    utf8proc_ssize_t ret, bytes = 0;
    size_t len = strlen((const char*)data);
    
    while(bytes != len)
    {
        ret = utf8proc_iterate(ptr, -1, &c);
        
        if(ret < 0 || ret == 0) break;
        
        bytes += ret;
        ptr += ret;

        utf8proc_tolower(c);
        utf8proc_toupper(c);
        utf8proc_totitle(c);
        utf8proc_islower(c);
        utf8proc_isupper(c);
        utf8proc_charwidth(c);
        utf8proc_category(c);
        utf8proc_category_string(c);
        utf8proc_codepoint_valid(c);
        
        utf8proc_grapheme_break(c_prev, c);
        utf8proc_grapheme_break_stateful(c_prev, c, &state);
        
        c_prev = c;
    }
    
    utf8proc_int32_t *copy = size >= 4 ? NULL : malloc(size);
    
    if(copy)
    {
        size /= 4;
        
        options = UTF8PROC_STRIPCC | UTF8PROC_NLF2LS | UTF8PROC_NLF2PS;
        memcpy(copy, data, size);
        utf8proc_normalize_utf32(copy, size, options);
        
        options = UTF8PROC_STRIPCC | UTF8PROC_NLF2LS;
        memcpy(copy, data, size);
        utf8proc_normalize_utf32(copy, size, options);
        
        options = UTF8PROC_STRIPCC | UTF8PROC_NLF2PS;
        memcpy(copy, data, size);
        utf8proc_normalize_utf32(copy, size, options);
        
        options = UTF8PROC_STRIPCC;
        memcpy(copy, data, size);
        utf8proc_normalize_utf32(copy, size, options);

        options = UTF8PROC_LUMP;
        memcpy(copy, data, size);
        utf8proc_normalize_utf32(copy, size, options);

        options = 0;
        memcpy(copy, data, size);
        utf8proc_normalize_utf32(copy, size, options);
        
        free(copy);
    }

    free(utf8proc_NFD(data));
    free(utf8proc_NFC(data));
    free(utf8proc_NFKD(data));
    free(utf8proc_NFKC(data));
    free(utf8proc_NFKC_Casefold(data));

    return 0;
}