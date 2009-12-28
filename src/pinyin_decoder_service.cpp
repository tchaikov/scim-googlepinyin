/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include <cassert>
#include <algorithm>
#include <cstring>

#include <scim.h>
#include "matrixsearch.h"
#include "userdict.h"

#include "pinyin_decoder_service.h"

using namespace ime_pinyin;

static const int RET_BUF_LEN = 256;
static char16 retbuf[RET_BUF_LEN];
static char16 (*predict_buf)[kMaxPredictSize + 1] = NULL;

// TODO: to sync user dict with sync server
//static Sync sync_worker;

wstring char16_to_wstr(char16* s, size_t len)
{
    wstring str;
    copy(s, s+len, std::back_inserter(str));
    return str;
}

PinyinDecoderService::PinyinDecoderService(const string& sys_dict_path,
                                           const string& usr_dict_path)
    : m_matrix_search(NULL),
      m_sys_dict(sys_dict_path),
      m_usr_dict(usr_dict_path)
{
    init_pinyin_engine();
}

bool
PinyinDecoderService::is_initialized() const
{
    return m_matrix_search != NULL;
}

PinyinDecoderService::~PinyinDecoderService()
{
    fini_pinyin_engine();
}

size_t
PinyinDecoderService::search(const string& spelling_str)
{
    m_matrix_search->search(spelling_str.c_str(), spelling_str.length());
    return m_matrix_search->get_candidate_num();
}

void
PinyinDecoderService::reset_search()
{
    m_matrix_search->reset_search();
}

size_t
PinyinDecoderService::del_search(size_t pos, bool is_pos_in_splid, bool clear_fixed_this_step)
{
    m_matrix_search->delsearch(pos, is_pos_in_splid, clear_fixed_this_step);
    return m_matrix_search->get_candidate_num();
}

int
PinyinDecoderService::choose(int choice)
{
    return m_matrix_search->choose(choice);
}

vector<int>
PinyinDecoderService::get_spelling_start()
{
    const unsigned short *spelling_start;
    size_t len = m_matrix_search->get_spl_start(spelling_start);
    vector<int> spl_start;
    // element 0 is used to store the length of buffer.
    spl_start.push_back(len);
    // There will be len + 1 elements in the buffer when len > 0.
    copy(spelling_start, spelling_start + len + 1, back_inserter(spl_start));
    return spl_start;
}

int 
PinyinDecoderService::get_predict_num(const wstring& fixed_str)
{
    const wchar_t *fixed_ptr = fixed_str.c_str();
    size_t fixed_len = fixed_str.length();
    char16 fixed_buf[kMaxPredictSize + 1];
    
    if (fixed_len > kMaxPredictSize) {
        fixed_ptr += fixed_len - kMaxPredictSize;
        fixed_len = kMaxPredictSize;
    }
    std::copy(fixed_ptr, fixed_ptr + fixed_len, fixed_buf);
    fixed_buf[fixed_len] = (char16)'\0';
    m_predict_len = m_matrix_search->get_predicts(fixed_buf, predict_buf, kMaxPredictSize);
    return m_predict_len;
}

wstring
PinyinDecoderService::get_predict_item(int predict_no) const
{
    wstring str;
    if (predict_no < 0 || (size_t)predict_no >= m_predict_len) {
        str = char16_to_wstr(predict_buf[0], 0);
    } else {
        str = char16_to_wstr(predict_buf[predict_no],
                             utf16_strlen(predict_buf[predict_no]));
    }
    return str;
}

list<wstring>
PinyinDecoderService::get_predict_list(int predicts_start, int predicts_num) const
{
    list<wstring> predict_list;
    for (int i = predicts_start; i < predicts_start + predicts_num; i++) {
        predict_list.push_back(get_predict_item(i));
    }
    return predict_list;
}

wstring
PinyinDecoderService::get_choice(size_t index) const
{
    if (m_matrix_search->get_candidate(index, retbuf, RET_BUF_LEN)) {
        return char16_to_wstr(retbuf, utf16_strlen(retbuf));
    } else {
        return char16_to_wstr(retbuf, 0);
    }
}


list<wstring>
PinyinDecoderService::get_choice_list(int choices_start, int choices_num,
                                      int sent_fixed_len) const
{
    list<wstring> choice_list;
    for (int i = choices_start; i < choices_start + choices_num; i++) {
        choice_list.push_back(get_choice(i));
    }
    return choice_list;
}

size_t
PinyinDecoderService::get_fixed_len() const
{
    return m_matrix_search->get_fixedlen();
}

string
PinyinDecoderService::get_py_str(bool decoded)
{
    SCIM_DEBUG_IMENGINE (3) << "PinyinDecoderService::get_py_str(" << decoded << ")\n";
    size_t py_len;
    const char *py = m_matrix_search->get_pystr(&py_len);
    assert(py != NULL);
    SCIM_DEBUG_IMENGINE (3) << "py = " << py <<"\n";
    if (!decoded)
        py_len = strlen(py);
    
    const unsigned short *spl_start;
    size_t len;
    len = m_matrix_search->get_spl_start(spl_start);

    return string(py, py_len);
}

void
PinyinDecoderService::init_pinyin_engine()
{
    if (!m_matrix_search)
        delete m_matrix_search;
    m_matrix_search = NULL;
    m_matrix_search = new MatrixSearch();
    m_matrix_search->init(m_sys_dict.c_str(),
                          m_usr_dict.c_str());
}

void
PinyinDecoderService::fini_pinyin_engine()
{
    assert(m_matrix_search);
    m_matrix_search->close();
    delete m_matrix_search;
    m_matrix_search = NULL;
}
