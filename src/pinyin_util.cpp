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

#define Uses_SCIM_UTILITY

#include "pinyin_util.h"
#include <scim.h>
#include <cctype>

using std::wstring;

HalfToFullConverter::HalfToFullConverter()
    : m_is_full_width_punct(true),
      m_is_full_width_letter(false),
      m_left_single_quote(true),
      m_left_double_quote(true)
{}

wstring
HalfToFullConverter::operator() (char ch)
{
    wstring full;
    if (m_is_full_width_punct && ispunct(ch)) {
        full = half_punct_to_full(ch);
    }
    else if (m_is_full_width_letter && isalpha(ch)) {
        full.push_back(half_letter_to_full(ch));
    }
    else {
        full.push_back(ch);
    }
    return full;
}

void
HalfToFullConverter::toggle_punct_width()
{
    m_is_full_width_punct = !m_is_full_width_punct;
}

void
HalfToFullConverter::toggle_letter_width()
{
    m_is_full_width_letter = !m_is_full_width_letter;
}

bool
HalfToFullConverter::is_full_punct() const
{
    return m_is_full_width_punct;
}

bool
HalfToFullConverter::is_full_letter() const
{
    return m_is_full_width_letter;
}

void
HalfToFullConverter::reset()
{
    m_left_double_quote = true;
    m_left_single_quote = true;
}

wstring
HalfToFullConverter::half_punct_to_full(char ch)
{
    switch (ch) {
    case '.':
        return L"\x3002";
    case '\\':
        return L"\x3001";
    case '^':
        return L"\x2026\x2026";
    case '\"': {
        wstring full = m_left_double_quote ? L"\x201C" : L"\x201D";
        m_left_double_quote = !m_left_double_quote;
        return full;
    }
    case '\'': {
        wstring full = m_left_single_quote ? L"\x2018" : L"\x2019";
        m_left_single_quote = !m_left_single_quote;
        return full;
    }
    case '<':
        return L"\x300A";
    case '>':
        return L"\x300B";
    case '$':
        return L"\xFFE5";
    case '_':
        return L"\x2014\x2014";
    default: 
        wstring full;
        full.push_back(scim::scim_wchar_to_full_width(ch));
        return full;
    }
}

wchar_t
HalfToFullConverter::half_letter_to_full(char ch)
{
    return scim::scim_wchar_to_full_width(ch);
}

std::wstring
str2wstr(const std::string& str)
{
    std::wstring wstr;
    std::copy(str.begin(), str.end(), std::back_inserter(wstr));
    return wstr;
}
