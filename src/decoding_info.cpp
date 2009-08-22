#include <cassert>

#include "pinyinime.h"
#include "pinyin_decoder_service.h"
#include "decoding_info.h"
#include "candidate_view.h"
#include "pinyin_util.h"

DecodingInfo::DecodingInfo(PinyinDecoderService *decoder_service,
                           const ImeState::State& ime_state)
    : m_surface_decoded_len(0),
      m_pos_del_spl(-1),
      m_decoder_service(decoder_service),
      m_ime_state(ime_state)
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
        m_decoder_service->reset_search();
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

wstring
DecodingInfo::get_original_spl_str() const
{
    return str2wstr(m_surface);
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

// XXX: this is a part of page_table
void
DecodingInfo::calculate_page(int page_no, CandidateView* cand_view)
{
    int from_page = m_page_start.size() - 1;
    if (m_page_start.size() > page_no - 1)
        return;

    const int cand_size = m_candidates_list.size();
    const int page_size = cand_view->get_page_size();
    for (int p = from_page; p <= page_no; p++) {
        int p_start = m_page_start[p];
        int p_size = 0;
        while (p_start + p_size < cand_size && p_size < page_size) {
            const int item_pos = p_start + p_size;
            const wstring& item_str = m_candidates_list[item_pos];
            cand_view->append_candidate(item_str);
            p_size++;
        }
        m_page_start.push_back(p_start + p_size);
    }
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
DecodingInfo::choose_decoding_candidate(int cand_id)
{
    if (m_ime_state == ImeState::STATE_PREDICT)
        return;
    
    reset_candidates();
    int n_candidates = 0;
    if (cand_id < 0) {
        if (length() == 0) {
            n_candidates = 0;
        } else {
            if (m_pos_del_spl < 0) {
                n_candidates = m_decoder_service->search(m_surface);
            } else {
                bool clear_fixed_this_step = true;
                if (m_ime_state == ImeState::STATE_COMPOSING) {
                    clear_fixed_this_step = false;
                }
                n_candidates = m_decoder_service->del_search(
                    m_pos_del_spl, m_is_pos_in_spl, clear_fixed_this_step);
                m_pos_del_spl = -1;
            }
        }
    } else {
        n_candidates = m_decoder_service->choose(cand_id);
    }
    update_for_search(n_candidates);
}

void
DecodingInfo::update_for_search(int n_candidates) {
    SCIM_DEBUG_IMENGINE (3) << "update_for_search("
                            << n_candidates << ")\n";
    m_total_choices_num = n_candidates;
    if (m_total_choices_num < 0) {
        m_total_choices_num = 0;
        return;
    }
    m_spl_start = m_decoder_service->get_spelling_start();
    string py_str = m_decoder_service->get_py_str(false);
    m_surface_decoded_len = py_str.length();
    
    m_full_sent = m_decoder_service->get_choice(0);
    m_fixed_len = m_decoder_service->get_fixed_len();
    
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
     SCIM_DEBUG_IMENGINE (3) << m_finish_selection
                             << "\n";
     if (!m_finish_selection) {
         prepare_page(0);
     }
}

// m_page_start is updated in CandidateView.calculatePage
bool
DecodingInfo::page_ready(int page_no) const
{
    if (page_no < 0) return false;
    
    // Page page_no's ending information is not ready.
    if (m_page_start.size() <= page_no + 1) {
        return false;
    }
    
    return true;
}

bool
DecodingInfo::prepare_page(int page_no)
{
    SCIM_DEBUG_IMENGINE (3) << "prepare_page("
                            << page_no << ")\n";
    // If the page number is less than 0, return false
    if (page_no < 0) return false;

    SCIM_DEBUG_IMENGINE (3) << "m_page_start.size() => "
                            << m_page_start.size() << "\n";
    // Make sure the starting information for page pageNo is ready.
    if (m_page_start.size() <= page_no) {
        return false;
    }
    
    // Page page_no's ending information is also ready.
    if (m_page_start.size() > page_no + 1) {
        return true;
    }

    SCIM_DEBUG_IMENGINE (3) << m_candidates_list.size() << "\n";
    SCIM_DEBUG_IMENGINE (3) << m_page_start[page_no] << "\n";
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
    if (m_ime_state != ImeState::STATE_PREDICT ||
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
DecodingInfo::get_candidate(int cand_id) const
{
    // Only loaded items can be gotten, so we use mCandidatesList.size()
    // instead mTotalChoiceNum.
    if (cand_id < 0 || cand_id > m_candidates_list.size()) {
        return wstring();
    }
    return m_candidates_list[cand_id];
}

size_t
DecodingInfo::get_candidates_number() const
{
    return m_total_choices_num;
}

void
DecodingInfo::get_candidates_for_cache()
{
    SCIM_DEBUG_IMENGINE (3) << __PRETTY_FUNCTION__ << "\n";
    int fetch_start = m_candidates_list.size();
    int fetch_size = m_total_choices_num - fetch_start;
    if (fetch_size > MAX_PAGE_SIZE_DISPLAY) {
        fetch_size = MAX_PAGE_SIZE_DISPLAY;
    }

    list<wstring> new_list;

    SCIM_DEBUG_IMENGINE (3) << "m_ime_state = " << m_ime_state << "\n";
    if (ImeState::STATE_INPUT == m_ime_state ||
        ImeState::STATE_IDLE == m_ime_state ||
        ImeState::STATE_COMPOSING == m_ime_state) {
        new_list = m_decoder_service->get_choice_list(
            fetch_start, fetch_size, m_fixed_len);
        SCIM_DEBUG_IMENGINE (3) << new_list.size() << " candidates gotten\n";
    } else if (ImeState::STATE_PREDICT == m_ime_state) {
        new_list = m_decoder_service->get_predict_list(
            fetch_start, fetch_size);
        SCIM_DEBUG_IMENGINE (3) << new_list.size() << " predicts gotten\n";
    }
    copy(new_list.begin(), new_list.end(), std::back_inserter(m_candidates_list));
}

void
DecodingInfo::move_cursor_to_edge(bool left)
{
    m_cursor_pos = left ? 0 : m_surface.length();
}

// count in the number of spaces in displayed composing string
int
DecodingInfo::get_cursor_pos_in_cmps_display() const
{
    int cursor_pos = get_cursor_pos_in_cmps();
    // +2 is because: one for mSplStart[0], which is used for other
    // purpose (The length of the segmentation string), and another
    // for the first spelling which does not need a space before it.
    for (int pos = m_fixed_len + 2; pos < m_spl_start.size() - 1; pos++) {
        if (m_cursor_pos <= m_spl_start[pos]) {
            break;
        } else {
            cursor_pos++;
        }
    }
    return cursor_pos;
}

int
DecodingInfo::get_cursor_pos_in_cmps() const
{
    int cursor_pos = m_cursor_pos;
    int fixed_len = 0;

    for (int hz_pos = 0; hz_pos < m_fixed_len; hz_pos++) {
        if (m_cursor_pos >= m_spl_start[hz_pos + 2]) {
            cursor_pos -= m_spl_start[hz_pos + 2] - m_spl_start[hz_pos + 1];
            cursor_pos += 1;
        }
    }
    return cursor_pos;
}
    

// Move cursor. If offset is 0, this function can be used to adjust
// the cursor into the bounds of the string.
void
DecodingInfo::move_cursor(int offset)
{
    assert(offset >= -1 && offset >= 1);
    
    if (offset != 0) {
        int hz_pos = 0;
        for (hz_pos = 0; hz_pos <= m_fixed_len; hz_pos++) {
            if (m_cursor_pos == m_spl_start[hz_pos + 1]) {
                if (offset < 0) {
                    if (hz_pos > 0) {
                        offset = m_spl_start[hz_pos] -
                                 m_spl_start[hz_pos + 1];
                    }
                } else {
                    if (hz_pos < m_fixed_len) {
                        offset = m_spl_start[hz_pos + 2] -
                                 m_spl_start[hz_pos + 1];
                    }
                }
                break;
            }
        }
    }
    m_cursor_pos += offset;
    if (m_cursor_pos < 0) {
        m_cursor_pos = 0;
    } else if (m_cursor_pos > m_surface.length()) {
        m_cursor_pos = m_surface.length();
    }
}

size_t
DecodingInfo::get_current_page_size(int current_page) const
{
    if (m_page_start.size() <= current_page + 1) return 0;
    return m_page_start[current_page + 1] - m_page_start[current_page];
}

size_t
DecodingInfo::get_current_page_start(int current_page) const
{
    if (m_page_start.size() < current_page + 1) return m_total_choices_num;
    return m_page_start[current_page];
}

bool
DecodingInfo::page_forwardable(int current_page) const
{
    if (m_page_start.size() <= current_page + 1) return false;
    if (m_page_start[current_page + 1] >= m_total_choices_num) {
        return false;
    }
    return true;
}

bool
DecodingInfo::page_backwardable(int current_page) const
{
    if (current_page > 0) return true;
    return false;
}

bool
DecodingInfo::char_before_cursor_is_separator() const
{
    int len = m_surface.length();
    if (m_cursor_pos > len) return false;
    if (m_cursor_pos > 0 && m_surface[m_cursor_pos - 1] == '\'') {
        return true;
    }
    return false;
}

int
DecodingInfo::get_fixed_len() const
{
    return m_fixed_len;
}
