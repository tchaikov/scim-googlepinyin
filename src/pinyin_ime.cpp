#define Uses_SCIM_EVENT
#include <scim.h>
#include "pinyin_decoder_service.h"
#include "pinyin_ime.h"

using namespace scim;

bool
PinyinIME::process_in_chinese(const KeyEvent& key)
{
    switch (m_ime_state) {
    case ImeState::STATE_IDLE:
        return process_state_idle(key);
    case ImeState::STATE_INPUT:
        return process_state_input(key);
    case ImeState::STATE_PREDICT:
        return process_state_predict(key);
    case ImeState::STATE_COMPOSING:
        return process_state_composing(key);
    default:
        return false;
    }
}

bool
PinyinIME::process_state_idle(const KeyEvent& key)
{
    char ch = key.get_ascii_code();
    if (ch >= 'a' && ch <= 'z' && !key.is_alt_down()) {
        m_dec_info.add_spl_char(ch, true);
        choose_and_update(-1);
        return true;
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
    char ch = key.get_ascii_code();
    if (ch >= 'a' && ch <= 'z' ||
        ch == '\'' && !m_dec_info.char_before_cursor_is_separator() ||
        key.code == SCIM_KEY_Delete) {
        return process_surface_change(key);
    }
    else if (ch == ',' || ch == '.' ) {
        input_comma_period(m_dec_info.get_current_full_sent(m_candidate_index),
                           ch, true);
        return true;
    } else if (key.code == SCIM_KEY_Left) {
        return m_im_engine->caret_left();
    } else if (key.code == SCIM_KEY_Right) {
        return m_im_engine->caret->right();
    } else if (key.code == SCIM_KEY_Up) {
        if (!m_im_engine->lookup_cursor_up()) {
            m_im_engine->enable_active_high_light(false);
            m_im_engine->set_ime_state(STATE_COMPOSING);
            m_im_engine->update_composing_text();
        }
    } else if (key.code == SCIM_KEY_Down) {
        return m_im_engine->lookup_cursor_down();
    } else if (key.code >= SCIM_KEY_0 && key.code <= SCIM_KEY_9) {
        int active_pos = key.code - SCIM_KEY_1;
        if (active_pos < m_lookup_table->get_current_page_size()) {
            active_pos += m_lookup_table->get_current_page_start();
            choose_and_update(active_pos);
        }
        return true;
    } else if (key.code == SCIM_KEY_Return) {
        m_im_engine->commit_string(m_dec_info->get_original_spl_str());
        reset_to_idle_state();
        return true;
    } else if (key.code == SCIM_KEY_space) {
        choose_candidate(-1);
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
        mDecInfo.add_spl_char(ch, true);
        choose_and_update(-1);
    } else if (ch == ',' || ch == '.' ) {
        input_comma_period(m_dec_info.get_current_full_sent(m_candidate_index),
                           ch, true);
        return true;
    } else if (key.code == SCIM_KEY_Left) {
        return m_im_engine->caret_left();
    } else if (key.code == SCIM_KEY_Right) {
        return m_im_engine->caret->right();
    } else if (key.code == SCIM_KEY_Up) {
        m_im_engine->lookup_cursor_up();
    } else if (key.code == SCIM_KEY_Down) {
        return m_im_engine->lookup_cursor_down();
    } else if (key.code >= SCIM_KEY_0 && key.code <= SCIM_KEY_9) {
        int active_pos = key.code - SCIM_KEY_1;
        if (active_pos < m_lookup_table->get_current_page_size()) {
            active_pos += m_lookup_table->get_current_page_start();
            choose_and_update(active_pos);
        }
        return true;
    } else if (key.code == SCIM_KEY_Return) {
        m_im_engine->commit_string(L"\n");
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
    if (key.code == SCIM_KEY_Down) {
        if (!m_dec_info.selection_finished()) {
            change_to_state_input(true);
        }
    } else if (key.code == SCIM_KEY_Left) {
        m_dec_info.move_cursor(-1);
    } else if (key.code == SCIM_KEY_Right) {
        m_dec_info.move_cursor(1);
    } else if (key.code == SCIM_KEY_space ||
               key.code == SCIM_KEY_Return) {
        if (m_cmps_status == SHOW_STRING_LOWERCASE) {
            commit_result_text(m_dec_info.get_original_spl_str());
        } else {
            commit_result_text(m_dec_info.get_composing_str());
        }
    } else {
        return process_surface_change(key);
    }
    return true;
}

bool
PinyinIME::process_surface_change(const KeyEvent& key)
{
    if (m_dec_info.is_spl_str_full() && key.code != SCIM_KEY_Delete) {
        return true;
    }
    char ch = key.get_ascii_code();
    
    if ((ch >= 'a' && ch <= 'z') ||
        (ch == '\'' && !m_dec_info.char_before_cursor_is_separator()) ||
        (((ch >= '0' && keyChar <= '9') || keyChar == ' ') && m_ime_state == STATE_COMPOSING)) {
        m_dec_info.add_spl_char(ch, false);
        choose_and_update(-1);
    } else if (key.code == SCIM_KEY_Delete) {
        m_dec_info.prepare_delete_before_cursor();
        choose_and_update(-1);
    } else if (key.code == SCIM_KEY_Return) {
        
    return true;
}

void
PinyinIME::choose_and_update(int index)
{
    if (m_ime_state != STATE_PREDICT) {
        // Get result candidate list, if choice_id < 0, do a new decoding.
        // If choice_id >=0, select the candidate, and get the new candidate
        // list.
        m_dec_info.choose_decoding_candidate(index);
    } else {
        // Choose a prediction item.
        m_dec_info.choose_predict_choice(index);
    }

    if (!m_dec_info.get_composing_str().empty()) {
        WideString result_str = m_dec_info.get_composing_str_active_part();
        if (m_ime_state == STATE_IDLE) {
            if (m_dec_info.get_spl_str_decoded_len() == 0) {
                m_ime_state = STATE_COMPOSING;
            } else {
                m_ime_state = STATE_INPUT;
            }
        } else {
            if (m_dec_info.selection_finished()) {
                m_ime_state = STATE_COMPOSING;
            }
        }
        m_im_engine->show_lookup_table();
    } else {
        reset();
    }
}
