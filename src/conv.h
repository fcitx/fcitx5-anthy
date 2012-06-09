#include <iconv.h>

class IConvert {
public:
    IConvert(const char* from, const char* to);
    iconv_t conv;
};