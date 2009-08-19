#include "google_imengine.h"
#include "composing_view.h"
#include "decoding_info.h"

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

void
ComposingView::redraw()
{
    switch (m_status) {
    case EDIT_PINYIN:
    case SHOW_PINYIN:
        draw_pinyin();
        break;
    case SHOW_STRING_LOWERCASE:
        draw_english();
        break;
    default:
        assert(false && "unknown composing status");
    }
}

// see PinyinInstance::refresh_aux_string()
void
ComposingView::draw_for_pinyin()
{
    int cursor_pos = m_dec_info->get_cursor_pos_in_cmps_display();
    int cmps_pos = cursor_pos;
    wstring cmps_str = m_dec_info->get_composing_str_for_display();
    int active_cmps_len = m_dec_info->get_active_cmps_display_len();
    if (cursor_pos > active_cmps_len) cmps_pos = active_cmps_len;
    
    AttributeList attrs;
    wstring aux;
    wstring item;
    item = cmps_str.substr(0, cmps_pos);
    attrs.push_back(
        Attribute(aux.length(), item.length(),
                  SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_NONE));
    aux += item;
    aux += L" ";
    
    if (cursor_pos <= active_cmps_len) {
        item = cmps_str.substr(cmps_pos, active_cmps_len);
        attrs.push_back(
            Attribute(aux.length(), item.length(),
                      SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_REVERSE));
        aux += item;
        aux += L" ";
    }
    
    if (cmps_str.length() > active_cmps_len) {
        int orig_pos = active_cmps_len;
        if (cursor_pos > active_cmps_len) {
            if (cursor_pos > cmps_str.length()) cursor_pos = cmps_str.length();
            item = cmps_str.substr(orig_pos, cursor_pos);
            attrs.push_back(Attribute(aux.length(), item.length()));
            aux += item;
            aux += L" ";
            orig_pos = cursor_pos;
        }
        item = cmps_str.substr(orig_pos);
        attrs.push_back(Attribute(aux.length(), item.length(),
                                  SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_NONE));
    }
    update_aux_string(aux, attrs);
}

void
ComposingView::draw_for_english()
{
    AttributeList attrs;
    wstring aux;
    wstring item;

    aux = m_dec_info->get_original_spl_str();
    attrs.push_back(Attribute(0, aux.length(),
                              SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_HIGHLIGHT));
    update_aux_string(aux, attrs);
}

void
ComposingView::update_aux_string(const wstring& aux,
                                 const AttributeList& attrs)
{
    if (!aux.empty()) {
        m_pinyin->update_aux_string(aux, attrs);
        show_aux_string();
    } else {
        hide_aux_string();
    }
}
