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

#define Uses_SCIM_EVENT
#define Uses_SCIM_IMENGINE
#include <scim.h>
#include "decoding_info.h"
#include "pinyin_util.h"
#include "pinyin_ime.h"
#include "candidate_view.h"
#include "composing_view.h"
#include "google_imengine.h"
#include "function_keys.h"


using namespace scim;

PinyinIME::PinyinIME(PinyinDecoderService *decoder_service,
                     FunctionKeys *func_keys,
                     GooglePyInstance *pinyin)
    : m_ime_state(ImeState::STATE_IDLE),
      m_pinyin(pinyin),
      m_func_keys(func_keys),
      m_input_mode(INPUT_CHINESE),
      m_candidate_index(0)
{
    m_dec_info = new DecodingInfo(decoder_service, m_ime_state);
    m_cand_view = new CandidateView(m_pinyin, m_dec_info);
    m_cmps_view = new ComposingView(m_pinyin, m_dec_info);
}

bool
PinyinIME::process_key(const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE (3) << "process_key() " << m_ime_state << "\n";
    if (m_ime_state == ImeState::STATE_BYPASS) return false;
    if (m_func_keys->is_mode_switch_key(key)) {
        trigger_input_mode();
        return true;
    }
    m_func_keys->remember_last_key(key);
    
    if (key.is_key_release()) return true;

    if (key.code == SCIM_KEY_Escape && key.mask == 0) {
        if (m_dec_info->get_original_spl_str().empty()) {
            return false;
        } else {
            reset();
            return true;
        }
    }
    
    if (is_chinese_mode()) {
        return process_in_chinese(key);
    } else {
        // m_input_mode == INPUT_ENGLISH)
        return false;
    }
}

void
PinyinIME::trigger_input_mode()
{
    m_input_mode = !m_input_mode;
    m_pinyin->refresh_status_property(is_chinese_mode());
    reset_to_idle_state(false);
}

void
PinyinIME::trigger_punct_width()
{
    m_half2full.toggle_punct_width();
    m_pinyin->refresh_punct_property(m_half2full.is_full_punct());
}

void
PinyinIME::trigger_letter_width()
{
    m_half2full.toggle_letter_width();
    m_pinyin->refresh_letter_property(m_half2full.is_full_letter());
}

bool
PinyinIME::is_chinese_mode() const
{
    return m_input_mode == INPUT_CHINESE;
}

bool
PinyinIME::is_full_punct() const
{
    return m_half2full.is_full_punct();
}

bool
PinyinIME::is_full_letter() const
{
    return m_half2full.is_full_letter();
}

void
PinyinIME::set_candidate_page_size(unsigned page_size)
{
    m_cand_view->set_page_size(page_size);
}

void
PinyinIME::reset()
{
    reset_to_idle_state();
}

void
PinyinIME::redraw()
{
    m_cmps_view->redraw();
    m_cand_view->redraw();
}

const DecodingInfo*
PinyinIME::get_decoding_info() const
{
    return m_dec_info;
}

bool
PinyinIME::process_in_chinese(const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE (3) <<  "process_in_chinese("
                            << m_ime_state << ")\n";
    switch (m_ime_state) {
    case ImeState::STATE_IDLE:
        return process_state_idle(key);
    case ImeState::STATE_INPUT:
        return process_state_input(key);
    case ImeState::STATE_PREDICT:
        return process_state_predict(key);
    case ImeState::STATE_COMPOSING:
        return process_state_edit_composing(key);
    default:
        return false;
    }
}

bool
PinyinIME::process_state_idle(const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE (3) <<  "process_state_idle()\n";
    char ch = key.get_ascii_code();
    if (ch >= 'a' && ch <= 'z' && !key.is_alt_down() && !key.is_control_down()) {
        m_dec_info->add_spl_char(ch, true);
        choose_and_update(-1);
        return true;
    } else if (ispunct(ch)) {
        return commit_char(ch);
    }
    
    if (key.code == SCIM_KEY_Delete) {
        // XXX may need forward this key to upper level
        return false;
    }
    return false;
}

bool
PinyinIME::process_state_input(const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE (3) <<  "process_state_input()\n";
    char ch = key.get_ascii_code();
    if (key.code == SCIM_KEY_Up) {
        return m_cand_view->cursor_left();
    } else if (key.code == SCIM_KEY_Down) {
        return m_cand_view->cursor_right();
    } else if (key.code == SCIM_KEY_Left) {
        m_cand_view->enable_active_highlight(false);
        change_to_state_composing(true);
        update_composing_text(true);
        return true;
    } else if (key.code == SCIM_KEY_Home) {
        m_cand_view->enable_active_highlight(false);
        change_to_state_composing(true);
        update_composing_text(true);
        m_cmps_view->move_cursor_to_edge(true);
        return true;
    } else if (m_func_keys->is_page_up_key(key)) {
        return m_cand_view->page_up();
    } else if (m_func_keys->is_page_down_key(key)) {
        return m_cand_view->page_down();
    } else if (key.code >= SCIM_KEY_0 && key.code <= SCIM_KEY_9) {
        int active_pos = key.code - SCIM_KEY_1;
        choose_candidate_in_page(active_pos);
        return true;
    } else if (key.code == SCIM_KEY_Return) {
        commit_result_text(m_dec_info->get_composing_str());
        reset_to_idle_state();
        return true;
    } else if (key.code == SCIM_KEY_space) {
        choose_candidate(-1);
        return true;
    } else if ((ch >= 'a' && ch <= 'z') ||
               (ch == '\'' && !m_dec_info->char_before_cursor_is_separator()) ||
               key.code == SCIM_KEY_BackSpace) {
        return process_surface_change(key);
    } else if (ch == ',' || ch == '.' ) {
        input_comma_period(m_dec_info->get_current_full_sent(m_candidate_index),
                           ch, true, ImeState::STATE_IDLE);
        return true;
    }
    return false;
}

// almost the same as process_state_input
bool
PinyinIME::process_state_predict(const KeyEvent& key)
{
    // In this status, when user presses keys in [a..z], the status will
    // change to input state.
    char ch = key.get_ascii_code();
    if (ch >= 'a' && ch <= 'z') {
        change_to_state_input(true);
        m_dec_info->add_spl_char(ch, true);
        choose_and_update(-1);
    } else if (ispunct(ch)) {
        input_comma_period(m_dec_info->get_current_full_sent(m_candidate_index),
                           ch, true, ImeState::STATE_IDLE);
        return true;
    } else if (key.code == SCIM_KEY_Left) {
        return m_cand_view->cursor_left();
    } else if (key.code == SCIM_KEY_Right) {
        return m_cand_view->cursor_right();
    } else if (m_func_keys->is_page_up_key(key)) {
        m_cand_view->page_up();
    } else if (m_func_keys->is_page_down_key(key)) {
        return m_cand_view->page_down();
    } else if (key.code >= SCIM_KEY_0 && key.code <= SCIM_KEY_9) {
        int active_pos = key.code - SCIM_KEY_1;
        choose_candidate_in_page(active_pos);
        return true;
    } else if (key.code == SCIM_KEY_Return) {
        commit_result_text(L"\n");
        reset_to_idle_state();
        return true;
    } else if (key.code == SCIM_KEY_space) {
        choose_candidate(-1);
        return true;
    }
    return true;
}

bool
PinyinIME::process_state_edit_composing(const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE (3) <<  "process_state_input()\n";
    if (key.code == SCIM_KEY_Down) {
        if (!m_dec_info->selection_finished()) {
            change_to_state_input(true);
        }
    } else if (key.code == SCIM_KEY_Home) {
        m_cmps_view->move_cursor_to_edge(true);
    } else if (key.code == SCIM_KEY_End) {
        m_cmps_view->move_cursor_to_edge(false);
    } else if (key.code == SCIM_KEY_Left) {
        m_cmps_view->move_cursor(-1);
    } else if (key.code == SCIM_KEY_Right) {
        m_cmps_view->move_cursor(1);
    } else if (key.code == SCIM_KEY_space ||
               key.code == SCIM_KEY_Return) {
        commit_result_text(m_dec_info->get_composing_str());
    } else {
        return process_surface_change(key);
    }
    return true;
}

bool
PinyinIME::process_surface_change(const KeyEvent& key)
{
    if (m_dec_info->is_spl_str_full() && key.code != SCIM_KEY_BackSpace) {
        return true;
    }
    char ch = key.get_ascii_code();
    
    if ((ch >= 'a' && ch <= 'z') ||
        (ch == '\'' && !m_dec_info->char_before_cursor_is_separator()) ||
        ( ((ch >= '0' && ch <= '9') || ch == ' ') &&
          m_ime_state == ImeState::STATE_COMPOSING)) {
        m_dec_info->add_spl_char(ch, false);
        choose_and_update(-1);
    } else if (key.code == SCIM_KEY_BackSpace) {
        m_dec_info->prepare_delete_before_cursor();
        choose_and_update(-1);
    } else if (key.code == SCIM_KEY_Delete) {
        m_dec_info->prepare_delete_after_cursor();
        choose_and_update(-1);
    }

    return true;
}

bool
PinyinIME::commit_char(char punct)
{
    if (m_half2full.is_full_letter() ||
        m_half2full.is_full_punct()) {
        SCIM_DEBUG_IMENGINE (3) << "commit_result_text() == full_letter or full_punct\n";
        commit_result_text(m_half2full(punct));
        return true;
    }
    return false;
}

void
PinyinIME::commit_result_text(const wstring& result_text)
{
    wstring text(result_text);
    
    
    m_pinyin->commit_string(text);
    m_cmps_view->set_visibility(false);
}

void
PinyinIME::update_composing_text(bool visible)
{
    if (visible) {
        m_cmps_view->set_decoding_info(m_dec_info,
                                       m_ime_state);
    }
    m_cmps_view->set_visibility(visible);
}

void
PinyinIME::choose_candidate_in_page(unsigned index)
{
    const int current_page = m_cand_view->get_current_page();
    if (index < m_dec_info->get_current_page_size(current_page)) {
        index += m_dec_info->get_current_page_start(current_page);
        choose_and_update(index);
    }
}

void
PinyinIME::candidate_page_up()
{
    m_cand_view->page_up();
}

void
PinyinIME::candidate_page_down()
{
    m_cand_view->page_down();
}

// see PinyinInstance::lookup_select()
// @param index the candidate index in the lookup table
void
PinyinIME::choose_and_update(int cand_id)
{
    if (m_input_mode != INPUT_CHINESE) {
        wstring choice = m_dec_info->get_candidate(cand_id);
        if (!choice.empty()) {
            commit_result_text(choice);
        }
        reset_to_idle_state(false);
        return;
    }
    
    if (m_ime_state != ImeState::STATE_PREDICT) {
        // Get result candidate list, if choice_id < 0, do a new decoding.
        // If choice_id >=0, select the candidate, and get the new candidate
        // list.
        m_dec_info->choose_decoding_candidate(cand_id);
    } else {
        // Choose a prediction item.
        m_dec_info->choose_predict_choice(cand_id);
    }
    
    if (m_dec_info->get_composing_str().empty()) {
        reset_to_idle_state(false);
        return;
    }
    
    wstring result_str = m_dec_info->get_composing_str_active_part();
    if (cand_id >= 0 && m_dec_info->can_do_prediction()) {
        commit_result_text(result_str);
        m_ime_state = ImeState::STATE_PREDICT;
        m_dec_info->reset_candidates();
        if (m_dec_info->get_candidates_number() > 0) {
            show_candidate_window(false);
        } else {
            reset_to_idle_state(false);
        }
    } else {
        if (m_ime_state == ImeState::STATE_IDLE) {
            if (m_dec_info->get_spl_str_decoded_len() == 0) {
                change_to_state_composing(true);
            } else {
                change_to_state_input(true);
            }
        } else {
            if (m_dec_info->selection_finished()) {
                change_to_state_composing(true);
            }
        }
        show_candidate_window(true);
    }
    
}

void
PinyinIME::choose_candidate(int cand_no)
{
    if (cand_no < 0) {
        cand_no = m_cand_view->get_active_candidate_pos();
    }
    if (cand_no >= 0) {
        choose_and_update(cand_no);
    }
}

void
PinyinIME::change_to_state_composing(bool)
{
    m_ime_state = ImeState::STATE_COMPOSING;
}

void
PinyinIME::input_comma_period(wstring pre_edit, char ch,
                              bool dismiss_cand_window, ImeState::State next_state)
{
    if (ispunct(ch)) {
        pre_edit += m_half2full(ch);
    } else {
        return;
    }
    commit_result_text(pre_edit);
    if (dismiss_cand_window) reset_candidate_window();
    m_ime_state = next_state;
}

    
void
PinyinIME::reset_to_idle_state(bool)
{
    if (ImeState::STATE_IDLE == m_ime_state) return;

    m_ime_state = ImeState::STATE_IDLE;
    m_dec_info->reset();
    m_cmps_view->reset();
    reset_candidate_window();
}

void
PinyinIME::change_to_state_input(bool)
{
    m_ime_state = ImeState::STATE_INPUT;
    show_candidate_window(true);
}

void
PinyinIME::show_candidate_window(bool show_composing_view)
{
    m_cand_view->set_visibility(true);
    update_composing_text(show_composing_view);
    m_cand_view->show_candidates(m_dec_info,
                                 ImeState::STATE_COMPOSING != m_ime_state);
}

void
PinyinIME::dismiss_candidate_window()
{
    m_cand_view->set_visibility(false);
}

void
PinyinIME::reset_candidate_window()
{
    m_dec_info->reset_candidates();
    show_candidate_window(false);
    m_cand_view->set_visibility(false);
}

