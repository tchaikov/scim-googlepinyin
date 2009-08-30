/*
 * Copyright (C) 2009 Kov Chai <tchaikov@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
