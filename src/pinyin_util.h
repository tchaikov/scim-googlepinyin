#ifndef PINYIN_UTIL_H
#define PINYIN_UTIL_H

#include <string>
#include <algorithm>
#include <cwchar>

class HalfToFullConverter
{
public:
    HalfToFullConverter();
    std::wstring operator() (char ch);
    void toggle_punct_width();
    void toggle_letter_width();
    bool is_full_punct() const;
    bool is_full_letter() const;
    void reset();
    
private:
    std::wstring half_punct_to_full(char);
    wchar_t half_letter_to_full(char);

private:
    bool m_is_full_width_punct;
    bool m_is_full_width_letter;
    bool m_left_single_quote;
    bool m_left_double_quote;
};

std::wstring str2wstr(const std::string& str);

#endif // PINYIN_UTIL_H
