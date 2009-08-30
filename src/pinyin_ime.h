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

#ifndef PINYIN_IME_H
#define PINYIN_IME_H

#include "ime_state.h"
#include "pinyin_util.h"

class PinyinDecoderService;
class CandidateView;
class ComposingView;
class GooglePyInstance;
class FunctionKeys;
class HalfToFullConverter;

namespace scim {
    struct KeyEvent;
}
using scim::KeyEvent;

/**
 * Main class of the Pinyin input method.
 */
class PinyinIME
{
    ImeState::State m_ime_state;
    DecodingInfo *m_dec_info;
    CandidateView *m_cand_view;
    ComposingView *m_cmps_view;
    GooglePyInstance *m_pinyin;
    FunctionKeys *m_func_keys;
    HalfToFullConverter m_half2full;
    
    enum InputMode {
        INPUT_CHINESE,
        INPUT_ENGLISH
    };
    int m_input_mode;
    /**
     * the absolute offset of candidate
     */
    size_t m_candidate_index;
    
public:
    PinyinIME(PinyinDecoderService *, FunctionKeys *, GooglePyInstance *);
    bool process_key(const KeyEvent& key);
    void set_candidate_page_size(unsigned page_size);
    /**
     * @param index the index in current page of the selected candidate
     */
    void choose_candidate_in_page(unsigned pos);
    void candidate_page_up();
    void candidate_page_down();
    void trigger_input_mode();
    void trigger_punct_width();
    void trigger_letter_width();
    bool is_chinese_mode() const;
    bool is_full_punct() const;
    bool is_full_letter() const;
    void reset();
    void redraw();
    /**
     * for PinyinLookupTable
     */
    const DecodingInfo *get_decoding_info() const;
    
private:
    bool process_in_chinese(const KeyEvent& key);
    bool process_state_idle(const KeyEvent& key);
    bool process_state_input(const KeyEvent& key);
    bool process_state_predict(const KeyEvent& key);
    bool process_state_edit_composing(const KeyEvent& key);
    void choose_candidate(int cand_no);
    void choose_and_update(int index);
    bool process_surface_change(const KeyEvent& key);

    void reset_to_idle_state(bool reset_inline_text=false);
    void change_to_state_composing(bool update_ui);
    void change_to_state_input(bool update_ui);
    void commit_result_text(const wstring& result_text);
    bool commit_char(char);
    void update_composing_text(bool visible);
    void input_comma_period(wstring pre_edit, char ch,
                            bool dismiss_cand_window, ImeState::State next_state);
    void show_candidate_window(bool show_composing_view);
    void dismiss_candidate_window();
    void reset_candidate_window();
    
};

#endif // PINYIN_IME_H
