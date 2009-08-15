#include <cassert>

#include "decoding_info.h"

wstring
str2wstr(const string& str)
{
    wstring wstr;
    copy(str.begin(), str.end(), std::back_inserter(wstr));
    return wstr;
}

DecodingInfo::DecodingInfo()
    : m_surface_decoded_len(0), m_pos_del_spl(-1)
{}

void
DecodingInfo::reset()
{
    m_surface.clear();
    m_surface_decoded_len = 0;
    m_cursor_pos = 0;
    m_full_sent.clear();
    m_fixed_len = 0;
    m_finish_selection = false;
    m_composing_str.clear();
    m_composing_str_display.clear();
    m_active_cmps_len = 0;
    m_active_cmps_display_len = 0;

    reset_candidates();
}

bool
DecodingInfo::is_spl_str_full() const
{
    return (m_surface.length() >= PY_STRING_MAX - 1);
}


void
DecodingInfo::add_spl_char(char ch, bool reset)
{
    if (reset) {
        m_surface.clear();
        m_surface_decoded_len = 0;
        m_cursor_pos = 0;
        m_pinyin_decode_service.reset_search();
    }
    m_surface.insert(m_cursor_pos, 1, ch);
    m_cursor_pos++;
}

void
DecodingInfo::prepare_delete_before_cursor()
{
    if (m_cursor_pos > 0) {
        for (int pos = 0; pos < m_fixed_len; pos++) {
            if (m_spl_start[pos + 2] >= m_cursor_pos &&
                m_spl_start[pos + 1] < m_cursor_pos) {
                m_pos_del_spl = pos;
                m_cursor_pos = m_spl_start[pos + 1];
                m_is_pos_in_spl = true;
                break;
            }
        }
        if (m_pos_del_spl < 0) {
            m_pos_del_spl = m_cursor_pos - 1;
            m_cursor_pos--;
            m_is_pos_in_spl = false;
        }
    }
}

int
DecodingInfo::length() const
{
    return m_surface.length();
}

string
DecodingInfo::get_original_spl_str() const
{
    return m_surface;
}

int
DecodingInfo::get_spl_str_decoded_len() const
{
    return m_surface_decoded_len;
}

wstring
DecodingInfo::get_composing_str() const
{
    return m_composing_str;
}

wstring
DecodingInfo::get_composing_str_active_part() const
{
    assert (m_active_cmps_len <= m_composing_str.length());
    return m_composing_str.substr(0, m_active_cmps_len);
}

wstring
DecodingInfo::get_composing_str_for_display() const
{
    return m_composing_str_display;
}

int
DecodingInfo::get_active_cmps_display_len() const
{
    return m_active_cmps_len;
}

wstring
DecodingInfo::get_full_sent() const
{
    return m_full_sent;
}

wstring
DecodingInfo::get_current_full_sent(int active_cand_pos) const
{
    wstring str = m_full_sent.substr(0, m_fixed_len);
    str += m_candidates_list[active_cand_pos];
    return str;
}

void
DecodingInfo::reset_candidates()
{
    m_candidates_list.clear();
    m_total_choices_num = 0;

    m_page_start.clear();
    m_page_start.push_back(0);
    
    m_cn_to_page.clear();
    m_cn_to_page.push_back(0);
}

bool
DecodingInfo::can_do_prediction() const
{
    return m_composing_str.length() == m_fixed_len;
}

bool
DecodingInfo::selection_finished() const
{
    return m_finish_selection;
}

void
DecodingInfo::choose_candidate(int cand_id)
{
    if (m_ime_state == STATE_PREDICT)
        return;
    
    reset_candidates();
    int n_candidates = 0;
    if (cand_id < 0) {
        if (length() == 0) {
            n_candidates = 0;
        } else {
            if (m_pos_del_spl < 0) {
                n_candidates = m_pinyin_decode_service.search(m_surface);
            } else {
                bool clear_fixed_this_step = true;
                if (m_ime_state == STATE_COMPOSING) {
                    clear_fixed_this_step = false;
                }
                n_candidates = m_pinyin_decode_service.del_search(
                    m_pos_del_spl, m_is_pos_in_spl, clear_fixed_this_step);
                m_pos_del_spl = -1;
            }
        }
    } else {
        n_candidates = m_pinyin_decode_service.choose(cand_id);
    }
    update_for_search(n_candidates);
}

void
DecodingInfo::update_for_search(int n_candidates) {
    m_total_choices_num = n_candidates;
    if (m_total_choices_num < 0) {
        m_total_choices_num = 0;
        return;
    }
    m_spl_start = m_pinyin_decode_service.get_spelling_start();
    string py_str = m_pinyin_decode_service.get_py_str(false);
    m_surface_decoded_len = py_str.length();
    
    m_full_sent = m_pinyin_decode_service.get_choice(0);
    m_fixed_len = m_pinyin_decode_service.get_fixed_len();
    
    // Update the surface string to the one kept by engine.
    m_surface.swap(py_str);

    if (m_cursor_pos > m_surface.length())
        m_cursor_pos = m_surface.length();
    m_composing_str = m_full_sent.substr(0, m_fixed_len) +
        str2wstr(m_surface.substr(m_spl_start[m_fixed_len + 1]));
    
    m_active_cmps_len = m_composing_str.length();
    if (m_surface_decoded_len > 0) {
        m_active_cmps_len -= (m_surface.length() - m_surface_decoded_len);
    }

     // Prepare the display string.
     if (0 == m_surface_decoded_len) {
         m_composing_str_display = m_composing_str;
         m_active_cmps_display_len = m_composing_str.length();
     } else {
         m_composing_str_display = m_full_sent.substr(0, m_fixed_len);
         for (int pos = m_fixed_len + 1; pos < m_spl_start.size() - 1; ++pos) {
             m_composing_str_display +=
                 str2wstr(m_surface.substr(m_spl_start[pos],
                                           m_spl_start[pos + 1]));
             if (m_spl_start[pos + 1] < m_surface_decoded_len) {
                 m_composing_str_display += L" ";
             }
         }
         m_active_cmps_display_len = m_composing_str_display.length();
         if (m_surface_decoded_len < m_surface.length()) {
             m_composing_str_display += str2wstr(m_surface.substr(m_surface_decoded_len));
         }
     }
     if (m_spl_start.size() == m_fixed_len + 2) {
         m_finish_selection = true;
     } else {
         m_finish_selection = false;
     }
     // Prepare page 0.
     if (!m_finish_selection) {
         prepare_page(0);
     }
}

bool
DecodingInfo::prepare_page(int page_no)
{
    // If the page number is less than 0, return false
    if (page_no < 0) return false;

    // Make sure the starting information for page pageNo is ready.
    if (m_page_start.size() <= page_no) {
        return false;
    }
    
    // Page page_no's ending information is also ready.
    if (m_page_start.size() > page_no + 1) {
        return true;
    }
    
    // If cached items is enough for page pageNo.
    if (m_candidates_list.size() - m_page_start[page_no] >= MAX_PAGE_SIZE_DISPLAY) {
        return true;
    }
    
    // Try to get more items from engine
    get_candidates_for_cache();
    
    // Try to find if there are available new items to display.
    // If no new item, return false;
    if (m_page_start[page_no] >= m_candidates_list.size()) {
        return false;
    }
    
    // If there are new items, return true;
    return true;
}

void
DecodingInfo::choose_predict_choice(int choice)
{
    if (m_ime_state != STATE_PREDICT ||
        choice < 0 ||
        choice >= m_total_choices_num) {
        return;
    }

    wstring tmp(m_candidates_list[choice]);

    reset_candidates();
    
    m_candidates_list.push_back(tmp);
    m_total_choices_num = 1;

    m_surface.clear();
    m_cursor_pos = 0;
    m_full_sent = tmp;
    m_fixed_len = tmp.length();
    m_composing_str = m_full_sent;
    m_active_cmps_len = m_fixed_len;

    m_finish_selection = true;
}

wstring
DecodingInfo::get_candidate(int candId) const
{
    // Only loaded items can be gotten, so we use mCandidatesList.size()
    // instead mTotalChoiceNum.
    if (candId < 0 || candId > m_candidates_list.size()) {
        return wstring();
    }
    return m_candidates_list[candId];
}

size_t
DecodingInfo::get_candidates_number() const
{
    return m_candidates_list.size();
}

void
DecodingInfo::get_candidates_for_cache()
{
    int fetch_start = m_candidates_list.size();
    int fetch_size = m_total_choices_num - fetch_start;
    if (fetch_size > MAX_PAGE_SIZE_DISPLAY) {
        fetch_size = MAX_PAGE_SIZE_DISPLAY;
    }

    list<wstring> new_list;
    if (STATE_INPUT == m_ime_state ||
        STATE_IDLE == m_ime_state ||
        STATE_COMPOSING == m_ime_state) {
        new_list = m_pinyin_decode_service.get_choice_list(
            fetch_start, fetch_size, m_fixed_len);
    } else if (STATE_PREDICT == m_ime_state) {
        new_list = m_pinyin_decode_service.get_predict_list(
                            fetch_start, fetch_size);
    }
    copy(new_list.begin(), new_list.end(), std::back_inserter(m_candidates_list));
}
