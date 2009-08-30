/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef PINYIN_DECODER_SERVICE
#define PINYIN_DECODER_SERVICE

#include <list>
#include <vector>
#include <string>

#include "dictdef.h"

using std::vector;
using std::list;
using std::wstring;
using std::string;
namespace ime_pinyin {
    class MatrixSearch;
}

class PinyinDecoderService
{
public:
    PinyinDecoderService(const string& sys_dict_path,
                         const string& usr_dict_path);
    ~PinyinDecoderService();
    bool is_initialized() const;
    
public:
    size_t search(const string& spelling_str);
    void reset_search();
    size_t del_search(size_t pos, bool is_pos_in_splid, bool clear_fixed_this_step);
    
    int choose(int cand_id);
    wstring get_choice(size_t cand_id) const;
    std::list<wstring> get_choice_list(int choices_start, int choices_num,
                                       int sent_fixed_len) const;
    int get_predict_num(const wstring& fixed_str);
    wstring get_predict_item(int predict_no) const;
    std::list<wstring> get_predict_list(int predicts_start,
                                        int predicts_num) const;
    vector<int> get_spelling_start();
    size_t get_fixed_len() const;
    string get_py_str(bool decoded);
    
private:
    void init_pinyin_engine();
    void fini_pinyin_engine();

private:
    // The maximum number of the prediction items.
    enum {kMaxPredictNum = 500};
    // Used to search Pinyin string and give the best candidate.
    ime_pinyin::MatrixSearch * m_matrix_search;
    uint16_t m_predict_buf[kMaxPredictNum][ime_pinyin::kMaxPredictSize + 1];
    size_t m_predict_len;
    string m_sys_dict;
    string m_usr_dict;
};

#endif // PINYIN_DECODER_SERVICE
