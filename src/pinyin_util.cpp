#include "pinyin_util.h"

std::wstring
str2wstr(const std::string& str)
{
    std::wstring wstr;
    std::copy(str.begin(), str.end(), std::back_inserter(wstr));
    return wstr;
}
