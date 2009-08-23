#include "decoding_info.h"
#include "candidate_view.h"
#include "google_imengine.h"
#include "pinyin_lookup_table.h"

CandidateView::CandidateView(GooglePyInstance *pinyin, DecodingInfo *dec_info)
    : m_pinyin(pinyin),
      m_dec_info(dec_info),
      m_page_no(-1),
      m_cand_in_page(0),
      m_page_size(10),
      m_active_highlight(true)
{}

bool
CandidateView::cursor_left()
{
    if (m_cand_in_page > 0) {
        show_page(m_page_no, m_cand_in_page - 1, true);
        return true;
    }
    return false;
}

bool
CandidateView::cursor_right()
{
    if (!m_dec_info->page_ready(m_page_no)) return false;
    if (m_cand_in_page + 1 < m_dec_info->get_current_page_size(m_page_no)) {
        show_page(m_page_no, m_cand_in_page + 1, true);
        return true;
    }
    return false;
}

bool
CandidateView::page_up()
{
    if (m_page_no == 0) return false;
    // XXX: always highlight
    show_page(m_page_no - 1, m_cand_in_page, m_active_highlight);
    m_pinyin->page_up();
    return true;
}

bool
CandidateView::page_down()
{
    if (!m_dec_info->prepare_page(m_page_no + 1)) {
        return false;
    }
    // XXX: always highlight
    
    show_page(m_page_no + 1, m_cand_in_page, m_active_highlight);
    m_pinyin->page_down();
    return true;
}

void
CandidateView::set_visibility(bool visibility)
{
    if (visibility) {
        m_pinyin->refresh_lookup_table();
        m_pinyin->show_lookup_table();
    } else {
        m_pinyin->hide_lookup_table();
    }
}

void
CandidateView::enable_active_highlight(bool enable)
{
    m_active_highlight = enable;
    // update lookup table
}

int
CandidateView::get_current_page() const
{
    return m_page_no;
}

int
CandidateView::get_active_candidate_pos() const
{
    return m_cand_in_page;
}

int
CandidateView::get_page_size() const
{
    return m_page_size;
}

void
CandidateView::set_page_size(int page_size)
{
    m_page_size = page_size;
}

void
CandidateView::show_candidates(DecodingInfo *dec_info,
                               bool enable_active_highlight)
{
    m_dec_info = dec_info;
    show_page(0, 0, enable_active_highlight);
    set_visibility(true);
}

void
CandidateView::redraw()
{
    if (m_dec_info->is_candidates_list_empty()) return;
    show_page(m_page_no, m_cand_in_page, true);
    set_visibility(true);
}

void
CandidateView::show_page(int page_no, int cand_in_page, bool enable_active_highlight)
{
    m_page_no = page_no;
    m_cand_in_page = cand_in_page;
    m_active_highlight = enable_active_highlight;
    m_dec_info->calculate_page(m_page_no, this);
    //set_visibility(true);
}

void
CandidateView::append_candidate(const wstring& cand)
{
    // TODO
}

AttributeList
CandidateView::get_attributes(int index) const
{
    AttributeList attrs;
    // TODO
    return attrs;
}
