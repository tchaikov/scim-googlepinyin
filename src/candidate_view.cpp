#include "decoding_info.h"
#include "candidate_view.h"
#include "google_imengine.h"

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

void
CandidateView::set_visibility(bool visibility)
{
    if (visibility) {
        m_pinyin->show_lookup_table();
    } else {
        m_pinyin->hide_lookup_table();
    }
}

bool
CandidateView::show_candidates(DecodingInfo *dec_info,
                               bool enable_active_highlight)
{
    m_dec_info = dec_info;
    show_page(0, 0, enable_active_highlight);
}

void
CandidateView::show_page(int page_no, int cand_in_page, bool enable_active_highlight)
{
    m_page_no = page_no;
    m_cand_in_page = cand_in_page;
    m_active_highlight = enable_active_highlight;
    set_visibility(true);
}

void
CandidateView::append_candidate(const wstring& cand)
{
    
}

AttributeList
CandidateView::get_attributes(int index) const
{

}
