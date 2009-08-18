#include "google_imengine.h"
#include "composing_view.h"

ComposingView::Status
ComposingView::get_status() const
{
    return m_status;
}

void
ComposingView::reset()
{
    m_status = SHOW_PINYIN;
}

void
ComposingView::set_visibility(bool visible)
{
    if (visible) {
        m_pinyin->show_preedit_string();
    } else {
        m_pinyin->hide_preedit_string();
    }
}

void
ComposingView::move_cursor(int offset)
{
    if (m_status == EDIT_PINYIN) {
        m_dec_info->move_cursor(offset);
    } else if (m_status == SHOW_STRING_LOWERCASE) {
        m_status = EDIT_PINYIN;
    }
    // TODO
    m_pinyin->refresh_preedit_string();
}

    
