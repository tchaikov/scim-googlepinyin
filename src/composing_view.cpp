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
{}
