
#include "mirror/string-inl.h"
#include "mirror/array-inl.h"
#include "utf.h"


namespace art {

std::string String::ToModifiedUtf8(const String *real)
{
    size_t str_len = static_cast<size_t>(GetLength(real));
    const Array* char_array = GetCharArray(real);

    size_t array_capacity = static_cast<size_t>(Array::GetCapacity(char_array));
    const uint16_t* utf16_in = Array::GetData(char_array);

    char utf8_out[array_capacity];
    memset(utf8_out, 0, sizeof(char) * array_capacity);
    ConvertUtf16ToModifiedUtf8(utf8_out, utf16_in, str_len);
    return std::string(utf8_out);
}

}