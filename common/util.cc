#include <util.h>


namespace util {

#define SPEW()                                                                  \
            do {                                                                \
                va_list args;                                                   \
                va_start(args, format);                                         \
                int32_t len = vsnprintf(buf, sizeof(buf), format, args);        \
                va_end(args);                                                   \
                                                                                \
                if ((len == -1) || (len >= SIZE_MID_BLAH)) {                    \
                    len = SIZE_MID_BLAH - 1;                                    \
                    buf[len] = 0;                                               \
                } else if(len == 0) {                                           \
                    len = 0;                                                    \
                    buf[0] = 0;                                                 \
                }                                                               \
                std::cout << buf << std::endl;                                  \
            } while (0);


void SpewMsg(const char* file, const char* func, const int line,
             const char* format, ...)
{
    char buf[SIZE_MID_BLAH];
    SPEW()
    sprintf(buf, "\t\tat line: %d  in function: %s/%s", line, file, func);
    std::cout << buf << std::endl;
}

void SpewMsg(const char* format, ...)
{
    char buf[SIZE_MID_BLAH];
    SPEW()
}

}
